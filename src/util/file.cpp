// Some file functions
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

// HPCA C++ header
#include "file.h"
#include "util.h"
#include "convert.h"

// C++ header
#include <stdexcept>
#include <string>
#include <cstring>
#include <cstdlib>
#include <fstream>


/** Get outputs file name
 **/
std::string const File::get_file_name( std::string const & filename
                                     , bool const  compress
                                     )
{
  int fileid=0;
  // create file
  File file( ( compress ) ? filename + ".gz" : filename );

  while ( file.is_readable() ) // while file already exists
  {
    file.file_name = filename + "-" + typeToString(++fileid) ;
    if ( compress ) file.file_name += ".gz";
  }

  // return new file name
  std::string idname =  ( fileid ) ? "-" + typeToString(fileid) : "";
  return filename + idname;
}

/** Say whether file is gzipped or not ?
 **/
bool File::gzip()
{
  FILE * stream;
  // open file
  stream = fopen ( file_name.c_str(), "r" );
  // get the first two bytes
  int byte1, byte2;
  byte1 = fgetc(stream);
  byte2 = fgetc(stream);
  // close file
  fclose(stream);
  return ( (byte1 == 0x1f) && (byte2 == 0x8b) );
}

/** file opening
 **/
void File::open( std::string mode )
{
  if ( mode == "r" )
  {
    if ( gzip() )
    {
      if ( ( gzos = gzopen(file_name.c_str(), "rb") ) == NULL)
      {
        std::string error_msg = std::string("Data file ")
                              + file_name
                              + std::string(" opening error !!!\n");
        throw std::runtime_error(error_msg);
      }
      zip = true;
    }
    else
    {
      if ((os = fopen(file_name.c_str(), "r")) == NULL)
      {
        std::string error_msg = std::string("Data file ")
                              + file_name
                              + std::string(" opening error !!!\n");
        throw std::runtime_error(error_msg);
      }
      zip = false;
    }
  }
  else
  {
    if ( zip )
    {
      if ((gzos = gzopen(typeToString(file_name+".gz").c_str(), typeToString(mode+"b6").c_str())) == NULL)
      {
        std::string error_msg = std::string("Data file ")
                              + file_name + ".gz"
                              + std::string(" opening error !!!\n");
        throw std::runtime_error(error_msg);
      }
    }
    else
    {
      if ((os = fopen(file_name.c_str(), mode.c_str())) == NULL)
      {
        std::string error_msg = std::string("Data file ")
                              + file_name
                              + std::string(" opening error !!!\n");
        throw std::runtime_error(error_msg);
      }
    }
  }
}

/** file closing
 **/
void File::close()
{
  if ( zip )
  {
    if (gzclose(gzos) != Z_OK)
    {
      std::string error_msg = std::string("Data file ")
                            + file_name + ".gz"
                            + std::string(" closing error detected !!!\n");
      throw std::runtime_error(error_msg);
    }
  }
  else
  {
    if (fclose(os) == EOF)
    {
      std::string error_msg = std::string("Data file ")
                            + file_name
                            + std::string(" closing error detected !!!\n");
      throw std::runtime_error(error_msg);
    }
  }
}

/** skip the header
 **/
void File::skip_header()
{
  // variable to store the first line
  char *line = NULL;
  //get the first line
  line = getline();
  if (line == NULL)
  {
    std::string error_msg = std::string("Data file ")
                          + file_name
                          + std::string(" is empty !!!\n");
    throw std::runtime_error(error_msg);
  }
  // release memory
  free(line);
}

/** get the number of line
 **/
int File::number_of_line()
{
  // open file
  open();
  int n=0;
  char* line=NULL;
  // count
  while( (line=getline()) )
  {  n++; free(line);  }
  // close file
  close();
  // return the number of lines
  return n;
}
/** get the number of columns
 **/
int File::number_of_column(const char delim, bool const header){
    NULL;
    open();
    if (header) skip_header();
    char *line=getline();
    char *olds = line;
    char olddelim = delim;
    int counter=0;
    while(olddelim && *line) {
        while(*line && (delim != *line)) line++;
        *line ^= olddelim = *line; // olddelim = *line; *line = 0;
        counter++;
        *line++ ^= olddelim; // *line = olddelim; line++;
        olds = line;
    }
    return counter;

}

/** Jump the n first line of file
 **/
void File::jump_to_line( const int n )
{
  // initialize a counter
 int cpt=0;
 char * line=NULL;
 // jump to the first line of that thread
 while ( cpt < n )
 {
   if ( (line=getline()) != NULL )
   {  ++cpt; free(line);  }
   else
   {
     std::string error_msg = std::string("Cannot jump to line n° ")
                           + std::string( typeToString(n) + " - " + file_name )
                           + std::string(" has not enough lines !!!\n");
     throw std::runtime_error(error_msg);
   }
 }
}

/** Check whether the file is readable
 **/
bool File::is_readable()
{
  std::ifstream file( file_name.c_str() );
  return !file.fail();
}

/** Flush a stream
 **/
int File::flush()
{
  if ( os ) fflush(os);
  else
  {
    int flush = 0;
    gzflush(gzos,flush);
  }
  return 0;
}

/** Write into file
 **/
int File::write( char const * str )
{
   if ( os )
   {
       fprintf(os, "%s", str);
   }
   else
   {
     gzwrite(gzos, str,(unsigned)strlen(str));
   }
   return 0;
}

/** Return next line in stream
 **/
char * File::getline()
{
  if ( os ) return get_next_line( os );
  else return get_next_gzline( gzos );
}
