// This tool provides a lowercase conversion and/or a replacement of all numbers with a special token ('0') for a given corpus text.
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
int zip = false; // compress in gzip
char *c_input_file_name, *c_output_file_name;


/**
 * the worker
 **/
void *preprocess( void *p )
{
    // get start & end for this thread
    Thread* thread = (Thread*)p;
    const long int start = thread->start();
    const long int end = thread->end();
    const long int nbop = (end-start)/100;
    // get output file name
    std::string output_file_name = std::string(c_output_file_name);
    
    // attach thread to CPU
    if (thread->id() != -1){
        thread->set();
        output_file_name += "-" + typeToString(thread->id());
        if (verbose) fprintf(stderr, "create pthread n°%ld, reading from position %ld to %ld\n",thread->id(), start, end-1);
    }  
    
    // create output file
    File output_file(output_file_name,zip);
    output_file.open("w");
    
    // open input file
    std::string input_file_name = std::string(c_input_file_name);
    File input_file(input_file_name);
    input_file.open();
    input_file.jump_to_position(start);
    
    char *line = NULL;
    long int position=input_file.position();
    int itr=0;
    if (verbose) loadbar(thread->id(), itr, 100);
    while ( position<end ){
       
        // get the line
        line = input_file.getline();
       
        // lowercase?
        if (lower) lowercase(line);
        
        // replace digits?
        if (digit) replace_digit(line);
        
        // write the preprocessed line
        output_file.write(line);
        output_file.write("\n");
        
        // get current position in stream
        position = input_file.position();
        // display progress bar
        if (verbose){
            if ( position-(start+(itr*nbop)) > nbop)
                loadbar(thread->id(), ++itr, 100);
        }
        // release memory
        free(line);
    }
    // display last percent
    if (verbose) loadbar(thread->id(), 100, 100);
    // close output file
    output_file.close();
    // close input file
    input_file.close();
    
    // exit thread
    if ( thread->id()!= -1 ){
        pthread_exit( (void*)thread->id() );
    }
    return 0;
}

int merge(const int nthreads){
    
    if (verbose){ fprintf(stderr, "\nmerging files\n"); fflush(stderr);}
    
    std::string output_file_name = std::string(c_output_file_name);
    std::string combined_file_name = output_file_name;
    if (zip){
        File fout(combined_file_name, true);
        fout.open("w");
        char *line= NULL;
        for (int i=0; i<nthreads; i++){
            std::string thread_file_name = output_file_name + "-" + typeToString(i);
            File fin(thread_file_name+".gz", true) ;
            fin.open();
            while( (line=fin.getline()) != NULL){
                fout.write(line);
                fout.write("\n");
                free(line);
            }
            fin.close();
            remove(thread_file_name.c_str());
        }
        fout.close();
    }else{
        std::ofstream combined_file( combined_file_name.c_str(), std::ofstream::out ) ;
        for (int i=0; i<nthreads; i++){
            std::string thread_file_name = output_file_name + "-" + typeToString(i);
            std::ifstream file(thread_file_name.c_str()) ;
            combined_file << file.rdbuf();
            remove(thread_file_name.c_str());
        }
        combined_file.close();
        
    }
    
    return 0;
}

/**
 * Run preprocessing with multithreading
 **/
int run() {
    
    // define input file
    std::string input_file_name = std::string(c_input_file_name);
    File input_file(input_file_name);
    
    // get input file byte size
    long int fsize = input_file.size();
    if (verbose) fprintf(stderr, "number of byte in %s = %ld\n",c_input_file_name,fsize);
    
    MultiThread threads( num_threads, 1, true, fsize, NULL, NULL);
    input_file.split(threads.nb_thread());
    threads.linear( preprocess, input_file.flines );
    
    if (threads.nb_thread()>1){
        merge(threads.nb_thread());
    }else fprintf(stderr,"\n");
    
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
        printf("\t\tSet verbosity: 0=off or 1=on (default)\n");
        printf("\t-input-file <file>\n");
        printf("\t\tInput file to preprocess\n");
        printf("\t-output-file <file>\n");
        printf("\t\tOutput file to save preprocessed data\n");
        printf("\t-gzip <int>\n");
        printf("\t\tSave in gzip format? 0=off (default) or 1=on\n");
        printf("\t-lower <int>\n");
        printf("\t\tLowercased? 0=off or 1=on (default)\n");
        printf("\t-digit <int>\n");
        printf("\t\tReplace all digits with a special token? 0=off or 1=on (default)\n");
        printf("\t-threads <int>\n");
        printf("\t\tNumber of threads; default 8\n");
        printf("\nExample usage:\n");
        printf("./preprocess -input-file data -output-file clean_data -lower 1 -digit 1 -verbose 1 -threads 8 -gzip 1\n\n");
        return 0;
    }
    
    if ((i = find_arg((char *)"-verbose", argc, argv)) > 0) verbose = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-lower", argc, argv)) > 0) lower = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-digit", argc, argv)) > 0) digit = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-threads", argc, argv)) > 0) num_threads = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-gzip", argc, argv)) > 0) zip = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-output-file", argc, argv)) > 0) strcpy(c_output_file_name, argv[i + 1]);
    else strcpy(c_output_file_name, (char *)"clean_data");
    if ((i = find_arg((char *)"-input-file", argc, argv)) > 0) strcpy(c_input_file_name, argv[i + 1]);

    /* check whether input file exists */
    is_file( c_input_file_name );
    
    /* launch preproccessing */
    run();

    // release memory
    free(c_input_file_name);
    free(c_output_file_name);

    return 0;
}
