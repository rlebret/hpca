// This tools constructs word-word cooccurrence statistics from the corpus.
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

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <stdexcept>

// include utility headers
#include "util/data.h"
#include "util/constants.h"
#include "util/convert.h"
#include "util/thread.h"
#include "util/file.h"
#include "util/util.h"
#include "util/hashtable.h"

int verbose = true; // true or false
int dyn_cxt = false; // true or false
int min_freq = 100; // keep words appearing at least min_freq times
char *c_input_file_name, *c_vocab_file_name, *c_output_dir_name, *c_output_file_name;
int vocab_size=0;
long int ntoken=0;
float upper_bound=1.0;
float lower_bound=0.00001;
int Wid=0;
int Cid_upper=0;
int Cid_lower=0;
int cxt_size=5;
int num_threads = 8; // pthreads
float memory_limit = 4.0; // soft limit, in gigabytes, used to estimate optimal array sizes
unsigned long long max_cooccur_size;
int max_vocab_size = 10000;
// variable for handling vocab
Hashtable *hash;
char ** tokename;
int * tokenfound;

/* Merge [num] sorted files of cooccurrence records */
int merge_files(const int num, const int thread_id) {
    int i, size;
    long long counter = 0;
    cooccur_id_t *pq, new_id, old_id;
    
    char tmp_output_file_name[MAX_FILE_NAME];
    FILE **fid = (FILE**)malloc(sizeof(FILE*) * num);
    pq = (cooccur_id_t*)malloc(sizeof(cooccur_id_t) * num);
    
    // define final output file
    (thread_id!=-1)
    ? sprintf(tmp_output_file_name,"%s_%04d.bin",c_output_file_name, thread_id)
    : sprintf(tmp_output_file_name,"%s.bin",c_output_file_name);
    FILE *fout = fopen(tmp_output_file_name,"wb");;
    if (verbose && thread_id==-1)  fprintf(stderr,"\n\033[0Gmerging cooccurrence files: processed 0 cooccurrences.");

    /* Open all files and add first entry of each to priority queue */
    for(i = 0; i < num; i++) {
        (thread_id!=-1)
        ? sprintf(tmp_output_file_name,"%s-%d_%04d.bin",c_output_file_name, thread_id, i)
        : sprintf(tmp_output_file_name,"%s_%04d.bin",c_output_file_name, i);
        fid[i] = fopen(tmp_output_file_name,"rb");
        if(fid[i] == NULL) {fprintf(stderr, "Unable to open file %s.\n",tmp_output_file_name); return 1;}
        fread(&new_id, sizeof(cooccur_t), 1, fid[i]);
        new_id.id = i;
        insert_pq(pq,new_id,i+1);
    }
    
    /* Pop top node, save it in old to see if the next entry is a duplicate */
    size = num;
    old_id = pq[0];
    i = pq[0].id;
    delete_pq(pq, size);
    fread(&new_id, sizeof(cooccur_t), 1, fid[i]);
    if(feof(fid[i])) size--;
    else {
        new_id.id = i;
        insert_pq(pq, new_id, size);
    }
    
    /* Repeatedly pop top node and fill priority queue until files have reached EOF */
    while(size > 0) {
        counter += merge_write(pq[0], &old_id, fout); // Only count the lines written to file, not duplicates
        if((counter%100000) == 0) if(verbose && thread_id==-1) fprintf(stderr,"\033[39G%lld cooccurrences.",counter);
        i = pq[0].id;
        delete_pq(pq, size);
        fread(&new_id, sizeof(cooccur_t), 1, fid[i]);
        if(feof(fid[i])) size--;
        else {
            new_id.id = i;
            insert_pq(pq, new_id, size);
            if (thread_id==-1){// set this token has found in target vocabulary
                if (!tokenfound[new_id.idx1]) tokenfound[new_id.idx1]=true; // set this token has found
            }
        }
    }
    fwrite(&old_id, sizeof(cooccur_t), 1, fout);
    if (verbose && thread_id==-1){
        fprintf(stderr,"\033[0Gmerging cooccurrence files: processed %lld cooccurrences.\n",++counter);
        fprintf(stderr,"done, all cooccurrences saved in file %s.bin.\n", c_output_file_name);
    }  
    for(i=0;i<num;i++) {
        (thread_id!=-1)
        ? sprintf(tmp_output_file_name,"%s-%d_%04d.bin",c_output_file_name, thread_id, i)
        : sprintf(tmp_output_file_name,"%s_%04d.bin",c_output_file_name, i);
        //remove(tmp_output_file_name);
    }
    
    // release memory
    free(pq);
    free(fid);
    
    return 0;
}

