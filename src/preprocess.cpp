//
//  preprocess.cpp
//  hpca
//
//  Created by Rémi Lebret on 14/07/2015.
//
//

#include <stdexcept>
#include <fstream>
#include <cstdlib>
#include <cstring>


// include utility headers
#include "util/util.h"
#include "util/convert.h"
#include "util/constants.h"
#include "util/thread.h"
#include "util/file.h"

int verbose = true; // true or false
int lower = true; // true or false
int digit = true; // true or false
int num_threads = 8; // pthreads
int zip = true; // compress in gzip
char *c_input_file_name, *c_output_file_name;


/**
 * the worker
 **/
void *preprocess( void *p )
{
    // get start & end for this thread
    Thread* thread = (Thread*)p;
    const int start = thread->start();
    const int end = thread->end();
    // get output file name
    std::string output_file_name = std::string(c_output_file_name);
    
    // attach thread to CPU
    if (thread->id() != -1){
        thread->set();
        output_file_name += "-" + typeToString(thread->id());
        if (verbose) printf("create pthread n°%ld, reading lines %d to %d\n",thread->id(), start+1, end);
    }  
    
    // create output file
    File output_file(output_file_name,zip);
    output_file.open("w");
    
    // open input file
    std::string input_file_name = std::string(c_input_file_name);
    File input_file(input_file_name);
    input_file.open();
    input_file.jump_to_line(start);
    
    char *line = NULL;
    for (int i=start; i<end; i++){
        
        // get the line
        line = input_file.getline();
       
        // lowercase?
        if (lower) lowercase(line);
        
        // replace digits?
        if (digit) replace_digit(line);
        
        // write the preprocessed line
        output_file.write(line);
        output_file.write("\n");
    }
    // close output file
    output_file.close();
    // close input file
    input_file.close();
    
    // exit thread
    if ( thread->id()!= -1 ){
        if (verbose) printf("delete pthread n°%ld\n",thread->id());
        pthread_exit( (void*)thread->id() );
    }
    return 0;
}

int merge(const int nthreads){
    
    if (verbose) printf("merge files\n");
    
    std::string output_file_name = std::string(c_output_file_name);
    std::string combined_file_name = output_file_name;
    if (zip) combined_file_name += ".gz";
    std::ofstream combined_file( combined_file_name.c_str(), std::ofstream::out ) ;
    for (int i=nthreads-1; i>=0; i--){
        std::string thread_file_name = output_file_name + "-" + typeToString(i);
        if (zip) thread_file_name += ".gz";
        std::ifstream file(thread_file_name.c_str()) ;
        combined_file << file.rdbuf();
        std::remove(thread_file_name.c_str());
    }
    combined_file.close();
    
    return 0;
}

/**
 * Run preprocessing with multithreading
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
    threads.linear( preprocess );
    
    if (threads.nb_thread()>1) merge(threads.nb_thread());
    
    return 0;
}

int main(int argc, char **argv) {
    int i;
    c_input_file_name = (char*)malloc(sizeof(char) * MAX_PATH_NAME+MAX_FILE_NAME);
    c_output_file_name = (char*)malloc(sizeof(char) * MAX_PATH_NAME+MAX_FILE_NAME);
    
    if (argc == 1) {
        printf("HPCA: Hellinger PCA for Word Representation, preprocessing stage\n");
        printf("Author: Remi Lebret (remi@lebret.ch)\n\n");
        printf("Usage options:\n");
        printf("\t-verbose <int>\n");
        printf("\t\tSet verbosity: 0 or 1 (default)\n");
        printf("\t-input-file <file>\n");
        printf("\t\tInput file to preprocess\n");
        printf("\t-output-file <file>\n");
        printf("\t\tOutput file to save preprocessed data\n");
        printf("\t-gzip <int>\n");
        printf("\t\tSave in gzip format? 0 or 1 (default)\n");
        printf("\t-lower <int>\n");
        printf("\t\tLowercased? 0 or 1 (default)\n");
        printf("\t-digit <int>\n");
        printf("\t\tReplace all digits with a special token? 0, 1 (default)\n");
        printf("\t-threads <int>\n");
        printf("\t\tNumber of threads; default 8\n");
        printf("\nExample usage:\n");
        printf("./preprocess -input-file data -output-file clean_data -lower 1 -digit 1 -verbose 1 -threads 8 -gzip 1\n\n");
        return 0;
    }
    
    if ((i = find_arg((char *)"-verbose", argc, argv)) > 0) verbose = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-lower", argc, argv)) > 0) lower = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-threads", argc, argv)) > 0) num_threads = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-gzip", argc, argv)) > 0) zip = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-output-file", argc, argv)) > 0) strcpy(c_output_file_name, argv[i + 1]);
    else strcpy(c_output_file_name, (char *)"clean_data");
    if ((i = find_arg((char *)"-input-file", argc, argv)) > 0) strcpy(c_input_file_name, argv[i + 1]);

    /* check whether input file exists */
    is_file( c_input_file_name );
    
    return run();
}
