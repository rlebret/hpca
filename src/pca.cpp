// This tool provides an efficient implementation of the Hellinger PCA of the word cooccurrence matrix.
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
#include <stdexcept>

// include utility headers
#include "util/util.h"
#include "util/thread.h"
#include "util/data.h"
#include "util/constants.h"

// include redsvd headers
#include "redsvd/util.h"
#include "redsvd/redsvd.h"
#include "redsvd/redsvdFile.h"

// include i/o headers
#include "io/cooccur.h"

// define global variables
int verbose = true; // true or false
int num_threads=8;
int rank = 300;
char *c_input_dir_name, *c_input_file_name;

int run() {
    // store cooccurrence in Eigen sparse matrix object
    REDSVD::SMatrixXf A;
    const int ncontext = read_cooccurrence(c_input_file_name, A, verbose);
    if (rank>ncontext){
        throw std::runtime_error("-rank must be lower than the number of context words!!");
    }

    if (verbose) fprintf(stderr, "Running randomized SVD...");
    const double start = REDSVD::Util::getSec();
    REDSVD::RedSVD svdOfA(A, rank);
    if (verbose) fprintf(stderr, "done in %.2f.\n",REDSVD::Util::getSec() - start);

    // set output name
    std::string output_name = std::string(c_input_dir_name) + "/svd";
    REDSVD::writeMatrix(output_name, svdOfA);

    return 0;
}



int main(int argc, char **argv) {
    int i;
    c_input_dir_name = (char*)malloc(sizeof(char) * MAX_PATH_NAME);

    if (argc == 1) {
        printf("HPCA: Hellinger PCA for Word Embeddings, performing randomized SVD \n");
        printf("Author: Remi Lebret (remi@lebret.ch)\n\n");
        printf("Usage options:\n");
        printf("\t-verbose <int>\n");
        printf("\t\tSet verbosity: 0=off or 1=on (default)\n");
        printf("\t-input-dir <dir>\n");
        printf("\t\tDirectory where to find cooccurrence.bin file\n");
        printf("\t-rank <int>\n");
        printf("\t\tNumber of components to keep; default 300\n");
        printf("\t-threads <int>\n");
        printf("\t\tNumber of threads; default 8\n");
        printf("\nExample usage:\n");
        printf("./pca -input-dir path_to_dir -rank 300\n\n");
        return 0;
    }

    if ((i = find_arg((char *)"-verbose", argc, argv)) > 0) verbose = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-rank", argc, argv)) > 0) rank = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-threads", argc, argv)) > 0) num_threads = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-input-dir", argc, argv)) > 0) strcpy(c_input_dir_name, argv[i + 1]);

    if (verbose){
        fprintf(stderr, "HPCA: Hellinger PCA for Word Embeddings\n");
        fprintf(stderr, "Author: Remi Lebret (remi@lebret.ch)\n");
        fprintf(stderr, "---------------------------------------\n");
        fprintf(stderr, "performing randomized SVD\n" );
        fprintf(stderr, "---------------------------------------\n\n");
    }

    /* check whether input directory exists */
    is_directory(c_input_dir_name);
    c_input_file_name = get_full_path(c_input_dir_name, "cooccurrence.bin");

    /* check whether cooccurrence.bin exists */
    is_file( c_input_file_name );

    /* check rank value */
    if ( rank<=0 ){
        throw std::runtime_error("-rank must be a positive integer!!");
    }

    /* set the optimal number of threads */
    num_threads = MultiThread::optimal_nb_thread(num_threads, 1, num_threads);
    // set threads
    Eigen::setNbThreads(num_threads);
    if (verbose) fprintf(stderr, "number of pthreads = %d\n", Eigen::nbThreads());

    /* perform PCA */
    run();

    /* release memory */
    free(c_input_dir_name);
    free(c_input_file_name);

    if (verbose){
        fprintf(stderr, "\ndone\n");
        fprintf(stderr, "---------------------------------------\n");
    }
    return 0;
}