/* load vocabulary */
int get_vocab(){
    
    char token[MAX_TOKEN];
    int freq;
    int i=0;
    
    // open vocabulary file
    FILE *fp = fopen(c_vocab_file_name, "r");
    // get statistics on vocabulary
    while(fscanf(fp, "%s %d\n", token, &freq) != EOF){
        if (freq>=min_freq) Wid++;
        vocab_size++;
        ntoken+=freq;
    }
    
    if (verbose){ // print out some statistics
        fprintf(stderr, "number of unique tokens                       = %d\n",vocab_size);
        fprintf(stderr, "total number of tokens in file                = %ld\n",ntoken);
        fprintf(stderr, "number of tokens to keep (>=%4d)             = %d\n",min_freq, Wid);
    }
    
    // get back at the beginning of the file
    fseek(fp, 0, SEEK_SET);
    
    // memory allocation
    float appearance_freq;
    const float ratio = 1.0/ntoken;
    hash = new Hashtable(vocab_size);
    tokename = (char**) malloc(sizeof(char*)*vocab_size);
    tokenfound = (int*)malloc(sizeof(int)*vocab_size);
    for (int i=0; i<vocab_size; i++) tokenfound[i]=false;
    
    // insert statistics on vocabulary
    while(fscanf(fp, "%s %d\n", token, &freq) != EOF){
        hash->insert(token, i);
        tokename[i] = (char*)malloc(strlen(token)+1);
        strcpy(tokename[i], token);
        appearance_freq=freq*ratio;
        if (appearance_freq>upper_bound) Cid_upper++;
        if (appearance_freq>=lower_bound) Cid_lower++;
        i++;
    }
    
    if (verbose) fprintf(stderr, "context vocabulary size [%.3e,%.3e] = %d\n",upper_bound, lower_bound, Cid_lower-Cid_upper);
    
    fclose(fp);
    
    return 0;
}

/* write out cooccurence vocabularies */
void write_vocab(){
    
    char * c_output_word_name = (char*)malloc(sizeof(char)*(strlen(c_output_dir_name)+strlen("target_words.txt")+2));
    sprintf(c_output_word_name, "%s/target_words.txt",c_output_dir_name);
    char * c_output_context_name = (char*)malloc(sizeof(char)*(strlen(c_output_dir_name)+strlen("context_words.txt")+2));
    sprintf(c_output_context_name, "%s/context_words.txt",c_output_dir_name);
    
    if (verbose){
        fprintf(stderr, "writing target words vocabulary in %s\n", c_output_word_name);
        fprintf(stderr, "writing context words vocabulary in %s\n", c_output_context_name);
    }
    // opening files
    FILE *fw = fopen(c_output_word_name, "w");
    FILE *fc = fopen(c_output_context_name, "w");
    
    for (int i=0; i<vocab_size; i++){
        if (tokenfound[i]){
            fprintf(fw, "%s\n", tokename[i]);
        }
        if (i>=Cid_upper && i<=Cid_lower){
            fprintf(fc, "%s\n", tokename[i]);
        }
    }
    //closing files
    fclose(fw);
    fclose(fc);
    
    // release memory
    free(c_output_word_name);
    free(c_output_context_name);
}

/* add context */
unsigned long long getcontext(cooccur_t *data, unsigned long long itr, const int* tokens, const int j, const int len){
    
    int rightcxt = (j-cxt_size)>0 ? j-cxt_size : 0;
    int leftcxt = (j+cxt_size+1)<len ? j+cxt_size+1 : len;
    const int target = tokens[j];
    
    // set dynamic context variables
    float weight, weight_itr;
    if (dyn_cxt){
        weight = (float)(cxt_size-(j-rightcxt)+1)/cxt_size;
        weight_itr = 1.0/cxt_size;
    }
    for (int k=rightcxt; k<leftcxt; k++){
        if (k!=j){
            const int t=tokens[k];
            // check whether this context is in our context vocabulary
            if (t>=Cid_upper && t<=Cid_lower){
                data[itr].idx1=target;
                data[itr].idx2=t-Cid_upper; // keep indices starting from 0
                (dyn_cxt) ? data[itr].val=weight : data[itr].val=1.0;
                itr++;
            }
        }
        if (dyn_cxt)
            (k<j)? weight+=weight_itr : weight-=weight_itr;
    }
    
    return itr;
}


