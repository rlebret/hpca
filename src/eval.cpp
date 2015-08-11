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
#include <algorithm>
#include <cfloat>
#include <iostream>

// include utility headers
#include "util/util.h"
#include "util/file.h"
#include "util/hashtable.h"
#include "util/constants.h"

// include redsvd headers
#include "redsvd/util.h"

// define global variables
int verbose=true;
char *c_word_file_name, *c_vocab_file_name, *c_output_file_name;
// variable for handling vocab
hashtable_t * hash;
int vocab_size;

template<typename T>
class CompareIndicesByAnotherVectorValues{
    std::vector<T>* _values;
    public:
        CompareIndicesByAnotherVectorValues(std::vector<T>* values) : _values(values) {}
    public:
        bool operator() (const int& a, const int& b) const { return (*_values)[a] > (*_values)[b]; }
};
    
    
std::vector<int> ordered(std::vector<float>* values) {
    std::vector<int> indices(values->size());
    for (int i = 0; i != indices.size(); ++i) indices[i] = i;
    CompareIndicesByAnotherVectorValues<float> comp(values);
    std::sort(indices.begin(), indices.end(), comp);
    return indices;
}

// Read matrix from file
void readMatrix(const char *filename, Eigen::MatrixXf& result)
{
    File matfile((std::string(filename)));
    // get number of rows from file
    const int rows = matfile.number_of_line();
    /* check whether number of lines in words is equal to the number of words into vocab file */
    if (rows != vocab_size){
        throw std::string("Size mismatch between words and vocabulary files!!!\n");
    }
    // get number of rows from file
    const int cols = matfile.number_of_column(' ');
    if (verbose) fprintf(stderr, "words vector size = %d\n",cols);
    
    // opening file
    matfile.open();
    
    // Populate matrix with numbers.
    result.resize(rows,cols);
    char *line=NULL;
    char delim=' ';
    for (int i = 0; i < rows; i++){
        line = matfile.getline();
        char *olds = line;
        char olddelim = delim;
        int j=0;
        while(olddelim && *line) {
            while(*line && (delim != *line)) line++;
            *line ^= olddelim = *line; // olddelim = *line; *line = 0;
            result(i,j++) = atof(olds);
            *line++ ^= olddelim; // *line = olddelim; line++;
            olds = line;
        }
    }
    // closing file
    matfile.close();
};

/* get cosine similarity between two vectors */
float cossim(const Eigen::VectorXf& a, const Eigen::VectorXf& b){
    const float ab = a.dot(b);
    const float a2 = a.dot(a);
    const float b2 = b.dot(b);
    const float ratio = 1./sqrtf(a2*b2);
    return ab * ratio;
}

/* load vocabulary */
void getvocab(){
    
    File fp(c_vocab_file_name);
    vocab_size = fp.number_of_line();
    // create vocab
    hash = ht_create(vocab_size);
    // get vocabulary
    fp.open();
    char * line = NULL;
    int i=0;
    while ((line = fp.getline()) != NULL) {
        ht_insert(hash, line, i++);
    }
    fp.close();
    if (verbose) fprintf(stderr, "number of words in vocabulary = %d\n",vocab_size);
}

/* return the mean of a vector */
float const getmean(const std::vector<float>& vec){
    const int nrow = vec.size();
    float s=0;
    for (int i=0; i<nrow; i++)
        s += vec[i];
    return s/nrow;
}

/* return the standard deviation of a vector */
float const getstd(const std::vector<float>& vec, const float mean){
    const int nrow = vec.size();
    float s=0;
    for (int i=0; i<nrow; i++)
        s += powf((vec[i]-mean), 2.0);
   
    const float variance = s/nrow;
    return sqrtf(variance);
}

/* worker for similarity dataset */
void runsimilarity( const Eigen::MatrixXf& words
                  , const char * filename ){
    
    // get data from file
    File fp(filename);
    fp.open();
    char token1[MAX_TOKEN];
    char token2[MAX_TOKEN];
    float coeff;
    
    int i,j;
    std::vector<int> idx1, idx2;
    std::vector<float> gold;
    char * line = NULL;
    while ((line = fp.getline()) != NULL) {
        sscanf(line, "%s\t%s\t%f", token1, token2, &coeff);
        if ( ((i = ht_get(hash, token1))) && ((j = ht_get(hash, token2))) ){
            idx1.push_back(i);
            idx2.push_back(j);
            gold.push_back(coeff);
        }
    }
    fp.close();

    
    const int npairs=gold.size();
    if (verbose) fprintf(stderr, "number of pairs = %d\n",npairs);
    
    // get scores
    std::vector<float> scores(npairs);
    for (int i=0; i<npairs; i++){
        const float ab = words.row(idx1[i]).dot(words.row(idx2[i]));
        const float a2 = words.row(idx1[i]).dot(words.row(idx1[i]));
        const float b2 = words.row(idx2[i]).dot(words.row(idx2[i]));
        const float ratio = 1./sqrtf(a2*b2);
        scores[i] = ab * ratio;
    }
    
    // Pearson's correlation
    // get mean
    const float cossim_mean = getmean(scores);
    // get standard deviation
    const float cossim_std = getstd(scores, cossim_mean);
    
    // get gold scores
    const float  gold_mean = getmean(gold);
    const float  gold_std = getstd(gold, gold_mean);
    
    float r=0;
    for (int i=0; i<npairs; i++){
        r += (scores[i]-cossim_mean)*(gold[i]-gold_mean);
        
    }
    r/=((npairs-1)*cossim_std*gold_std);
    
    //  Spearman's correlation
    std::vector<int> cossimsortidx = ordered(&scores);
    std::vector<int> goldsortidx = ordered(&gold);
    
    float sum=0;
    for (int i=0; i<npairs; i++){
        const int idx = cossimsortidx[i];
        int j=0;
        while (goldsortidx[j] != idx) j++;
        const int s = i-j;
        sum += s*s;
    }
    const float p = 1-((6*sum)/(npairs*(npairs*npairs-1)));
    
    if (verbose){
        fprintf(stderr, "Pearson's correlation = %f\n",r);
        fprintf(stderr, "Spearman's correlation = %f\n",p);
    }
}


