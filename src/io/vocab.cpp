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

#include "vocab.h"
#include <stdlib.h>     /* malloc, free, rand */

// include utility headers
#include "../util/file.h"

/* load vocabulary */
int const get_vocab(
      const char* filename
    , vocab& hash
  ){

    File fp((std::string(filename)));
    const int vocab_size = fp.number_of_line();
    // create vocab
    hash.reserve(vocab_size);
    // open file
    fp.open();
    // get vocabulary
    char * line = NULL;
    int i=0;
    while( (line=fp.getline()) != NULL) {
        hash[line]=i++;
        free(line);
    }
    // closing file
    fp.close();

    return vocab_size;
}


/* load vocabulary */
char** const get_words(
      const char* filename
    , const int vocab_size
  ){
    File fp((std::string(filename)));
    char **tokename = (char**) malloc(sizeof(char*)*vocab_size);
    // open file
    fp.open();
    // get vocabulary
    char * line = NULL;
    int i=0;
    while( (line=fp.getline()) != NULL) {
        tokename[i] = (char*)malloc(strlen(line)+1);
        strcpy(tokename[i], line);
        free(line);
        i++;
    }
    // closing file
    fp.close();

    return tokename;
}