/**
 * the worker
 **/
void *cooccurrence( void *p ){
    
    // get start & end for this thread
    Thread* thread = (Thread*)p;
    const long int start = thread->start();
    const long int end = thread->end();
    const long int nbop = (end-start)/100;
    // get output file name
    char output_file_name[MAX_FILE_NAME];
    
    // attach thread to CPU
    if (thread->id() != -1){
        thread->set();
        sprintf(output_file_name, "%s-%ld", c_output_file_name, thread->id());
        if (verbose) fprintf(stderr, "create pthread n°%ld, reading from position %ld to %ld\n",thread->id(), start, end-1);
    }else{
        strcpy(output_file_name, c_output_file_name);
    }
    
    // create output file
    int ftmp_itr=0;
    char tmp_output_file_name[MAX_FILE_NAME];
    sprintf(tmp_output_file_name,"%s_%04d.bin",output_file_name, ftmp_itr);
    if (verbose)  fprintf(stderr, "write in temporary files: %s_####.bin\n",output_file_name);
    FILE *ftmp = fopen(tmp_output_file_name, "wb");
    
    // create struct to store cooccurrence
    cooccur_t * data = (cooccur_t*)malloc(sizeof(cooccur_t)*max_cooccur_size);
    unsigned long long data_itr=0;
    const unsigned long long data_overflow = max_cooccur_size-cxt_size*2;
    
    // open input file
    std::string input_file_name = std::string(c_input_file_name);
    File input_file(input_file_name);
    input_file.open();
    input_file.jump_to_position(start);
    
    int k, itr=0;
    int line_size = MAX_TOKEN_PER_LINE;
    int *tokens = (int*)malloc(line_size*sizeof(int));
    char word[MAX_TOKEN];
    long int position=input_file.position();
    if (verbose) loadbar(thread->id(), itr, 100);
    // read and store tokens
    while (position<end){
        k=0;
        // get next word
        while (input_file.getword(word)){
            tokens[k++] = hash->get(word);
            if(k>=line_size) {
                line_size *= 2;
                tokens = (int*)realloc(tokens, sizeof(int) * line_size);
            }
        }
        // store token with context
        for (int j=0; j<k; j++){
            if (tokens[j]<Wid){
                data_itr = getcontext( data, data_itr, tokens, j, k);
                if (data_itr>data_overflow){ // save date on disk
                    qsort(data, data_itr, sizeof(cooccur_t), compare);
                    write(data,data_itr,ftmp);
                    fclose(ftmp);
                    sprintf(tmp_output_file_name,"%s_%04d.bin",output_file_name, ++ftmp_itr);
                    ftmp = fopen(tmp_output_file_name,"wb");
                    data_itr=0;
                }
            }
        }
        // get current position in stream
        position = input_file.position();
        if (verbose){
            if ( position-(start+(itr*nbop)) > nbop)
                loadbar(thread->id(), ++itr, 100);
        }
    }
    if (verbose) loadbar(thread->id(), 100, 100);
    qsort(data, data_itr, sizeof(cooccur_t), compare);
    write(data,data_itr,ftmp);
    fclose(ftmp);

    // closing input file
    input_file.close();
    
    // free memory
    free(data);
    free(tokens);
    
    // merge cooccurence files
    merge_files(ftmp_itr+1, thread->id());
    
    // exit thread
    if ( thread->id()!= -1 ){
        // existing pthread
        pthread_exit( (void*)thread->id() );
    }
    
    return 0;
}


/**
 * Run with multithreading
 **/
int run(){

    // get vocabulary from file
    get_vocab();
    
    // define input file
    std::string input_file_name = std::string(c_input_file_name);
    File input_file(input_file_name);
    
    // get input file byte size
    long int fsize = input_file.size();
    if (verbose){
        fprintf(stderr, "number of byte in %s = %ld\n",c_input_file_name,fsize);
        fflush(stderr);
    }
    
    // get optimal number of threads
    MultiThread threads( num_threads, 1, true, fsize, NULL, NULL);
    input_file.split(threads.nb_thread());
    // set max size for storing cooccurence
    const float current_memory = (float)get_available_memory()/GIGAOCTET;
    if (memory_limit>current_memory) memory_limit = current_memory;
    max_cooccur_size = (unsigned long long) (0.85 * current_memory * GIGAOCTET/(sizeof(cooccur_t)) / threads.nb_thread());
    // launch threads
    threads.linear( cooccurrence, input_file.flines );
    
    // merge temporary files
    if (threads.nb_thread()>1){
       merge_files(threads.nb_thread(), -1);
    } 
    
    // write vocabularies
    write_vocab();
    
    // free
    free(tokenfound);
    for (int i=0; i<vocab_size; i++) free(tokename[i]);
    free(tokename);
    free(hash);
    
    return 0;
}