/* worker for analogies dataset */
float const runanalogy( const Eigen::MatrixXf& words
                      , const char * filename){
    
    char token1[MAX_TOKEN];
    char token2[MAX_TOKEN];
    char token3[MAX_TOKEN];
    char token4[MAX_TOKEN];
    int i,j,k,l;
    
    std::vector<int> a,b,c,d,idx;
    File fp(filename);
    fp.open();
    char * line = NULL;
    while ((line = fp.getline()) != NULL) {
        sscanf(line, "%s %s %s %s", token1, token2, token3, token4);
        if (   ((i = ht_get(hash, token1)))
            && ((j = ht_get(hash, token2)))
            && ((k = ht_get(hash, token3)))
            && ((l = ht_get(hash, token4)))  ){
            a.push_back(i);
            b.push_back(j);
            c.push_back(k);
            d.push_back(l);
            idx.push_back(i);
            idx.push_back(j);
            idx.push_back(k);
            idx.push_back(l);
        }
    }
    fp.close();
    
    // get unique indices
    std::vector<int>::iterator it;
    it = std::unique (idx.begin(), idx.end());
    
    const int n=a.size();
    const int nword=idx.size();
    if (verbose){
        fprintf(stderr, "number of questions = %d\n",n);
        fprintf(stderr, "number of words = %d\n",nword);
    }
    
    // define vector to store distances
    std::vector<float> distances(nword);
    
    int acc=0;
    // loop over examples
    for (int i=0; i<n; i++){
      
        // compute prediction
        Eigen::VectorXf y = words.row(b[i]) - words.row(a[i]) + words.row(c[i]);
        
        // initialize distances with dummy values
        std::fill(distances.begin(), distances.end(), FLT_MAX);
        
        // prepare stuff for cosine similary
        const float len = y.dot(y);
        y/=sqrtf(len);
            
        // loop over all words
        for (int j=0; j<nword; j++){
            // get distance
            const int curridx = idx[j];
            if ( (curridx!=a[i]) && (curridx!=b[i]) && (curridx!=c[i]))
                distances[j]=words.row(curridx).dot(y);
        }
        // ordered
        std::vector<int> sortidx = ordered(&distances);
        if (sortidx[sortidx.size()-1]==d[i]) acc++;
    }
    const float final_acc = (float)acc/n;
    if (verbose) fprintf(stderr,"accuracy = %.4f\n", final_acc);

    return final_acc;
}



