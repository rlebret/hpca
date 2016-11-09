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
#include "util/constants.h"
#include "util/util.h"

/* load vocabulary */
int get_stats(const char* filename){

    char token[MAX_TOKEN];
    int freq;
    int i_freq=0;
    int nfreq=11;
    int cnt[11]={10000,5000,1000,500,100,50,10,5,4,3,2};
    int res[11]={0,0,0,0,0,0,0,0,0,0,0};
    float threshold=1;
    int i=0;
    int vocab_size=0;
    long int ntoken=0;

    // open vocabulary file
    FILE *fp = fopen(filename, "r");
    // get statistics on vocabulary
    while(fscanf(fp, "%s %d\n", token, &freq) != EOF){
        vocab_size++;
        ntoken+=freq;
    }

    fprintf(stderr, "number of word types           = %d\n",vocab_size);
    fprintf(stderr, "total number of tokens in file = %ld\n",ntoken);
    fprintf(stderr, "---------------------------------------\n");

    // get back at the beginning of the file
    fseek(fp, 0, SEEK_SET);

    float appearance_freq;
    const float ratio = 1.0/ntoken;
    while(fscanf(fp, "%s %d\n", token, &freq) != EOF){
        appearance_freq=freq*ratio;
        while ( (i_freq<nfreq) && (freq<cnt[i_freq]) ){
          res[i_freq]=i;
          i_freq++;
        }
        while (appearance_freq<threshold){
          fprintf(stderr, "number of word types with probability of occurrence >=%.1e   = %d\n",threshold, i);
          threshold/=10;
        }
        i++;
    }

    fclose(fp);
    fprintf(stderr, "---------------------------------------\n");
    for (i=0; i<11; i++)
        fprintf(stderr, "number of word types with occurrences >=%5d                   = %d\n",cnt[i], res[i]);

    return 0;
}


int main(int argc, char **argv) {
    int i;
    char *c_vocab_file_name = (char*)malloc(sizeof(char) * MAX_FULLPATH_NAME);

    if (argc == 1) {
        printf("HPCA: Hellinger PCA for Word Embeddings, get corpus descriptive statistics\n");
        printf("Author: Remi Lebret (remi@lebret.ch)\n\n");
        printf("Usage options:\n");
        printf("\t-vocab-file <file>\n");
        printf("\t\tVocabulary file\n");
        printf("\nExample usage:\n");
        printf("./stats -vocab-file vocab.txt\n\n");
        return 0;
    }

    fprintf(stderr, "HPCA: Hellinger PCA for Word Embeddings\n");
    fprintf(stderr, "Author: Remi Lebret (remi@lebret.ch)\n");
    fprintf(stderr, "---------------------------------------\n");
    fprintf(stderr, "get corpus descriptive statistics\n" );
    fprintf(stderr, "---------------------------------------\n\n");

    if ((i = find_arg((char *)"-vocab-file", argc, argv)) > 0) strcpy(c_vocab_file_name, argv[i + 1]);
    else strcpy(c_vocab_file_name, (char *)"vocab");

    /* check whether vocab file exists */
    is_file(c_vocab_file_name);

    /* get statistics */
    get_stats(c_vocab_file_name);
    /* release memory */
    free(c_vocab_file_name);

    fprintf(stderr, "\ndone\n");
    fprintf(stderr, "---------------------------------------\n");

    return 0;
}
