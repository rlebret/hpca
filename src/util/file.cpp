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
#include "constants.h"

// C++ header
#include <stdexcept>
#include <string>
#include <cstring>
#include <cstdlib>
#include <fstream>

/** Destructor */
File::~File() {
  if(flines) free(flines);
}

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
  stream = fopen ( file_name.c_str(), "rb" );
  // get the first two bytes
  int byte1, byte2;
  byte1 = fgetc(stream);
  byte2 = fgetc(stream);
  // close file
  fclose(stream);
  return ( (byte1 == 0x1f) && (byte2 == 0x8b) );
}

/** Return file byte size ?
 **/
long int File::size()
{
    if (fsize == 0){ // size never been computed before
        FILE * fin = fopen(file_name.c_str(),"rb");
        if (fin == NULL){
            std::string error_msg = std::string("Data file ")
            + file_name
            + std::string(" opening error !!!\n");
            throw std::runtime_error(error_msg);
        }
        if (gzip()){
            fseek(fin, -4, SEEK_END);
            int b4 = fgetc(fin);
            int b3 = fgetc(fin);
            int b2 = fgetc(fin);
            int b1 = fgetc(fin);
            fsize = (b1 << 24) | (b2 << 16) + (b3 << 8) + b4;
        }else{
            fseek(fin, 0, SEEK_END);
            fsize=ftell(fin);
        }
        fclose(fin);
        if (fsize == 0){
            std::string error_msg = std::string("Data file ")
            + file_name
            + std::string(" is empty !!!\n");
            throw std::runtime_error(error_msg);
        }
    }
    return fsize;
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
      if ((os = fopen(file_name.c_str(), "rb")) == NULL)
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
  char *line=NULL;
  (zip) 
  ? line=get_next_gzline(gzos) 
  : line=get_next_line(os);
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

/** split file into n parts
 **/
void File::split(const int npart){
    const long int sp = fsize/npart;
    // allocation
    flines = (long int*)malloc(sizeof(long int)*(npart+1));
    flines[0] = 0; // set start
    flines[npart]=fsize; // set end
    char *line = NULL;
    if (npart>1){
        // open file
        open();
        // get the split
        if ( zip ){
            for(int i=1;i<npart; i++){
                gzseek(gzos, i*sp, SEEK_SET);
                line = get_next_gzline(gzos);
                free(line);
                flines[i] = gztell(gzos);
            }
        }else{
            for(int i=1;i<npart; i++){
                fseek(os, i*sp, SEEK_SET);
                line = get_next_line(os);
                free(line);
                flines[i] = ftell(os);
            }
        }
        close();
        
    }
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
  while( (line = (zip) ? get_next_gzline(gzos) : get_next_line(os)) != NULL )
  {  n++;  free(line); }
  
  // close file
  close();
  // return the number of lines
  return n;
}

/** get the number of columns
 **/
int File::number_of_column(const char delim, bool const header){
    // open the file
    open();
    if (header) skip_header();
    char *line=NULL;
    (zip) 
    ? line=get_next_gzline(gzos) 
    : line=get_next_line(os);

    char *olds = line;
    char olddelim = delim;
    int counter=0;
    while(olddelim && *line) {
        while(*line && (delim != *line)) line++;
        *line ^= olddelim = *line; // olddelim = *line; *line = 0;
        counter++;
        *line++ ^= olddelim; // *line = olddelim; line++;
    }
    free(olds);
    close();
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
    (zip) 
    ? line=get_next_gzline(gzos) 
    : line=get_next_line(os);
    if ( line!= NULL )
    {  ++cpt; }
    else
    {
      std::string error_msg = std::string("Cannot jump to line n° ")
                           + std::string( typeToString(n) + " - " + file_name )
                           + std::string(" has not enough lines !!!\n");
      throw std::runtime_error(error_msg);
    }
    free(line);
  }
 
}

long int const File::position(){
    if (zip) {
        return gztell(gzos);
    }else{
        return ftell(os);
    }
}

/** Jump the given position of file
 **/
void File::jump_to_position( const long int n )
{
    if (zip){
        gzseek(gzos, n, SEEK_SET);
    }else{
        fseek(os, n, SEEK_SET);
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
  //if ( os ) return fgets(line, MAX_STRING_LENGTH, os);
  //else return gzgets( gzos, line, MAX_STRING_LENGTH);
  if ( os ) return get_next_line(os);
  else return get_next_gzline(gzos);
}


/** Return next word in stream
 **/
int File::getword(char * word)
{
  if ( os ) return get_next_word( word, os );
  else return get_next_gzword( word, gzos );
}