int main(int argc, char **argv) {
    int i;
    c_input_file_name = (char*)malloc(sizeof(char) * MAX_PATH_NAME+MAX_FILE_NAME);
    c_vocab_file_name = (char*)malloc(sizeof(char) * MAX_PATH_NAME+MAX_FILE_NAME);
    c_output_dir_name = (char*)malloc(sizeof(char) * MAX_PATH_NAME);
    
    if (argc == 1) {
        printf("HPCA: Hellinger PCA for Word Representation, get co-occurrence probability matrix\n");
        printf("Author: Remi Lebret (remi@lebret.ch)\n\n");
        printf("Usage options:\n");
        printf("\t-verbose <int>\n");
        printf("\t\tSet verbosity: 0=off or 1=on (default)\n");
        printf("\t-input-file <file>\n");
        printf("\t\tInput file containing the tokenized and cleaned corpus text.\n");
        printf("\t-vocab-file <file>\n");
        printf("\t\tVocabulary file\n");
        printf("\t-output-dir <dir>\n");
        printf("\t\tOutput directory name to save files\n");
        printf("\t-min-freq <int>\n");
        printf("\tDiscarding all words with a lower appearance frequency (default is 100)\n");
        printf("\t-upper-bound <float>\n");
        printf("\tDiscarding words from the context vocabulary with a upper appearance frequency (default is 1.0)\n");
        printf("\t-lower-bound <float>\n");
        printf("\tDiscarding words from the context vocabulary with a lower appearance frequency (default is 0.00001)\n");
        printf("\t-cxt-size <int>\n");
        printf("\tSymmetric context size around words (default is 5)\n");
        printf("\t-dyn-cxt <int>\n");
        printf("\t\tDynamic context window, i.e. weighting by distance form the focus word: 0=off (default), 1=on\n");
        printf("\t-memory <float>\n");
        printf("\t\tSoft limit for memory consumption, in GB -- based on simple heuristic, so not extremely accurate; default 4.0\n");
        printf("\t-threads <int>\n");
        printf("\t\tNumber of threads; default 8\n");
        printf("\nExample usage:\n");
        printf("./cooccurrence -input-file data -vocab-file vocab.txt -output-dir path_to_dir -min-freq 100 -cxt-size 5 -dyn-cxt 1 -memory 4.0 -upper-bound 1.0 -lower-bound 0.00001 -verbose 1 -threads 4\n\n");
        return 0;
    }
    
    if ((i = find_arg((char *)"-verbose", argc, argv)) > 0) verbose = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-min-freq", argc, argv)) > 0) min_freq = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-upper-bound", argc, argv)) > 0) upper_bound = atof(argv[i + 1]);
    if ((i = find_arg((char *)"-lower-bound", argc, argv)) > 0) lower_bound = atof(argv[i + 1]);
    if ((i = find_arg((char *)"-cxt_size", argc, argv)) > 0) cxt_size = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-memory", argc, argv)) > 0) memory_limit = atof(argv[i + 1]);
    if ((i = find_arg((char *)"-threads", argc, argv)) > 0) num_threads = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-output-dir", argc, argv)) > 0) strcpy(c_output_dir_name, argv[i + 1]);
    else strcpy(c_output_dir_name, (char *)".");
    if ((i = find_arg((char *)"-vocab-file", argc, argv)) > 0) strcpy(c_vocab_file_name, argv[i + 1]);
    else strcpy(c_vocab_file_name, (char *)"vocab");
    if ((i = find_arg((char *)"-input-file", argc, argv)) > 0) strcpy(c_input_file_name, argv[i + 1]);
    
    /* check whether output directory exists */
    is_directory(c_output_dir_name);
    c_output_file_name = (char*)malloc(sizeof(char)*(strlen(c_output_dir_name)+strlen("cooccurence")+2));
    sprintf(c_output_file_name, "%s/cooccurence",c_output_dir_name);
    
    /* check whether input file exists */
    is_file(c_input_file_name);
    /* check whether vocab file exists */
    is_file(c_vocab_file_name);
    
    return run();
}
