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

// include utility headers
#include "util/util.h"
#include "util/thread.h"
#include "util/file.h"
#include "util/hashtable.h"
#include "util/constants.h"

// include datasets
#include "data/wordsim353.h"
#include "data/rg65.h"
#include "data/rareword.h"
#include "data/question/gram1-adjective-to-adverb.h"
#include "data/question/gram2-opposite.h"
#include "data/question/gram3-comparative.h"
#include "data/question/gram4-superlative.h"
#include "data/question/gram5-present-participle.h"
#include "data/question/gram6-nationality-adjective.h"
#include "data/question/gram7-past-tense.h"
#include "data/question/gram8-plural.h"
#include "data/question/gram9-plural-verbs.h"
#include "data/question/family.h"
#include "data/question/capital-common-countries.h"
#include "data/question/capital-world.h"
#include "data/question/city-in-state.h"
#include "data/question/currency.h"

// include redsvd headers
#include "redsvd/util.h"

// define global variables
int verbose=true;
int lower=true; 
int num_threads=8;
char *c_word_file_name, *c_vocab_file_name;
// variable for handling vocab
Hashtable * hash;
int vocab_size;

template<typename T>
class CompareIndicesByAnotherVectorValues{
    std::vector<T>* _values;
    public:
        CompareIndicesByAnotherVectorValues(std::vector<T>* values) : _values(values) {}
    public:
        bool operator() (const int& a, const int& b) const { return (*_values)[a] > (*_values)[b]; }
};
    
    
std::vector<float> rank(std::vector<float>& values, std::vector<int>& indices){
    std::vector<float> r(values.size());
    int i=0;
    while (1){
        int n=0,s=0;
        while ( (i<(r.size()-1)) && (values[indices[i]]==values[indices[i+1]]) ){
            s += i;
            i++; n++;
        }
        if ( n>0 ){ // found equal values, update rank.
            const float newrank = (float)(s+i)/(n+1);
            for (int j=(i-n); j<=i; j++) r[j] = newrank;
            n=0; s=0; 
        }else{ r[i]=i; }
        i++;
        if (i==r.size()) break;
    }
    return r;
}

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
        throw std::runtime_error("Size mismatch between words and vocabulary files!!!\n");
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
        char *ptr = line;
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
        free(ptr);
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
    
    File fp((std::string(c_vocab_file_name)));
    vocab_size = fp.number_of_line();
    // create vocab
    hash = new Hashtable(vocab_size);
    // open file
    fp.open();
    // get vocabulary
    char * line = NULL;
    int i=0;
    while( (line=fp.getline()) != NULL) {
        hash->insert(line, i++);
        free(line);
    }
    // closing file
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
   
    const float variance = s/(nrow-1);
    return sqrtf(variance);
}

/* worker for similarity dataset */
void runsimilarity( const Eigen::MatrixXf& words
                  , char * data
                  , unsigned int length){
    
    // get data
    char * ptr_data = data;
    char * buffer = (char*)malloc(MAX_TOKEN);
    char token1[MAX_TOKEN];
    char token2[MAX_TOKEN];
    float coeff;
    
    int itr=0;
    int i,j;
    std::vector<int> idx1, idx2;
    std::vector<float> gold;
    while ((buffer = string_copy(buffer, ptr_data, &itr, '\n')) != '\0') {
        sscanf(buffer, "%s\t%s\t%f", token1, token2, &coeff);
        // lowercase?
        if (lower){
            lowercase(token1);
            lowercase(token2);
        } 
        if ( ((i = hash->value(token1))!=-1) && ((j = hash->value(token2))!=-1) ){
            idx1.push_back(i);
            idx2.push_back(j);
            gold.push_back(coeff);
        }
        ptr_data=&data[++itr];
        if (itr==length) break;
    }
    free(buffer);

    
    const long long int npairs=gold.size();
    if (verbose) fprintf(stderr, "number of pairs = %d\n",npairs);
    if (npairs>0){
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

        // compute correlation
        float r=0;
        for (int i=0; i<npairs; i++){
            r += (scores[i]-cossim_mean)*(gold[i]-gold_mean);
            
        }
        r/=((npairs-1)*cossim_std*gold_std);
        
        //  Spearman's correlation
        std::vector<int> cossimsortidx = ordered(&scores);
        std::vector<int> goldsortidx = ordered(&gold);
        
        // get ranks
        std::vector<float> cossimrank = rank(scores, cossimsortidx);
        std::vector<float> goldrank = rank(gold, goldsortidx);
        float sum=0;
        for (int i=0; i<npairs; i++){
            const int idx = cossimsortidx[i];
            int j=0;
            while (goldsortidx[j] != idx) j++;
            const float s = cossimrank[i]-goldrank[j];
            sum += s*s;
        }
        const float p = 1-(6*sum)/(npairs*(npairs*npairs-1));
        
        fprintf(stdout, "Pearson's correlation = %f\n",r);
        fprintf(stdout, "Spearman's correlation = %f\n",p);
        if (verbose && !isatty(STDOUT_FILENO)){
            fprintf(stderr, "Pearson's correlation = %f\n",r);
            fprintf(stderr, "Spearman's correlation = %f\n",p);
        }
    }
}


