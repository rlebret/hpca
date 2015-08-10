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

#include "data.h"


/* Write cooccurrence records to file, accumulating duplicate entries */
int write(cooccur_t *cr, unsigned long long length, FILE *fout) {
    unsigned long long a = 0;
    cooccur_t old = cr[a];
    
    for(a = 1; a < length; a++) {
        if(cr[a].idx1 == old.idx1 && cr[a].idx2 == old.idx2) {
            old.val += cr[a].val;
            continue;
        }
        fwrite(&old, sizeof(cooccur_t), 1, fout);
        old = cr[a];
    }
    fwrite(&old, sizeof(cooccur_t), 1, fout);
    return 0;
}

/* Check if two cooccurrence records are for the same two words, used for qsort */
int compare(const void *a, const void *b) {
    int c;
    if( (c = ((cooccur_t *) a)->idx1 - ((cooccur_t *) b)->idx1) != 0) return c;
    else return (((cooccur_t *) a)->idx2 - ((cooccur_t *) b)->idx2);
    
}


/* Check if two cooccurrence records are for the same two words */
int compare_id(cooccur_id_t a, cooccur_id_t b) {
    int c;
    if( (c = a.idx1 - b.idx1) != 0) return c;
    else return a.idx2 - b.idx2;
}

/* Swap two entries of priority queue */
void swap_entry(cooccur_id_t *pq, int i, int j) {
    cooccur_id_t temp = pq[i];
    pq[i] = pq[j];
    pq[j] = temp;
}

/* Insert entry into priority queue */
void insert_pq(cooccur_id_t *pq, cooccur_id_t new_id, int size) {
    int j = size - 1, p;
    pq[j] = new_id;
    while( (p=(j-1)/2) >= 0 ) {
        if(compare_id(pq[p],pq[j]) > 0) {swap_entry(pq,p,j); j = p;}
        else break;
    }
}

/* Delete entry from priority queue */
void delete_pq(cooccur_id_t *pq, int size) {
    int j, p = 0;
    pq[p] = pq[size - 1];
    while( (j = 2*p+1) < size - 1 ) {
        if(j == size - 2) {
            if(compare_id(pq[p],pq[j]) > 0) swap_entry(pq,p,j);
            return;
        }
        else {
            if(compare_id(pq[j], pq[j+1]) < 0) {
                if(compare_id(pq[p],pq[j]) > 0) {swap_entry(pq,p,j); p = j;}
                else return;
            }
            else {
                if(compare_id(pq[p],pq[j+1]) > 0) {swap_entry(pq,p,j+1); p = j + 1;}
                else return;
            }
        }
    }
}


/* Write top node of priority queue to file, accumulating duplicate entries */
int merge_write(cooccur_id_t new_id, cooccur_id_t *old_id, FILE *fout) {
    if(new_id.idx1 == old_id->idx1 && new_id.idx2 == old_id->idx2) {
        old_id->val += new_id.val;
        return 0; // Indicates duplicate entry
    }
    fwrite(old_id, sizeof(cooccur_t), 1, fout);
    *old_id = new_id;
    return 1; // Actually wrote to file
}
