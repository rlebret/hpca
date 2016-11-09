// This tool provides a quick evaluation of the word embeddings.
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


#include <vector>
#include <stdexcept>
#include <algorithm>
#include <cfloat>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

// include utility headers
#include "util/util.h"
#include "util/thread.h"
#include "util/file.h"
#include "util/hashtable.h"
#include "util/constants.h"

// include redsvd headers
#include "redsvd/util.h"

// include i/o headers
#include "io/eigen.h"
#include "io/vocab.h"

// define global variables
int verbose=true;
int num_threads=8;
int top=10;
char *c_word_file_name, *c_vocab_file_name, *c_list_file_name;
// variable for handling vocab
vocab hash;
char ** tokename;
int vocab_size;

/* return word nearest neighbors in the embedding space */
void getnn(FILE* fout, Eigen::MatrixXf m, const int idx){
    // find nearest neighbour
    Eigen::VectorXf dist = (m.rowwise() - m.row(idx)).rowwise().squaredNorm();
    std::vector<int> sortidx = REDSVD::Util::ascending_order(dist);
    for (int i=1;i<top;i++){
        fprintf(fout, "%s, ", tokename[sortidx[i]]);
    }
    fprintf(fout, "%s\n", tokename[sortidx[top]]);
}


int main(int argc, char **argv) {
    int i, idx;
    int interact=false;
    vocab::iterator vocab_itr;

    c_word_file_name = (char*)malloc(sizeof(char) * MAX_FULLPATH_NAME);
    c_list_file_name = (char*)malloc(sizeof(char) * MAX_FULLPATH_NAME);
    c_vocab_file_name = (char*)malloc(sizeof(char) * MAX_FULLPATH_NAME);

    if (argc == 1) {
        printf("HPCA: Hellinger PCA for Word Embeddings, nearest neighbors\n");
        printf("Author: Remi Lebret (remi@lebret.ch)\n\n");
        printf("Usage options:\n");
        printf("\t-verbose <int>\n");
        printf("\t\tSet verbosity: 0=off or 1=on (default)\n");
        printf("\t-word-file <file>\n");
        printf("\t\tFile containing word embeddings to evaluate\n");
        printf("\t-vocab-file <file>\n");
        printf("\t\tFile containing word vocabulary\n");
        printf("\t-list-file <file>\n");
        printf("\t\tFile containing a list of words from which the nearest neighbors will be computed, otherwise interactive mode\n");
        printf("\t-top <int>\n");
        printf("\t\tNumber of nearest neighbors; default 10\n");
        printf("\t-threads <int>\n");
        printf("\t\tNumber of threads; default 8\n");
        printf("\nExample usage:\n");
        printf("./eval -word-file path_to_words -vocab-file path_to_vocab -top 10\n\n");
        return 0;
    }

    if ((i = find_arg((char *)"-verbose", argc, argv)) > 0) verbose = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-word-file", argc, argv)) > 0) strcpy(c_word_file_name, argv[i + 1]);
    if ((i = find_arg((char *)"-list-file", argc, argv)) > 0) strcpy(c_list_file_name, argv[i + 1]);
    else interact=true;
    if ((i = find_arg((char *)"-vocab-file", argc, argv)) > 0) strcpy(c_vocab_file_name, argv[i + 1]);
    if ((i = find_arg((char *)"-top", argc, argv)) > 0) top = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-threads", argc, argv)) > 0) num_threads = atoi(argv[i + 1]);


    if (verbose){
        fprintf(stderr, "HPCA: Hellinger PCA for Word Embeddings\n");
        fprintf(stderr, "Author: Remi Lebret (remi@lebret.ch)\n");
        fprintf(stderr, "---------------------------------------\n");
        fprintf(stderr, "nearest neighbors\n" );
        fprintf(stderr, "---------------------------------------\n\n");
    }

    /* set the optimal number of threads */
    num_threads = MultiThread::optimal_nb_thread(num_threads, 1, num_threads);
    if (verbose) fprintf(stderr, "number of pthreads = %d\n", num_threads);
    // set threads
    Eigen::setNbThreads(num_threads);

    /* check whether files exist */
    is_file(c_word_file_name);
    is_file(c_vocab_file_name);

    /* get vocabulary */
    vocab_size = get_vocab(c_vocab_file_name, hash);
    tokename = get_words(c_vocab_file_name, vocab_size);
    if (verbose) fprintf(stderr, "number of words in vocabulary = %d\n",vocab_size);

    /* get words */
    Eigen::MatrixXf words;
    read_eigen_matrix(c_word_file_name, words);
    /* check whether number of lines in words is equal to the number of words into vocab file */
    if (words.rows() != vocab_size){
        throw std::runtime_error("Size mismatch between words and vocabulary files!!!\n");
    }
    if (verbose) fprintf(stderr, "words vector size = %d\n",words.cols());

    fprintf(stderr, "---------------------------------------\n");
    if (interact){
        /* initialize random seed: */
        srand (time(NULL));

        fprintf(stderr,"Interactive mode, please enter words.\nAlternative options:\n - press 'R' key for a random sample\n - press 'Q' key to exit\n");
        fprintf(stderr, "---------------------------------------\n");
        while(1){
            char w[MAX_TOKEN];
            fprintf(stderr, "\nEnter a word: ");
            scanf("%s",w);

            if ( (strcmp(w,"q") == 0) || (strcmp(w,"Q") == 0) ){ // exit
                break;
            }else{
                if ( (strcmp(w,"r") == 0) || (strcmp(w,"R") == 0) ){
                    idx = rand() % vocab_size + 1;
                }
                else{
                    if ( (vocab_itr = hash.find(w)) == hash.end() ){
                        fprintf(stderr, "unknown word, please enter a new one\n\n");
                        continue;
                    }else{
                        idx = vocab_itr->second;
                    }
                }
                fprintf(stderr, "computing nearest neighbors of %s...\n", tokename[idx]);
                getnn(stderr, words, idx);
            }
        }
    }else{
        is_file(c_list_file_name);
        fprintf(stderr, "computing nearest neighbors of words in %s...\n", c_list_file_name);
        File fp((std::string(c_list_file_name)));
        // open file
        fp.open();
        // get vocabulary
        char * line = NULL;
        int i=0;
        while( (line=fp.getline()) != NULL) {
            if ( (vocab_itr = hash.find(line)) != hash.end() ){
                fprintf(stdout, "%s --> ", line);
                getnn(stdout, words, vocab_itr->second);
            }
        }
        fp.close();
    }

    /* release memory */
    free(c_vocab_file_name);
    free(c_word_file_name);
    for (int i=0; i<vocab_size; i++) if (tokename[i]) free(tokename[i]);
    free(tokename);

    if (verbose){
        fprintf(stderr, "\ndone\n");
        fprintf(stderr, "---------------------------------------\n");
    }
    return 0;
}
