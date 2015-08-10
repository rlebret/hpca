//
//  vocab.cpp
//  hpca
//
//  Created by Rémi Lebret on 14/07/2015.
//
//

#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <iostream>

// include utility headers
#include "util/util.h"
#include "util/convert.h"
#include "util/constants.h"
#include "util/thread.h"
#include "util/file.h"
#include "util/hashtable.h"

int verbose = true; // true or false
int num_threads = 8; // pthreads
char *c_input_file_name, *c_vocab_file_name;


/* struct for storing vocabulary */
typedef struct vocabulary {
    char *word;
    long long count;
} Vocab;

/* Efficient string comparison */
int scmp( char *s1, char *s2 ) {
    while(*s1 != '\0' && *s1 == *s2) {s1++; s2++;}
    return(*s1 - *s2);
}

/* Vocab frequency comparison; no tie-breaker */
int compare_vocab(const void *a, const void *b) {
    long long c;
    if( (c = ((Vocab *) b)->count - ((Vocab *) a)->count) != 0) return ( c > 0 ? 1 : -1 );
    else return 0;
}


/**
 * Write out vocabulary file
 **/
int writevocab(std::string output_file_name, hashtable_t *hash){
    
    if (verbose) fprintf(stderr, "Writing vocabulary file in %s\n", output_file_name.c_str());
    // get from hash table to vocabulary
    int vocab_size = 12500;
    long long j=0;
    Vocab *vocab =  (Vocab*)malloc(sizeof(Vocab) * vocab_size);
    entry_t *htmp;
    
    for(int i = 0; i < TSIZE; i++) { // Migrate vocab to array
        htmp = hash->table[i];
        while (htmp != NULL) {
            vocab[j].word = htmp->key;
            vocab[j].count = htmp->value;
            j++;
            if(j>=vocab_size) {
                vocab_size += 2500;
                vocab = (Vocab *)realloc(vocab, sizeof(Vocab) * vocab_size);
            }
            htmp = htmp->next;
        }
    }
    if(verbose) fprintf(stderr,"Counted %lld unique words.\n", j);
    // Ordering words in descending order
    qsort(vocab, j, sizeof(Vocab), compare_vocab);
    
    // write out vocabulary file
    File output_file(output_file_name);
    output_file.open("w");
    char buffer[MAX_TOKEN+10];
    for (int k=0; k<j; k++){
        sprintf(buffer, "%s %lld\n",vocab[k].word, vocab[k].count);
        output_file.write(buffer);
    }
    output_file.close();
    
    // release memory
    free(vocab);
    
    return 0;
}

/**
 * the worker
 **/
void *getvocab( void *p ){
    
    // get start & end for this thread
    Thread* thread = (Thread*)p;
    const int start = thread->start();
    const int end = thread->end();
    // get output file name
    std::string output_file_name = std::string(c_vocab_file_name);
    
    // attach thread to CPU
    if (thread->id() != -1){
        thread->set();
        output_file_name += "-" + typeToString(thread->id());
        if (verbose) printf("create pthread n°%ld, reading lines %d to %d\n",thread->id(), start+1, end);
    }
    
    
    // create vocab
    hashtable_t *hash = ht_create(TSIZE);
    // open input file
    std::string input_file_name = std::string(c_input_file_name);
    File input_file(input_file_name);
    input_file.open();
    input_file.jump_to_line(start);
    
    long long ntoken=0;
    char delim = ' ';
    // read and store tokens
    char *line = NULL;
    for (int i=start; i<end; i++){
        // get the line
        line = input_file.getline();
        char *olds = line;
        char olddelim = delim;
        while(olddelim && *line) {
            while(*line && (delim != *line)) line++;
            *line ^= olddelim = *line; // olddelim = *line; *line = 0;
            ht_insert( hash, olds);
            if((((++ntoken)%100000) == 0) && verbose){
                if (thread->id() != -1) fprintf(stderr,"pthread n°%ld --> %lld tokens.\n", thread->id(), ntoken);
                else fprintf(stderr,"%lld tokens.\n", ntoken);
            }
            *line++ ^= olddelim; // *line = olddelim; line++;
            olds = line;
        }
    }
    // closing input file
    input_file.close();
    
    // exit thread
    if ( thread->id()!= -1 ){
        // write hash table
        ht_print(output_file_name.c_str(), hash);
        
        // release hash table
        ht_delete(hash);
        
        if (verbose) printf("delete pthread n°%ld\n",thread->id());
        pthread_exit( (void*)thread->id() );
    }else{
        // write out
        writevocab(output_file_name, hash);
        
        // release hash table
        ht_delete(hash);
    }
    return 0;
}

