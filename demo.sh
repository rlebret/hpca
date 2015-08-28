#!/bin/bash

# build and compile
./configure
make

# get corpus online if needed
if [ ! -e data/reuters.token.gz ]; then
cd data
wget http://lebret.ch/data/reuters.token.gz
cd ..
fi

# general options
EXP_DIR=demo
CORPUS=data/reuters.token.gz
CLEAN_CORPUS=data/reuters.token.clean
VOCAB_FILE=data/vocab.txt
VERBOSE=1
NUM_THREADS=4

# preprocessing options
DIGIT=1
LOWER=1
GZIP=0

# cooccurence options
MEMORY=4.0
VOCAB_MIN_COUNT=10
CONTEXT_VOCAB_UPPER_BOUND_FREQ=1.0
CONTEXT_VOCAB_LOWER_BOUND_FREQ=0.00001
DYN_CXT=0
WINDOW_SIZE=5

# pca options
RANK=300

# embeddings options
VECTOR_SIZE=100
NORM=0
EIG=0
EMB_NAME=embeddings.txt

# create output directory
mkdir $EXP_DIR

# preprocessing
./bin/preprocess -input-file $CORPUS -output-file $CLEAN_CORPUS -gzip $GZIP -lower $LOWER -digit $DIGIT -threads $NUM_THREADS -verbose $VERBOSE 

if [[ $? -eq 0 ]]
  then # get vocabulary from cleaned corpus
  ./bin/vocab -input-file $CLEAN_CORPUS  -vocab-file $VOCAB_FILE -threads $NUM_THREADS -verbose $VERBOSE 
  if [[ $? -eq 0 ]]
  then # get cooccurrence statistics
    ./bin/cooccurrence -input-file $CLEAN_CORPUS  -vocab-file $VOCAB_FILE -output-dir $EXP_DIR -min-freq $VOCAB_MIN_COUNT -cxt-size $WINDOW_SIZE -dyn-cxt $DYN_CXT -upper-bound $CONTEXT_VOCAB_UPPER_BOUND_FREQ -lower-bound $CONTEXT_VOCAB_LOWER_BOUND_FREQ -memory $MEMORY -threads $NUM_THREADS -verbose $VERBOSE 
    if [[ $? -eq 0 ]]
    then # perform Hellinger PCA
      ./bin/pca -input-dir $EXP_DIR -rank $RANK -threads $NUM_THREADS -verbose $VERBOSE
      if [[ $? -eq 0 ]]
      then # compute word embeddings
	      ./bin/embeddings -input-dir $EXP_DIR -output-name $EMB_NAME -dim $VECTOR_SIZE -eig $EIG -norm $NORM -threads $NUM_THREADS -verbose $VERBOSE
        if [[ $? -eq 0 ]]
        then # do word embeddings evaluation
          ./bin/eval -word-file $EXP_DIR/$EMB_NAME -vocab-file $EXP_DIR/target_words.txt -threads $NUM_THREADS -verbose $VERBOSE > $EXP_DIR/words-eval.txt
        fi
      fi
    fi
  fi
fi
