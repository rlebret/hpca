// Some utilities functions
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
 * @file         util.h
 * @author       Remi Lebret
 * @brief		 some utilities
 */

#ifndef UTIL_H_
#define UTIL_H_

// C++ header
#include <string>
#include <cstdio>
#include <zlib.h>


/**
 * 	@ingroup Utility
 * 	@{
 *
 * 	@brief Check whether a directory exists
 *
 *  @param path the path
 *  @return a boolean
 */
bool is_directory( const char * path );

/**
 * 	@brief Check whether a file exists
 *
 *  @param path the path
 *  @return a boolean
 */
bool is_file(const char * path );

/**
 * 	@brief Return full path of a file
 *
 *  @param dir path to the directory
 *  @param filename name of the file
 *  @return a string containing the full path
 */
char* const get_full_path(const char *dir, const char *filename);

/**
 *  @brief Accessor
 *
 *  Get the total of memory
 */
const unsigned long long int get_total_memory();

/**
 *  @brief Accessor
 *
 *  Get the total of memory
 */
const unsigned long long int get_total_swap();

/**
 *  @brief Accessor
 *
 *  Get the available memory
 */
unsigned long long int get_available_memory();

/**
 * 	@brief Read a line in a file
 *
 * 	@param stream the output stream
 *	@return the line
 */
char * get_next_line( FILE * stream );

/**
 *  @brief Read a line in a gzip file
 *
 *  @param stream the output stream
 *  @return the line
 */
char * get_next_gzline ( gzFile stream );

/**
 * 	@brief Read a word in a file
 *
 *  @param word where to store the output word
 * 	@param stream the output stream
 *  @return 0 if end of line, 1 otherwise
 */
int get_next_word( char * word, FILE * stream );

/**
 *  @brief Read a word in a gzip file
 *
 *  @param word where to store the output word
 *  @param stream the output stream
 *  @return 0 if end of line, 1 otherwise
 */
int get_next_gzword ( char * word, gzFile stream );


/**
 *  @brief Lowercase a line
 *
 *  @param p the input line
 */
int lowercase ( char* p );

/**
 *  @brief Check whether the character is a digit
 *
 *  @param a the given character
 */
int check_digit(char a);


/**
 *  @brief Replace all digits with the special character '0'
 *
 *  @param string the string to modify
 */
int replace_digit(char* string);


/**
 * 	@brief Copy a string from file to char*
 *
 *	@param dest the output
 *	@param src the input
 *	@param i the the line iterator
 *  @param delim the string delimiter
 */
char *string_copy(char * dest, const char * src, int * i, char const delim=',');

/**
 *  @brief Remove all occurrences of character @a C
 *
 *  @param Str the input
 *  @param C the character to remove
 */
void remove_all_characters( std::string & Str, char C );

/**
 *  @brief Remove all characters @a C at the beginning of @a Str
 *
 *  @param Str the input
 *  @param C the character to remove
 */
std::string remove_first_characters( const std::string & Str, char C );


/**
 *  @brief Find arguments from options in command line
 *
 *  @param str the argument to find
 *  @param argc the number of arguments
 *  @param argv the list of arguments
 */
int find_arg(char *str, int argc, char **argv);


/**
 *  @brief Display a progress bar
 *
 *  @param thread_id the thread id
 *  @param x the current status
 *  @param w width of the progress bar
 */
void loadbar(long int const thread_id, unsigned int const x, unsigned int n, unsigned int const w = 50);

/** @} */

#endif /*UTIL_H_*/
