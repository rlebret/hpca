/**
 * @file       hashtable.h
 * @author     Remi Lebret
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

char *strdup(const char *s);

/* Create a new hashtable. */
hashtable_t *ht_create( int size );

/* Delete a hashtable. */
void ht_delete( hashtable_t *hashtable );

/* Hash a string for a particular hash table. */
int ht_hash( hashtable_t *hashtable, char *key );

/* Create a key pair. */
entry_t *ht_newpair( char *key );
/* Create a key-value pair. */
entry_t *ht_newpair( char *key, unsigned int value );

/* Insert a key into a hash table. */
void ht_insert( hashtable_t *hashtable, char *key );
/* Insert a key-value pair into a hash table. */
void ht_insert( hashtable_t *hashtable, char *key, unsigned int value );

/* Retrieve a key-value pair from a hash table. */
int ht_get( hashtable_t *hashtable, char *key ) ;

/* Function to print nodes in a given linked list */
void ht_print(const char *filename, hashtable_t* hashtable);


/** @} */

#endif /* HASHTABLE_H_ */