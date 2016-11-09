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

#include "cooccur.h"

// C++ header
#include <stdexcept>

// include utility headers
#include "../util/constants.h"
#include "../util/data.h"

// read cooccurence triplets
int const read_cooccurrence(
      const char* c_input_file_name
    , REDSVD::SMatrixXf& A
    , const int verbose
){
    int nrow = 0;
    unsigned long long nonZeroNum = 0;
    const double start = REDSVD::Util::getSec();
    // read and store cooccurrence data
    cooccur_t data;
    // vector to store rowsum
    std::vector<float> rowsum;
    float s=0;
    // open file
    FILE *fin = fopen(c_input_file_name,"rb");
    if(fin == NULL) {
      std::string err = "Unable to open file " + std::string(c_input_file_name) + "!";
      throw std::runtime_error(err);
    }
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
    const int ncontext = maxid+1;
    if (verbose) fprintf(stderr, "# of words:%d, # of context words:%d, # of non-zero entries:%lld\n", nrow, ncontext, nonZeroNum);
    // get back at the beginning of the file
    fseek(fin, 0, SEEK_SET);

    if (verbose) fprintf(stderr, "Storing cooccurrence matrix in memory...");

    // store cooccurrence in Eigen sparse matrix object
    A.resize(nrow, ncontext);
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

    return ncontext;
}
