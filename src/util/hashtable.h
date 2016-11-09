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
/**
 *
 * @file       hashtable.h
 * @author     Rémi Lebret
 * @brief	   hashtable functions
 */

#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#include "sparsepp.h"
using spp::sparse_hash_map;
typedef sparse_hash_map<std::string,unsigned int> vocab;


/**
 * 	@ingroup Utility
 * 	@{
 *
 * 	@struct entry_s structure for storing key-value pairs
 *
 *	@brief a entry_s object contains:
 *  @var key
 *  a key (a string of characters)
 *  @var value
 *  a value (integer)
 */
struct entry_s {
	const char *key;
	unsigned int value;
};

typedef struct entry_s entry_t;

int hash_compare(const void *a, const void *b);

entry_t const * hash_sort(vocab *hash);

void hash_print(const vocab *hash, const char *filename);

/** @} */

#endif /* HASHTABLE_H_ */