/* worker for analogies dataset */
float const runanalogy( const Eigen::MatrixXf& words
                       , char * data
                       , unsigned int length){
    
    // get data
    char * ptr_data = data;
    char * buffer = (char*)malloc(MAX_TOKEN);
    char token1[MAX_TOKEN];
    char token2[MAX_TOKEN];
    char token3[MAX_TOKEN];
    char token4[MAX_TOKEN];
    int i,j,k,l;
    int itr=0;
    
    std::vector<int> a,b,c,d,idx;
    while ((buffer = string_copy(buffer, ptr_data, &itr, '\n')) != '\0') {
        sscanf(buffer, "%s %s %s %s", token1, token2, token3, token4);        
        // lowercase?
        if (lower){
            lowercase(token1);
            lowercase(token2);
            lowercase(token3);
            lowercase(token4);
        } 
        if (   ((i = hash->value(token1))!=-1)
            && ((j = hash->value(token2))!=-1)
            && ((k = hash->value(token3))!=-1)
            && ((l = hash->value(token4))!=-1)  
            ){
            a.push_back(i);
            b.push_back(j);
            c.push_back(k);
            d.push_back(l);
            idx.push_back(i);
            idx.push_back(j);
            idx.push_back(k);
            idx.push_back(l);
        }
        ptr_data=&data[++itr];
        if (itr==length) break;
    }
    free(buffer);
    
    // get unique indices
    std::vector<int>::iterator it;
    it = std::unique (idx.begin(), idx.end());
    
    const int n=a.size();
    const int nword=idx.size();
    if (verbose){
        fprintf(stderr, "number of questions = %d\n",n);
        fprintf(stderr, "number of words = %d\n",nword);
    }
    if (n>0){
        // define vector to store distances
        std::vector<float> distances(nword);
        
        // define temp matrices
        Eigen::MatrixXf A(words.cols(),n);
        Eigen::MatrixXf B(words.cols(),n);
        Eigen::MatrixXf C(words.cols(),n);
        Eigen::MatrixXf D(nword,words.cols());
        // initialize A, B and C matrices
        for (int i=0; i<n; i++){
            A.col(i) = words.row(a[i]);
            B.col(i) = words.row(b[i]);
            C.col(i) = words.row(c[i]);
        }
        // initialize D matrix
        for (int i=0; i<nword; i++) D.row(i) = words.row(idx[i]);

        // cosine similarity
        Eigen::MatrixXf y = D * (B - A + C);
        
        // get accuracy
        int acc=0;    
        // loop over all words
        for (int j=0; j<n; j++){
            float maxval=0;
            int maxidx;
            for (int i=0; i<nword; i++){
                const int curridx =idx[i];
                if ( (y(i,j)>maxval) && (curridx!=a[j]) && (curridx!=b[j]) && (curridx!=c[j])){
                    maxval = y(i,j);
                    maxidx=curridx;
                }
            }
            if ( maxidx == d[j] ) acc++;
        }
        const float final_acc = (float)acc/n;
        fprintf(stdout,"accuracy = %.4f\n", final_acc);
        if ( verbose && !isatty(STDOUT_FILENO) ){
            fprintf(stderr,"accuracy = %.4f\n", final_acc);
        }
        return final_acc;
    }else{
        return 0;
    }
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
        printf("HPCA: Hellinger PCA for Word Embeddings, embeddings evaluation\n");
        printf("Author: Remi Lebret (remi@lebret.ch)\n\n");
        printf("Usage options:\n");
        printf("\t-verbose <int>\n");
        printf("\t\tSet verbosity: 0=off or 1=on (default)\n");
        printf("\t-word-file <file>\n");
        printf("\t\tFile containing word embeddings to evaluate\n");
        printf("\t-vocab-file <file>\n");
        printf("\t\tFile containing word vocabulary\n");
        printf("\t-ws353 <int>\n");
        printf("\t\tDo WordSim-353 evaluation: 0=off or 1=on (default)\n");
        printf("\t-rg65 <int>\n");
        printf("\t\tDo Rubenstein and Goodenough 1965 evaluation: 0=off or 1=on (default)\n");
        printf("\t-rw <int>\n");
        printf("\t\tDo Stanford Rare Word evaluation: 0=off or 1=on (default)\n");
        printf("\t-syn <int>\n");
        printf("\t\tDo Microsoft Research Syntactic Analogies: 0=off or 1=on (default)\n");
        printf("\t-sem <int>\n");
        printf("\t\tDo Google Semantic Analogies: 0=off or 1=on (default)\n");
        printf("\t-lower <int>\n");
        printf("\t\tLowercased datasets? 0=off or 1=on (default)\n");
        printf("\t-threads <int>\n");
        printf("\t\tNumber of threads; default 8\n");
        printf("\nExample usage:\n");
        printf("./eval -word-file path_to_words -vocab-file path_to_vocab -ws353 1 -rg65 1 -rw 1 -syn 0 -sem 0\n\n");
        return 0;
    }
    
    if ((i = find_arg((char *)"-verbose", argc, argv)) > 0) verbose = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-ws353", argc, argv)) > 0) ws = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-rg65", argc, argv)) > 0) rg = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-rw", argc, argv)) > 0) rw = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-syn", argc, argv)) > 0) syn = atof(argv[i + 1]);
    if ((i = find_arg((char *)"-sem", argc, argv)) > 0) sem = atof(argv[i + 1]);
    if ((i = find_arg((char *)"-word-file", argc, argv)) > 0) strcpy(c_word_file_name, argv[i + 1]);
    if ((i = find_arg((char *)"-vocab-file", argc, argv)) > 0) strcpy(c_vocab_file_name, argv[i + 1]);
    if ((i = find_arg((char *)"-lower", argc, argv)) > 0) lower = atoi(argv[i + 1]);
    if ((i = find_arg((char *)"-threads", argc, argv)) > 0) num_threads = atoi(argv[i + 1]);
    
    if (verbose){
        fprintf(stderr, "HPCA: Hellinger PCA for Word Embeddings\n");
        fprintf(stderr, "Author: Remi Lebret (remi@lebret.ch)\n");
        fprintf(stderr, "---------------------------------------\n");
        fprintf(stderr, "embeddings evaluation\n" );
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
    getvocab();
    
    /* get words */
    Eigen::MatrixXf words;
    readMatrix(c_word_file_name, words);
    
    if (ws){
        fprintf(stdout, "\n---------------------------------------\n");
        fprintf(stdout, "WordSim-353 Dataset\n");
        fprintf(stdout, "---------------------------------------\n");
        if (verbose && !isatty(STDOUT_FILENO)){
            fprintf(stderr, "\n---------------------------------------\n");
            fprintf(stderr, "WordSim-353 Dataset\n");
            fprintf(stderr, "---------------------------------------\n"); 
        }
        runsimilarity( words, wordsim353_txt, wordsim353_txt_len);
    }
    if (rg){
        fprintf(stdout, "\n---------------------------------------\n");
        fprintf(stdout, "Rubenstein and Goodenough (1965) Dataset\n");
        fprintf(stdout, "---------------------------------------\n");
        if (verbose && !isatty(STDOUT_FILENO)){
            fprintf(stderr, "\n---------------------------------------\n");
            fprintf(stderr, "Rubenstein and Goodenough (1965) Dataset\n");
            fprintf(stderr, "---------------------------------------\n");
        }
        runsimilarity( words, rg65_txt, rg65_txt_len);
    }
    if (rw){
        fprintf(stdout, "\n---------------------------------------\n");
        fprintf(stdout, "Stanford Rare Word Dataset\n");
        fprintf(stdout, "---------------------------------------\n");
        if (verbose && !isatty(STDOUT_FILENO)){
            fprintf(stderr, "\n---------------------------------------\n");
            fprintf(stderr, "Stanford Rare Word Dataset\n");
            fprintf(stderr, "---------------------------------------\n");
        }
        runsimilarity( words, rareword_txt, rareword_txt_len);
    }
    if (syn || sem){ // prepare stuff for cosine similary
        words.rowwise().normalize();
    }
    if (syn){
        if (verbose && !isatty(STDOUT_FILENO)) {
            fprintf(stderr, "\n---------------------------------------\n");
            fprintf(stderr, "Microsoft Research Syntactic Analogies\n");
            fprintf(stderr, "---------------------------------------\n");
        }
        fprintf(stdout, "\n---------------------------------------\n");
        fprintf(stdout, "Microsoft Research Syntactic Analogies\n");
        fprintf(stdout, "---------------------------------------\n");
        
        float acc=0.0;
        if (verbose && !isatty(STDOUT_FILENO)) fprintf(stderr,"---- adjective-to-adverb analogies ----\n");
        fprintf(stdout,"---- adjective-to-adverb analogies ----\n");
        acc += runanalogy( words, question_gram1_adjective_to_adverb_txt, question_gram1_adjective_to_adverb_txt_len );
        if (verbose && !isatty(STDOUT_FILENO)) fprintf(stderr,"---- opposite analogies ----\n");
        fprintf(stdout,"---- opposite analogies ----\n");
        acc += runanalogy( words, question_gram2_opposite_txt, question_gram2_opposite_txt_len );
        if (verbose && !isatty(STDOUT_FILENO)) fprintf(stderr,"---- comparative analogies ----\n");
        fprintf(stdout,"---- comparative analogies ----\n");
        acc += runanalogy( words, question_gram3_comparative_txt, question_gram3_comparative_txt_len );
        if (verbose && !isatty(STDOUT_FILENO))  fprintf(stderr,"---- superlative analogies ----\n");
        fprintf(stdout,"---- superlative analogies ----\n");
        acc += runanalogy( words, question_gram4_superlative_txt, question_gram4_superlative_txt_len );
        if (verbose && !isatty(STDOUT_FILENO)) fprintf(stderr,"---- present-participle analogies ----\n");
        fprintf(stdout,"---- present-participle analogies ----\n");
        acc += runanalogy( words, question_gram5_present_participle_txt, question_gram5_present_participle_txt_len );
        if (verbose && !isatty(STDOUT_FILENO)) fprintf(stderr,"---- nationality-adjective analogies ----\n");
        fprintf(stdout,"---- nationality-adjective analogies ----\n");
        acc += runanalogy( words, question_gram6_nationality_adjective_txt, question_gram6_nationality_adjective_txt_len );
        if (verbose && !isatty(STDOUT_FILENO)) fprintf(stderr,"---- past-tense analogies ----\n");
        fprintf(stdout,"---- past-tense analogies ----\n");
        acc += runanalogy( words, question_gram7_past_tense_txt, question_gram7_past_tense_txt_len );
        if (verbose && !isatty(STDOUT_FILENO)) fprintf(stderr,"---- plural analogies ----\n");
        fprintf(stdout,"---- plural analogies ----\n");
        acc += runanalogy( words, question_gram8_plural_txt, question_gram8_plural_txt_len );
        if (verbose && !isatty(STDOUT_FILENO)) fprintf(stderr,"---- plural-verbs analogies ----\n");
        fprintf(stdout,"---- plural-verbs analogies ----\n");
        acc += runanalogy( words, question_gram9_plural_verbs_txt, question_gram9_plural_verbs_txt_len);
        
        if (verbose && !isatty(STDOUT_FILENO)) fprintf(stderr,"\nSyntactic accuracy = %.4f\n", acc/9);
        fprintf(stdout,"\nSyntactic accuracy = %.4f\n", acc/9);
    }
    if (sem){
        if (verbose && !isatty(STDOUT_FILENO)) {
            fprintf(stderr, "\n---------------------------------------\n");
            fprintf(stderr, "Google Semantic Analogies\n");
            fprintf(stderr, "---------------------------------------\n");
        }
        fprintf(stdout, "\n---------------------------------------\n");
        fprintf(stdout, "Google Semantic Analogies\n");
        fprintf(stdout, "---------------------------------------\n");
        
        float acc=0.0;

        if (verbose && !isatty(STDOUT_FILENO)) fprintf(stderr,"---- capital-common-countries analogies ----\n");
        fprintf(stdout,"---- capital-common-countries analogies ----\n");
        acc += runanalogy( words, question_capital_common_countries_txt, question_capital_common_countries_txt_len );
        if (verbose && !isatty(STDOUT_FILENO)) fprintf(stderr,"---- capital-world analogies ----\n");
        fprintf(stdout,"---- capital-world analogies ----\n");
        acc += runanalogy( words, question_capital_world_txt, question_capital_world_txt_len );
        if (verbose && !isatty(STDOUT_FILENO)) fprintf(stderr,"---- city-in-state analogies ----\n");
        fprintf(stdout,"---- city-in-state analogies ----\n");
        acc += runanalogy( words, question_city_in_state_txt, question_city_in_state_txt_len );
        if (verbose && !isatty(STDOUT_FILENO)) fprintf(stderr,"---- currency analogies ----\n");
        fprintf(stdout,"---- currency analogies ----\n");
        acc += runanalogy( words, question_currency_txt, question_currency_txt_len );
        if (verbose && !isatty(STDOUT_FILENO)) fprintf(stderr,"---- family analogies ----\n");
        fprintf(stdout,"---- family analogies ----\n");
        acc += runanalogy( words, question_family_txt, question_family_txt_len );
        
        if (verbose && !isatty(STDOUT_FILENO)) fprintf(stderr,"\nSemantic accuracy = %.4f\n", acc/5);
        fprintf(stdout,"\nSemantic accuracy = %.4f\n", acc/5);
    }

    /* release memory */
    free(c_vocab_file_name);
    free(c_word_file_name);
    delete hash;

    if (verbose){
        fprintf(stderr, "\ndone\n");
        fprintf(stderr, "---------------------------------------\n");
    }
    return 0;
}