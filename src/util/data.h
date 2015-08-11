//  Tool to calculate word-word cooccurrence statistics
//
//  Copyright (c) 2014 The Board of Trustees of
//  The Leland Stanford Junior University. All Rights Reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
//
//  For more information, bug reports, fixes, contact:
//    Jeffrey Pennington (jpennin@stanford.edu)
//    GlobalVectors@googlegroups.com
//    http://www-nlp.stanford.edu/projects/glove/

/**
 * @file       data.h
 * @brief	   tool to calculate word-word cooccurrence statistics
 */

#ifndef __HPCA__data__
#define __HPCA__data__

#include <stdio.h>


/**
 * 	@ingroup Utility
 * 	@{
 *
 * 	@struct cooccur_t
 *
 *	@brief a cooccur object contains:
 *  @var idx1
 *  a first token index (an integer)
 *  @var idx2
 *  a second token index (an integer)
 *  @var value
 *  a weighting value (float)
 */
struct cooccur {
    unsigned int idx1;
    unsigned int idx2;
    float val;
};
typedef cooccur cooccur_t;

/**
 * 	@struct cooccur_id_t
 *
 *	@brief a cooccur_id object contains:
 *  @var idx1
 *  a first token index (an integer)
 *  @var idx2
 *  a second token index (an integer)
 *  @var value
 *  a weighting value (float)
 *  @var id
 *  a file id (unsigned int)
 */
struct cooccur_id {
    unsigned int idx1;
    unsigned int idx2;
    float val;
    unsigned int id;
};
typedef cooccur_id cooccur_id_t;


/**
 * @brief Write cooccurrence records to file, accumulating duplicate entries 
 *
 * @param cr pointer to @c cooccur_t object
 * @param length size of the object to write
 * @param fout file stream
 **/
int write(cooccur_t *cr, unsigned long long length, FILE *fout);

/**
 * @brief Check if two cooccurrence records are for the same two words, used for qsort
 *
 * @param a pointer to the first record
 * @param b pointer to the second record
 **/
int compare(const void *a, const void *b);

/**
 * @brief Check if two cooccurrence records are for the same two words
 *
 * @param a pointer to the first @c cooccur_id_t object
 * @param b pointer to the second @c cooccur_id_t object
 **/
int compare_id(cooccur_id_t a, cooccur_id_t b);


/**
 * @brief Swap two entries of priority queue
 *
 * @param pq pointer to a @c cooccur_id_t object
 * @param i first index
 * @param j second index
 **/
void swap_entry(cooccur_id_t *pq, int i, int j);


/**
 * @brief Insert entry into priority queue
 *
 * @param pq pointer to a @c cooccur_id_t object
 * @param size size of the priority queue
 **/
void insert_pq(cooccur_id_t *pq, cooccur_id_t new_id, int size);


/**
 * @brief Delete entry from priority queue
 *
 * @param pq pointer to a @c cooccur_id_t object
 * @param size size of the priority queue
 **/
void delete_pq(cooccur_id_t *pq, int size);


/**
 * @brief Write top node of priority queue to file, accumulating duplicate entries
 *
 * @param new_id a new @c cooccur_id_t object to merge
 * @param old_id pointer to @c cooccur_t object
 * @param fout file stream
 **/
int merge_write(cooccur_id_t new_id, cooccur_id_t *old_id, FILE *fout);

/** @} */

#endif /* defined(__HPCA__data__) */
