// Some file functions
//
// Copyright (c) 2009 PGXIS - UMR CNRS 8524
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
 * @file       file.h
 * @author     Remi Lebret
 * @brief	   some file functions
 */

#ifndef FILE_H_
#define FILE_H_

// C++ header
#include <stdio.h>
#include <zlib.h>
#include <string>
#include <stdarg.h>
/**
 *  @defgroup Utility
 *
 *  @brief all classes and functions used along a HPCA analysis.
 *  It defines classes for using files, multithreading.
 *  Constants, functions to handle string of characters.
 *
 *  @{
 *
 * 	@class File
 *
 * 	@brief a @c File object creates, opens and closes a file.
 * 	It can also make some operations on the file.
 */
class File
{
  public:
    /**< file name */
    std::string file_name;
    /**< file byte size */
    long int fsize;
    /**< pointer to lines */
    long int * flines;
    /**< file stream */
    FILE* os;
    /**< file stream with compression */
    gzFile gzos;
    /**< compression ? */
    bool zip;

    /**
     * 	@brief Constructor
     *
     * 	Create a @c File object.
     *
     * 	@param name the file name
     *  @param compression - boolean compression or not? false by default
     */
    File( std::string const & name
        , bool const compression=false
        )
        : file_name(name)
        , fsize(0)
        , os(0), gzos(0)
        , zip(compression)
    {}

    /**
     * 	@brief Destructor
     *
     * 	Release a @c File.
     */
    ~File() {}

    /**
     * 	@brief Open the file.
     *
     * 	Opening modes:
     * 	- "r": read only
     * 	- "w": write only
     * 	- "rw": read and write
     * 	- "a": append
     *
     * 	@param mode the opening mode. Read only by default.
     */
    void open( std::string mode="r" );

    /**
     * 	@brief Close the file
     */
    void close();

    /**
     *  @brief Say whether file is gzipped or not ?
     *
     *  @return boolean - true if gzipped, false otherwise.
     */
    bool gzip();
    
    /**
     *  @brief Return file byte size ?
     *
     *  @return the byte size
     */
    long int size();

    /**
     * 	@brief Skip the header
     */
    void skip_header();
    
    /** 
      * @brief Split file into n parts
      *
      * @param npart number of parts
     **/
    void split(const int npart);
    
    /**
     * 	@brief Count the number of lines into the file
     *
     * 	@return the number of lines
     */
    int number_of_line();
    
    /**
     * 	@brief Count the number of columns into the file
     *
     *  @param delim char which defines the column delimiter
     *  @param skip_header boolean - true if header file needs to be skipped, false by default.
     *
     * 	@return the number of columns
     */
    int number_of_column(const char delim, bool const header=false);
    
    /**
     * 	@brief Jump the @a n first line of file.
     *
     * 	@param n the number of lines to jump
     */
    void jump_to_line( const int n );
    
    /**
     * 	@brief Return the current position in file
     *
     * 	@return the current position in stream
     */
    long int const position();
    
    /**
     * 	@brief Jump the @a n position of file.
     *
     * 	@param n the position to jump
     */
    void jump_to_position( const long int n );

    /**
     * 	@brief Check whether a file is readable.
     *  http://cpp.developpez.com/faq/cpp/?page=fichiers#FICHIERS_existence
     *
     *  @return a boolean - true if the file is readable, false otherwise
     */
    bool is_readable();

    /**
     *  @brief write into file
     *
     *  @param str data to write
     */
    int write( char const * str );

    /**
     *  @brief Return next line in stream
     */
    char * getline();

    /**
     *  @brief Return next word in stream
     *
     *  @param word where to store next word
     *  @return 0 if end of line, 1 otherwise
     */
    int getword(char * word);

    /**
     *  @brief Flush a stream
     */
    int flush();

    /**
     *  @brief Accessor
     *
     *  Get outputs file name.
     *
     *  @param filename in/out output name
     *  @param compress - boolean compression or not? false by default
     */
    static std::string const get_file_name( std::string const & filename
                                          , bool const compress=false
                                          );
};

/** @} */

#endif /* FILE_H_ */
