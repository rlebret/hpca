/**
 * [https://gist.github.com/tonious/1377667]
 * 
 * @file       hashtable.h
 * @author     Tony Thompson
 * @brief	   hashtable functions
 */

#ifndef HASHTABLE_H_
#define HASHTABLE_H_


/**
 * 	@ingroup Utility
 * 	@{
 *
 * 	@struct entry_s
 *
 *	@brief a entry_s object contains:
 *  @var key 
 *  a key (a string of characters)
 *  @var value 
 *  a value (integer)
 *  @var next 
 *  a pointer to the next entry.
 */
struct entry_s {
	char *key;
	unsigned int value;
	struct entry_s *next;
};
 
typedef struct entry_s entry_t;
 
/** 
 * 	@struct hashtable_s
 *
 *	@brief a hashtable_s object contains:
 *  @var size 
 *  the size of the hashtable
 *  @var table 
 *  an array of entry_s
 */
struct hashtable_s {
	int size;
	struct entry_s **table;	
};
 
typedef struct hashtable_s hashtable_t;

/**
 * @brief Create a new hashtable. 
 *
 * @param size size of the hashtable
 *
 * @return a new @c hashtable_t object
 **/
hashtable_t *ht_create( int size );

/**
 * @brief Delete a hashtable. 
 *
 * @param hashtable pointer to a @c hashtable_t object
 **/
void ht_delete( hashtable_t *hashtable );


/**
 * @brief Hash a string for a particular hash table.
 *
 * @param hashtable pointer to a @c hashtable_t object
 * @param key a string of characters containing a hashtable key
 **/
int ht_hash( hashtable_t *hashtable, char *key );

/**
 * @brief Create a key pair.
 *
 * @param key a string of characters containing a hashtable key
 * 
 * @return a new @c entry_t object
 **/
entry_t *ht_newpair( char *key );

/**
 * @brief Create a key-value pair. 
 *
 * @param key a string of characters containing a hashtable key
 * @param value an integer to associate with the key
 *
 * @return a new @c entry_t object
 */
entry_t *ht_newpair( char *key, unsigned int value );

/**
 * @brief Insert a key into a hash table. 
 *
 *
 **/
void ht_insert( hashtable_t *hashtable, char *key );

/** 
 * @brief Insert a key-value pair into a hash table. 
 *
 * @param hashtable pointer to a @c hashtable_t object
 * @param key a string of characters containing a hashtable key
 * @param value an integer to associate with the key
 **/
void ht_insert( hashtable_t *hashtable, char *key, unsigned int value );

/**
 * @brief Retrieve a key-value pair from a hash table. 
 *
 * @param hashtable pointer to a @c hashtable_t object
 * @param key a string of characters containing a hashtable key
 *
 * @return the value
 **/
int ht_get( hashtable_t *hashtable, char *key ) ;

/**
 * @brief Function to print nodes in a given linked list.
 *
 * @param filename string of characters containing the filename
 * @param hashtable pointer to a @c hashtable_t object
 **/
void ht_print(const char *filename, hashtable_t* hashtable);


/** @} */

#endif /* HASHTABLE_H_ */