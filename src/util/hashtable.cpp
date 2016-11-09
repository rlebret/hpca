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

/* Used in ht_sort for sorting by value */
int hash_compare(const void *a, const void *b) {
    return ((entry_t *)b)->value - ((entry_t *)a)->value;
}

/* Sorts hashtable by values (descending order) */
entry_t const * hash_sort(vocab *hash) {
  int k=0;
  const size_t size = hash->size();
  entry_t *table_;
  /* Allocate the table itself. */
  if( ( table_ = (entry_t*)calloc( size, sizeof(entry_t) ) ) == NULL ) {
    throw std::runtime_error("error while allocating hash table!!");
  }

  for (vocab::iterator it=hash->begin(); it!=hash->end(); ++it){
    table_[k].key = it->first.c_str();
    table_[k++].value = it->second;
  }
  // Sort the vocabulary
  qsort(table_, size, sizeof(entry_t), hash_compare);
  return table_;
}

/* Function to print hashtable (key value\n). */
void hash_print(const vocab *hash, const char *filename){
  FILE * fout = fopen(filename, "wb");
  if (fout == NULL){
    std::string error_msg = std::string("Cannot open file ")
                          + std::string(filename)
                          + std::string(" !!!");
    throw std::runtime_error(error_msg);
  }
  for (vocab::iterator it=hash->begin(); it!=hash->end(); ++it)
    fprintf(fout, "%s %d\n", it->first.c_str(), it->second);
  fclose(fout);
}
