// This tool extracts from the corpus words with their respective frequency.
//
// Copyright (c) 2015 Idiap Research Institute, http://www.idiap.ch/
// Written by Rémi Lebret <remi@lebret.ch>
//
// This file is part of HPCA.
//
// HPCA is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 3 as
// published by the Free Software Foundation.
//
// HPCA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with HPCA. If not, see <http://www.gnu.org/licenses/>.



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
int max_vocab_size = 10000;

/**
 * Write out vocabulary file
 **/
int writevocab(std::string output_file_name, Hashtable *hash){
    
    if (verbose) fprintf(stderr, "Writing vocabulary file in %s\n", output_file_name.c_str());
   
    // sorting
    hash->sort();

    // writing
    hash->print(output_file_name.c_str());
    
    if(verbose) fprintf(stderr,"Counted %ld unique words.\n", hash->size());
    
    return 0;
}

/**
 * the worker
 **/
void *getvocab( void *p ){
    
    // get start & end for this thread
    Thread* thread = (Thread*)p;
    const long int start = thread->start();
    const long int end = thread->end();
    const long int nbyte = (end-start);
    // get output file name
    std::string output_file_name = std::string(c_vocab_file_name);
    
    // attach thread to CPU
    if (thread->id() != -1){
        thread->set();
        output_file_name += "-" + typeToString(thread->id());
        if (verbose){
            fprintf(stderr,"create pthread n°%ld, reading from position %ld to %ld\n",thread->id(), start, end-1);
        }
    }
    
    // create vocab
    Hashtable hash(max_vocab_size);
    // open input file
    std::string input_file_name = std::string(c_input_file_name);
    File input_file(input_file_name);
    input_file.open();
    input_file.jump_to_position(start);
    
    long long ntokens=0;
    // read and store tokens
    char word[MAX_TOKEN];
    long int position=input_file.position();
    int ht_idx;
    while ( position<end ){
        // get next word
        while (input_file.getword(word)){
            if ( (ht_idx = hash.get(word)) == -1 ){ // get hashing index
                ht_idx = hash.insert(word); // new word to add
            }
            hash.increment(ht_idx); // increment counters
            ++ntokens;
        }
        position = input_file.position();
        if (verbose) loadbar(thread->id(), (position-start), nbyte);
    }
    // closing input file
    input_file.close();

    // exit thread
    if ( thread->id()!= -1 ){
        long long *ptr_ntokens = (long long *) thread->object; 
        // increment total number of tokens
        (*ptr_ntokens) += ntokens;
        // write hash table
        hash.print(output_file_name.c_str());
        // existing pthread
        pthread_exit( (void*)thread->id() );
    }else{
        if (verbose) fprintf(stderr, "\ndone after reading %lld tokens.\n", ntokens);
        // write out
        writevocab(output_file_name, &hash);
    }
    return 0;
}

int merge(const int nthreads){
    
    if (verbose) fprintf(stderr,"merging all %d temporary files\n",nthreads);
    
    // get output file name
    std::string output_file_name = std::string(c_vocab_file_name);
    // create vocab
    Hashtable hash(max_vocab_size);
    
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
            hash.insert(token, freq);
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
    writevocab( output_file_name, &hash );

    return 0;
}

/**
 * Run with multithreading
 **/
int run() {
    // define input file
    std::string input_file_name = std::string(c_input_file_name);
    File input_file(input_file_name);
    
    // get input file byte size
    long int fsize = input_file.size();
    if (verbose) fprintf(stderr, "number of byte in %s = %ld\n",c_input_file_name,fsize);
    
    // initialize number of tokens counter
    long long ntokens=0;
    
    // get optimal number of threads
    MultiThread threads( num_threads, 1, true, fsize, NULL, &ntokens);
    input_file.split(threads.nb_thread());    
    threads.linear( getvocab, input_file.flines );
    
   if (threads.nb_thread()>1){
       if (verbose) fprintf(stderr, "\ndone after reading %lld tokens.\n", ntokens);
       merge(threads.nb_thread());
   } 
    
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
        printf("\t\tSet verbosity: 0=off or 1=on (default)\n");
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