int main(int argc, char **argv) {
    int i;
    int rw=true;
    int rg=true;
    int ws=true;
    int syn=true;
    int sem=true;
    
    c_word_file_name = (char*)malloc(sizeof(char) * MAX_PATH_NAME + MAX_FILE_NAME);
    c_vocab_file_name = (char*)malloc(sizeof(char) * MAX_PATH_NAME + MAX_FILE_NAME);
    
    if (argc == 1) {
        printf("HPCA: Hellinger PCA for Word Representation, embeddings evaluation\n");
        printf("Author: Remi Lebret (remi@lebret.ch)\n\n");
        printf("Usage options:\n");
        printf("\t-verbose <int>\n");
        printf("\t\tSet verbosity: 0 or 1 (default)\n");
        printf("\t-word-file <file>\n");
        printf("\t\tFile containing word embeddings to evaluate\n");
        printf("\t-vocab-file <file>\n");
        printf("\t\tFile containing word vocabulary\n");
        printf("\t-ws353 <int>\n");
        printf("\t\tDo WordSim-353 evaluation: 0 or 1 (default)\n");
        printf("\t-rg65 <int>\n");
        printf("\t\tDo Rubenstein and Goodenough 1965 evaluation: 0 or 1 (default)\n");
        printf("\t-rw <int>\n");
        printf("\t\tDo Stanford Rare Word evaluation: 0 or 1 (default)\n");
        printf("\t-syn <int>\n");
        printf("\t\tDo Microsoft Research Syntactic Analogies: 0 or 1 (default)\n");
        printf("\t-sem <int>\n");
        printf("\t\tDo Google Semantic Analogies: 0 or 1 (default)\n");
        printf("\nExample usage:\n");
        printf("./eval -input-file path_to_words -vocab-file path_to_vocab -ws353 1 -rg65 1 -rw 1 -syn 0 -sem 0\n\n");
        return 0;
    }
    
    if ((i = find_arg((char *)"-verbose", argc, argv)) > 0) verbose = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-sim353", argc, argv)) > 0) ws = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-rg65", argc, argv)) > 0) rg = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-rw", argc, argv)) > 0) rw = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-syn", argc, argv)) > 0) syn = atof(argv[i + 1]);
    if ((i = find_arg((char *)"-sem", argc, argv)) > 0) sem = atof(argv[i + 1]);
    if ((i = find_arg((char *)"-word-file", argc, argv)) > 0) strcpy(c_word_file_name, argv[i + 1]);
    if ((i = find_arg((char *)"-vocab-file", argc, argv)) > 0) strcpy(c_vocab_file_name, argv[i + 1]);
    
    /* check whether files exist */
    is_file(c_word_file_name);
    is_file(c_vocab_file_name);
    
    /* get vocabulary */
    getvocab();
    
    /* get words */
    Eigen::MatrixXf words;
    readMatrix(c_word_file_name, words);
    
    if (ws){
        if (verbose){
            fprintf(stderr, "\n------------------------------------------------\n");
            fprintf(stderr, "WordSim-353 Dataset\n");
            fprintf(stderr, "------------------------------------------------\n");
            
        }
        runsimilarity( words, "../../data/wordsim353.txt");
    }
    if (rg){
        if (verbose){
            fprintf(stderr, "\n------------------------------------------------\n");
            fprintf(stderr, "Rubenstein and Goodenough (1965) Dataset\n");
            fprintf(stderr, "------------------------------------------------\n");
            
        }
        runsimilarity( words, "../../data/rg65.txt");
    }
    if (rw){
        if (verbose){
            fprintf(stderr, "\n------------------------------------------------\n");
            fprintf(stderr, "Stanford Rare Word Dataset\n");
            fprintf(stderr, "------------------------------------------------\n");
        }
        runsimilarity( words, "../../data/rareword.txt");
    }
    if (syn || sem){ // prepare stuff for cosine similary
        Eigen::VectorXf rownorm = words.rowwise().norm();
        for (int i=0; i<words.rows(); i++)
            words.row(i)*=1.0/rownorm(i);
    }
    if (syn){
        if (verbose){
            fprintf(stderr, "\n------------------------------------------------\n");
            fprintf(stderr, "Microsoft Research Syntactic Analogies\n");
            fprintf(stderr, "------------------------------------------------\n");
        }
        const char * filename[]={"../../data/question/gram1-adjective-to-adverb.txt"
            ,"../../data/question/gram2-opposite.txt"
            ,"../../data/question/gram3-comparative.txt"
            ,"../../data/question/gram4-superlative.txt"
            ,"../../data/question/gram5-present-participle.txt"
            ,"../../data/question/gram6-nationality-adjective.txt"
            ,"../../data/question/gram7-past-tense.txt"
            ,"../../data/question/gram8-plural.txt"
            ,"../../data/question/gram9-plural-verbs.txt"};
        const char * taskname[] = {"adjective-to-adverb"
            ,"opposite"
            ,"comparative"
            ,"superlative"
            ,"present-participle"
            ,"nationality-adjective"
            ,"past-tense"
            ,"plural"
            ,"plural-verbs"};
        
        float acc=0.0;
        for (int i=0;i<9; i++){
            if (verbose) fprintf(stderr,"---- %s analogies ----\n", taskname[i]);
            acc += runanalogy( words, filename[i] );
        }
        if (verbose) fprintf(stderr,"\nSyntactic accuracy = %.4f\n", acc);
    }
    if (sem){
        if (verbose){
            fprintf(stderr, "\n------------------------------------------------\n");
            fprintf(stderr, "Google Semantic Analogies\n");
            fprintf(stderr, "------------------------------------------------\n");
        }
        const char * filename[]={"../../data/question/capital-common-countries.txt"
            ,"../../data/question/capital-world.txt"
            ,"../../data/question/city-in-state.txt"
            ,"../../data/question/currency.txt"
            ,"../../data/question/family.txt"};
        const char * taskname[]={"capital-common-countries"
            ,"capital-world"
            ,"city-in-state"
            ,"currency"
            ,"family"};
        
        float acc=0.0;
        for (int i=0;i<5; i++){
            if (verbose) fprintf(stderr,"---- %s analogies ----\n", taskname[i]);
            acc += runanalogy( words, filename[i] );
        }
        if (verbose) fprintf(stderr,"Semantic accuracy = %.4f\n", acc);
    }
    // release memory
    ht_delete(hash);
}