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

// include utility headers
#include "util/util.h"
#include "util/thread.h"
#include "util/data.h"
#include "util/constants.h"

// include redsvd headers
#include "redsvd/util.h"
#include "redsvd/redsvd.h"
#include "redsvd/redsvdFile.h"

// define global variables
int verbose = true; // true or false
int num_threads=8;
int rank = 300;
char *c_input_dir_name, *c_input_file_name, *c_output_name;

int run() {
    int nrow = 0;
    unsigned long long nonZeroNum = 0;
    double start = REDSVD::Util::getSec();
    // read and store cooccurence data
    cooccur_t data;
    // vector to store rowsum
    std::vector<float> rowsum;
    float s=0;
    // open file
    FILE *fin = fopen(c_input_file_name,"rb");
    if(fin == NULL) {fprintf(stderr, "Unable to open file %s.\n",c_input_file_name); return 1;}
    if (verbose) fprintf(stderr, "Reading cooccurrence file %s.\n",c_input_file_name);
    // read to get information on data
    fread(&data, sizeof(cooccur_t), 1, fin);
    ++nrow; ++nonZeroNum;
    unsigned int current_idx = data.idx1;
    unsigned int maxid = data.idx2;
    s+=data.val;
    while(true){
        fread(&data, sizeof(cooccur_t), 1, fin);
        if(feof(fin)) break;
        ++nonZeroNum;
        if (data.idx2>maxid) maxid=data.idx2;
        if (data.idx1!=current_idx){
            ++nrow;
            current_idx=data.idx1;
            rowsum.push_back(s);
            s=0; // set sum to 0
        }
        s+=data.val;
    }
    if (verbose) fprintf(stderr, "# of words:%d, # of context words:%d, # of non-zero entries:%lld\n", nrow, maxid, nonZeroNum);
    // get back at the beginning of the file
    fseek(fin, 0, SEEK_SET);
    
    if (verbose) fprintf(stderr, "Storing cooccurence matrix in memory...");

    // store cooccurence in Eigen sparse matrix object
    REDSVD::SMatrixXf A;
    A.resize(nrow, maxid+1);
    A.reserve(nonZeroNum);
    nrow=0;
    // read to get information on data
    fread(&data, sizeof(cooccur_t), 1, fin);
    current_idx=data.idx1;
    A.startVec(nrow);
    A.insertBack(nrow, data.idx2) = sqrtf(data.val/(rowsum[nrow]+EPSILON)); // prevent division by 0 (should not happen anyway)
    while(true){
        fread(&data, sizeof(cooccur_t), 1, fin);
        if(feof(fin)) break;
        if (data.idx1!=current_idx){
            A.startVec(++nrow);
            current_idx=data.idx1;
        }
        A.insertBack(nrow, data.idx2) = sqrtf(data.val/(rowsum[nrow]+EPSILON));
    }
    A.finalize();
    
    // closing file
    fclose(fin);

    if (verbose) fprintf(stderr, "done in %.2f.\n",REDSVD::Util::getSec() - start);

    if (verbose) fprintf(stderr, "Running randomized SVD...");
    start = REDSVD::Util::getSec();
    REDSVD::RedSVD svdOfA(A, rank);
    if (verbose) fprintf(stderr, "done in %.2f.\n",REDSVD::Util::getSec() - start);
    
    // set output name
    std::string output_name = std::string(c_input_dir_name) + "/svd";
    REDSVD::writeMatrix(output_name, svdOfA);
    
    return 0;
}



int main(int argc, char **argv) {
    int i;
    c_input_dir_name = (char*)malloc(sizeof(char) * MAX_FILE_NAME);
    c_output_name = (char*)malloc(sizeof(char) * MAX_FILE_NAME);
    
    if (argc == 1) {
        printf("HPCA: Hellinger PCA for Word Representation, performing randomized SVD \n");
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
    
    /* check whether input directory exists */
    is_directory(c_input_dir_name);
    c_input_file_name = (char*)malloc(sizeof(char)*(strlen(c_input_dir_name)+strlen("cooccurence.bin")+2));
    sprintf(c_input_file_name, "%s/cooccurence.bin",c_input_dir_name);
    
    /* check whether cooccurrence.bin exists */
    is_file( c_input_file_name );
    
    /* set the optimal number of threads */
    num_threads = MultiThread::optimal_nb_thread(num_threads, 1, num_threads);
    // set threads
    Eigen::setNbThreads(num_threads);

    /* perform PCA */
    run();

    /* release memory */
    free(c_input_dir_name);
    free(c_output_name);
    free(c_input_file_name);

    return 0;
}
