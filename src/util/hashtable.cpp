// Hash table implementation
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


#include "hashtable.h"
// C++ header
#include <stdexcept>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#ifdef __APPLE__
  #include <string>
#else
  #include <string.h>
#endif

/* Create a new hashtable. */
Hashtable::Hashtable( const long int size
                    , const long int max_size )
                    : hash_size_(max_size)
                    , size_(0)
                    , max_size_(size)
                    , min_shrink_(1)
{
	/* Allocate the table itself. */
  if( ( table_ = (entry_t*)calloc( size, sizeof(entry_t) ) ) == NULL ) {
    throw std::runtime_error("error while allocating hash table!!");
  }
	if( ( hash_ = (long int*)malloc( max_size * sizeof(long int) ) ) == NULL ) {
		throw std::runtime_error("error while allocating hash table!!");
	}
  // initialization hash
  for (int i = 0; i < max_size; i++) hash_[i] = -1;
}

/* Delete a hashtable. */
Hashtable::~Hashtable()
{
    for (int i=0; i<size_; i++) free(table_[i].key);
    free(table_);
    free(hash_);
}


/* Hash a string for a particular hash table. */
long int const Hashtable::hash( const char *key ) {

	unsigned long int hashval = 0;
	int i = 0;

	/* Convert our string to an integer */
	while( hashval < ULONG_MAX && i < strlen( key ) ) {
		hashval = hashval << 8;
		hashval += key[ i ];
		i++;
	}

	return hashval % hash_size_;
}


/* Insert a key into a hash table. */
long int const Hashtable::insert( const char *key ) {
  unsigned int bin, length = strlen(key) + 1;
  if (length > MAX_TOKEN) length = MAX_TOKEN;
  table_[size_].key = (char *)calloc(length, sizeof(char));
  strcpy(table_[size_].key, key);
  table_[size_].value = 0;
  size_++; // increment the size

  // shrink hashtable if needed
  if (size_>hash_size_*0.7) shrink();

  // Reallocate memory if needed
  if (size_ + 2 >= max_size_) {
    max_size_ += 1000;
    table_ = (entry_t *)realloc(table_, max_size_ * sizeof(entry_t));
  }
  bin = hash(key);
  while (hash_[bin] != -1) bin = (bin + 1) % hash_size_;
  hash_[bin] = size_ - 1;

  return size_ - 1;
}

/* Insert a key-value pair into a hash table. */
void Hashtable::insert( const char *key, const int value ) {
  // get hashing index
  int ht_idx = get(key);
  if (ht_idx == -1){
    ht_idx = insert(key); // new word to add
  }
  table_[ht_idx].value+=value; // increment value
}

/* Retrieve a hashing index from a hash table key. */
long int const Hashtable::get( const char *key ) {
	int bin = hash(key);
  /* Step through the bin, looking for our value. */
  while (1) {
    if (hash_[bin] == -1) return -1;
    if (!strcmp(key,table_[hash_[bin]].key)) return hash_[bin];
    bin = (bin + 1) % hash_size_;
  }
  return -1;
}

/* Retrieve a key-value pair from a hash table. */
int const Hashtable::value( const char *key ) {
  // get hashing index
  int ht_idx = get(key);
  if (ht_idx == -1){
    return -1;
  }else{
    return table_[ht_idx].value;
  }
}


// Shrink the hashtable by removing infrequent values
void Hashtable::shrink(){
  int a, b = 0;
  int bin;
  for (a = 0; a < size_; a++) if (table_[a].value > min_shrink_) {
    table_[b].value = table_[a].value;
    table_[b].key = table_[a].key;
    b++;
  } else free(table_[a].key);
  size_ = b; // set the new size
  for (a = 0; a < hash_size_; a++) hash_[a] = -1;
  for (a = 0; a < size_; a++) {
    // Hash will be re-computed, as it is not actual
    bin = get(table_[a].key);
    while (hash_[bin] != -1) bin = (bin + 1) % hash_size_;
    hash_[bin] = a;
  }
  min_shrink_++; // increment shrinking value
}

/* Used in ht_sort for sorting by value */
int Hashtable::compare(const void *a, const void *b) {
    return ((entry_t *)b)->value - ((entry_t *)a)->value;
}

/* Sorts hashtable by values (descending order) */
void Hashtable::sort() {
  // Sort the vocabulary
  qsort(table_, size_, sizeof(entry_t), compare);
}

/* Function to print hashtable (key value\n). */
void Hashtable::print(const char *filename){
  int i;
  FILE * fout = fopen(filename, "wb");
  if (fout == NULL){
    std::string error_msg = std::string("Cannot open file ")
                          + std::string(filename)
                          + std::string(" !!!");
    throw std::runtime_error(error_msg);
  }
  for (i = 0; i < size_; i++)
    fprintf(fout, "%s %d\n", table_[i].key, table_[i].value);
  fclose(fout);
}
