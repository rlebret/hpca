// This tool generates word embeddings from the Hellinger PCA.
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


// include utility headers
#include "util/util.h"
#include "util/file.h"
#include "util/data.h"
#include "util/constants.h"

// include redsvd headers
#include "redsvd/util.h"
#include "redsvd/redsvd.h"
#include "redsvd/redsvdFile.h"

// define global variables
int norm = false; // true or false
int dim = 100;
float eig=0.0;
char *c_input_dir_name, *c_input_file_U_name, *c_input_file_S_name, *c_output_file_name;


// Read matrix from file
void readMatrix(const char *filename, Eigen::MatrixXf& result)
{
    File matfile((std::string(filename)));
    // get number of rows from file
    const int rows = matfile.number_of_line();
    char *line=NULL;
    char delim=' ';
    
    // opening file
    matfile.open();
    
    // Populate matrix with numbers.
    result.resize(rows,dim);
    for (int i = 0; i < rows; i++){
        line = matfile.getline();
        char *olds = line;
        char olddelim = delim;
        int j=0;
        while(olddelim && *line) {
            while(*line && (delim != *line)) line++;
            *line ^= olddelim = *line; // olddelim = *line; *line = 0;
            result(i,j++) = atof(olds);
            if (j==dim) break;
            *line++ ^= olddelim; // *line = olddelim; line++;
            olds = line;
        }
    }
    // closing file
    matfile.close();
};

// Read vector from file
void readVector(const char *filename, Eigen::VectorXf& result)
{
    File vecfile((std::string(filename)));
    char *line=NULL;
    
    // opening file
    vecfile.open();
    
    // Populate matrix with numbers.
    result.resize(dim);
    for (int i = 0; i < dim; i++){
        line = vecfile.getline();
        if (eig==0.5){
            result(i) = sqrtf(atof(line));
        }else if(eig==0){
            result(i) = 1.0;
        }else{
            result(i) = atof(line);
        }
    }
    // closing file
    vecfile.close();
};

int run() {
    
    // read U matrix from file
    Eigen::MatrixXf U;
    readMatrix(c_input_file_U_name,U);
    // read S matrix from file
    Eigen::VectorXf S;
    readVector(c_input_file_S_name,S);
    
    Eigen::MatrixXf embeddings = U * S.asDiagonal();
    if (norm) embeddings.rowwise().normalize();
    
    // write out embeddings
    std::string output_name = std::string(c_input_dir_name) + "/" + std::string(c_output_file_name);
    FILE* outfp = fopen(output_name.c_str(), "wb");
    if (outfp == NULL){
        throw std::string("cannot open ") + output_name;
    }
    for (int i = 0; i < embeddings.rows(); ++i){
        for (int j = 0; j < embeddings.cols(); ++j){
            fprintf(outfp, "%+f ",  embeddings(i, j));
        }
        fprintf(outfp, "\n");
    }
    fclose(outfp);
    
    return 0;
}



int main(int argc, char **argv) {
    int i;
    c_input_dir_name = (char*)malloc(sizeof(char) * MAX_FILE_NAME);
    c_output_file_name = (char*)malloc(sizeof(char) * MAX_FILE_NAME);
    
    if (argc == 1) {
        printf("HPCA: Hellinger PCA for Word Representation, embeddings computation\n");
        printf("Author: Remi Lebret (remi@lebret.ch)\n\n");
        printf("Usage options:\n");
        printf("\t-verbose <int>\n");
        printf("\t\tSet verbosity: 0=off or 1=on (default)\n");
        printf("\t-input-dir <dir>\n");
        printf("\t\tDirectory where to find files from the SVD\n");
        printf("\t-output-name <string>\n");
        printf("\t\tFilename for embeddings file which will be placed in <input-dir> (default is words.txt)\n");
        printf("\t-eig <float>\n");
        printf("\t\tEigenvalue weighting (0.0, 0.5 or 1.0); default is 0.0\n");
        printf("\t-dim <int>\n");
        printf("\t\tWord vector dimension; default 100\n");
        printf("\t-norm <int>\n");
        printf("\t\tAre vectors normalized to unit length? 0=off or 1=on (default is 0)\n");
        printf("\nExample usage:\n");
        printf("./embeddings -input-dir path_to_svd_files -output-name words.txt -eig 0.0 -dim 100 -norm 0\n\n");
        return 0;
    }
    
    if ((i = find_arg((char *)"-norm", argc, argv)) > 0) norm = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-dim", argc, argv)) > 0) dim = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-eig", argc, argv)) > 0) eig = atof(argv[i + 1]);
    if ((i = find_arg((char *)"-input-dir", argc, argv)) > 0) strcpy(c_input_dir_name, argv[i + 1]);
    if ((i = find_arg((char *)"-output-name", argc, argv)) > 0) strcpy(c_output_file_name, argv[i + 1]);
    else strcpy(c_output_file_name, "words.txt");
    
    /* check whether input directory exists */
    is_directory(c_input_dir_name);
    
    /* create output filenames */
    c_input_file_U_name = (char*)malloc(sizeof(char)*(strlen(c_input_dir_name)+strlen("svd.U")+2));
    c_input_file_S_name = (char*)malloc(sizeof(char)*(strlen(c_input_dir_name)+strlen("svd.S")+2));
    sprintf(c_input_file_U_name, "%s/svd.U",c_input_dir_name);
    sprintf(c_input_file_S_name, "%s/svd.S",c_input_dir_name);
    
    /* check whether files exist */
    is_file( c_input_file_U_name );
    is_file( c_input_file_S_name );
    
    return run();
}
