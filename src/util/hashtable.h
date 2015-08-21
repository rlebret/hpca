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

#include "constants.h"

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
	char *key;
	unsigned int value;
};
 
typedef struct entry_s entry_t;
 
/** 
 * 	@class Hashtable
 *
 *	@brief a @c Hashtable object contains
 *  an array of key-value pairs (entry_t struct)
 *  along with a hashing function to retrieve elements
 *  in a quick manner.
 */
class Hashtable
{
  private:
    /**< the hash maximum size*/
	long int hash_size_;
    /**< array containing hashing index */
    long int *hash_;
    /**< the size of the hashtable */
    long int size_;
    /**< the size of the hashtable */
    long int max_size_;
    /**< min_shrink */
    int min_shrink_;
    /**< an array of entry_t */
	entry_t *table_;
 
  public:
    
    
    /**
     *  @brief Constructor
     *
     *  Create a @c Hashtable
     */
    Hashtable(const long int size, const long int max_size = MAX_HASH_SIZE);
    
    
    /**
     *  @brief Destructor
     *
     *  Delete a @c Hashtable
     */
    ~Hashtable();
       
    
    /**
     *  @brief Accessor
     *
     *  Get the hashtable size.
     *
     *  @return the size
     */
    inline int long const size() const
    {  return size_;  }
    
    
    /**
     *  @brief Accessor
     *
     *  Get the hashtable size.
     *
     *  @return the size
     */
    inline void increment(const long int idx)
    {  table_[idx].value++; }
    
    /**
     * @brief Hash a string for a particular hash table.
     *
     * @param key a string of characters containing a hashtable key
     * @return the hash value
     **/
    long int const hash( const char *key );
    
    
    /**
     * @brief Insert a key into a hash table.
     *
     * @param key a string of characters containing a hashtable key
     *
     * @return the new index
     **/
    long int const insert( const char *key );
    
    /**
     * @brief Insert a key-value pair into a hash table.
     *
     * @param key a string of characters containing a hashtable key
     * @param value the value to add up
     **/
    void insert( const char *key, const int value );
    
    /**
     * @brief Retrieve a hashing index from a key.
     *
     * @param key a string of characters containing a hashtable key
     *
     * @return the hashing index
     **/
    long int const get( const char *key ) ;
    
    /**
     * @brief Retrieve a key-value pair from a hash table.
     *
     * @param key a string of characters containing a hashtable key
     *
     * @return the value
     **/
    int const value( const char *key ) ;
    
    /**
     * @brief Shrink the hashtable by removing infrequent values
     **/
    void shrink() ;
    
    /**
     * @brief Compare two @c entry_t objects.
     *
     * @param a pointer to a @c entry_t object
     * @param b pointer to a @c entry_t object
     *
     * @return positive value if a>b, negative value otherwise
     **/
    static int compare( const void *a, const void *b ) ;
    
    /**
     * @brief Sorts hashtable by values (descending order).
     **/
    void sort() ;
    
    /**
     * @brief Function to print hashtable (key value\n).
     *
     * @param filename string of characters containing the filename
     **/
    void print(const char *filename);
};


/** @} */

#endif /* HASHTABLE_H_ */