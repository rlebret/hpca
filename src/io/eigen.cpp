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

#include "array.h"

// C++ header
#include <stdexcept>

// include utility headers
#include "../util/file.h"
#include "../util/convert.h"

// redsvd headers
#include "../redsvd/util.h"

void read_matrix(
      File& matfile
    , Eigen::MatrixXf& result
    , const int nrows
    , const int ncols
    , const int truncated
){
    // opening file
    matfile.open();

    // Populate matrix with numbers.
    result.resize(nrows, ncols);
    char *line=NULL;
    char delim=' ';
    for (int i = 0; i < nrows; i++){
        line = matfile.getline();
        char *ptr = line;
        char *olds = line;
        char olddelim = delim;
        int j=0;
        while(olddelim && *line) {
            while(*line && (delim != *line)) line++;
            *line ^= olddelim = *line; // olddelim = *line; *line = 0;
            result(i,j++) = atof(olds);
            if (truncated && j==ncols) break;
            *line++ ^= olddelim; // *line = olddelim; line++;
            olds = line;
        }
        free(ptr);
    }
    // closing file
    matfile.close();
}

// Read matrix from file
void read_eigen_truncated_matrix(
      const char *filename
    , Eigen::MatrixXf& result
    , const int dim
){
    File matfile((std::string(filename)));
    // get number of rows from file
    const int rows = matfile.number_of_line();
    const int cols = matfile.number_of_column(' ');
    if (cols < dim){
        std::string err = "number of columns in " + std::string(filename) + " = " + typeToString(cols) + " --> choose a lower word vector dimension !!!";
        throw std::runtime_error(err);
    }
    // read file
    read_matrix(matfile, result, rows, dim, true);
};

void read_eigen_matrix(
      const char *filename
    , Eigen::MatrixXf& result
){
    File matfile((std::string(filename)));
    // get number of rows from file
    const int rows = matfile.number_of_line();
    const int cols = matfile.number_of_column(' ');
    // read file
    read_matrix(matfile, result, rows, cols, false);
};

// Read vector from file
void read_eigen_vector(
      const char *filename
    , Eigen::VectorXf& result
    , const int dim
    , const float eig
){
    File vecfile((std::string(filename)));
    const int rows = vecfile.number_of_line();
    if (rows < dim){
        std::string err = "number of lines in " + std::string(filename) + " = " + typeToString(rows) + " --> choose a lower word vector dimension !!!";
        throw std::runtime_error(err);
    }

    // opening file
    vecfile.open();

    // Populate matrix with numbers.
    result.resize(dim);
    char *line=NULL;
    for (int i = 0; i < dim; i++){
        line = vecfile.getline();
        if (eig==0.5){
            result(i) = sqrtf(atof(line));
        }else if(eig==0){
            result(i) = 1.0;
        }else{
            result(i) = atof(line);
        }
        free(line);
    }
    // closing file
    vecfile.close();
};


void write_eigen_matrix(
      const char *output_name
    , Eigen::MatrixXf matrix
){
    FILE* outfp = fopen(output_name, "wb");
    if (outfp == NULL){
        throw std::string("cannot open ") + std::string(output_name);
    }
    for (int i = 0; i < matrix.rows(); ++i){
        for (int j = 0; j < matrix.cols(); ++j){
            fprintf(outfp, "%+f ",  matrix(i, j));
        }
        fprintf(outfp, "\n");
    }
    fclose(outfp);
}