int merge(const int nthreads){
    
    if (verbose) printf("merging all %d temporary files\n",nthreads);
    
    // get output file name
    std::string output_file_name = std::string(c_vocab_file_name);
    // create vocab
    hashtable_t *hash = ht_create(TSIZE);
    
    char token[MAX_TOKEN];
    int freq;
    char temp_hash_file[MAX_FILE_NAME];
    // loop over pthread
    for (int t=0; t<nthreads; t++){
        sprintf(temp_hash_file, "%s-%d", c_vocab_file_name,t);
        FILE *fp = fopen(temp_hash_file,"r");
        if (fp==NULL){
            std::string error_msg = std::string("Error opening tempory file ")
            + output_file_name + "-" + typeToString(t)
            + std::string(" !!!\n");
            throw std::runtime_error(error_msg);
        }
        while(fscanf(fp, "%s %d\n", token, &freq) != EOF){
            ht_insert( hash, token, freq);
        }
        fclose(fp);
        if( remove(temp_hash_file ) != 0 ){
            std::string error_msg = std::string("Error deleting tempory file ")
            + output_file_name + "-" + typeToString(t)
            + std::string(" !!!\n");
            throw std::runtime_error(error_msg);
        }
    }
    // write out
    writevocab( output_file_name, hash );
    
    // release hash table
    ht_delete(hash);
    
    return 0;
}

/**
 * Run with multithreading
 **/
int run() {
    
    std::string input_file_name = std::string(c_input_file_name);
    File input_file(input_file_name);
    int nbline = input_file.number_of_line();
    if (nbline==0){
        std::string error_msg = std::string("Data file ")
        + input_file_name
        + std::string(" is empty !!!\n");
        throw std::runtime_error(error_msg);
    }
    if (verbose) printf("number of lines in %s = %d\n",c_input_file_name,nbline);
    
    MultiThread threads( num_threads, 1, true, nbline, NULL, NULL);
    threads.linear( getvocab );
    
   if (threads.nb_thread()>1) merge(threads.nb_thread());
    
    return 0;
}


int main(int argc, char **argv) {
    int i;
    c_input_file_name = (char*)malloc(sizeof(char) * MAX_FILE_NAME);
    c_vocab_file_name = (char*)malloc(sizeof(char) * MAX_FILE_NAME);
    
    if (argc == 1) {
        printf("HPCA: Hellinger PCA for Word Representation, vocabulary extraction\n");
        printf("Author: Remi Lebret (remi@lebret.ch)\n\n");
        printf("Usage options:\n");
        printf("\t-verbose <int>\n");
        printf("\t\tSet verbosity: 0 or 1 (default)\n");
        printf("\t-input-file <file>\n");
        printf("\t\tInput file from which to extract the vocabulary\n");
         printf("\t-vocab-file <file>\n");
        printf("\t\tOutput file to save the vocabulary\n");
        printf("\t-threads <int>\n");
        printf("\t\tNumber of threads; default 8\n");
        printf("\nExample usage:\n");
        printf("./vocab -input-file clean_data -vocab-file vocab.txt -nthread 8 -verbose 1\n\n");
        return 0;
    }
    
    if ((i = find_arg((char *)"-verbose", argc, argv)) > 0) verbose = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-threads", argc, argv)) > 0) num_threads = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-input-file", argc, argv)) > 0) strcpy(c_input_file_name, argv[i + 1]);
    if ((i = find_arg((char *)"-vocab-file", argc, argv)) > 0) strcpy(c_vocab_file_name, argv[i + 1]);
    else strcpy(c_vocab_file_name, (char *)"vocab.txt");

    run();
    
    // free
    free(c_input_file_name);
    free(c_vocab_file_name);
    
    return 0;
}
