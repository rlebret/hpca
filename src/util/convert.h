/*--------------------------------------------------------------------*/
/*     Copyright (C) 2004-2007  Serge Iovleff

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this program; if not, write to the
    Free Software Foundation, Inc.,
    59 Temple Place,
    Suite 330,
    Boston, MA 02111-1307
    USA

    Contact : Serge.Iovleff@stkpp.org
*/
/**
 * @file 	  convert.h
 * @date 	  8 févr. 2010
 * @author  iovleff, serge.iovleff@stkpp.org
 * @brief   convert string to TYPE or TYPE to string
 */

#ifndef CONVERT_H_
#define CONVERT_H_

#include <sstream>

/**
 * 	@ingroup Utility
 * 	@{
 *
 * 	@brief convert a String to TYPE
 *
 *  This method return true if the String s could be converted into
 *  a correct TYPE t.
 *  http://www.codeguru.com/forum/showpost.php?p=678440&postcount=1
 *  http://c.developpez.com/faq/cpp/?page=strings#STRINGS_is_type
 *
 *  The conversion is successful if it does not remain
 *  char inside the string.
 *
 *  @param t The value to get from the string
 *  @param s the string to convert
 *  @param f flags
 *  @return boolean
 **/
template <class TYPE>
bool stringToType( TYPE  &t
                 , const std::string &s
                 , std::ios_base& (*f)(std::ios_base&) = std::dec
                 )
{
  std::istringstream iss(s);
  bool flag1 = (iss >> t).fail();
  // ok if the conversion success and the string is exhausted
  return ( !flag1 && iss.eof() );
}


/**
 * 	@brief convert a TYPE to String
 *
 *  This method return the TYPE t into a String s.
 *  @see http://www.codeguru.com/forum/showpost.php?p=678440&postcount=1
 *  @see http://c.developpez.com/faq/cpp/?page=strings#STRINGS_convertform
 *
 *  @param t The value to convert to String
 *  @param f flags
 *  @return a string
 **/
template <class TYPE>
std::string typeToString( const TYPE &t
                        , std::ios_base& (*f)(std::ios_base&) = std::dec
                        )
{
  std::ostringstream oss;
  oss << f << t;
  return oss.str();
}

/** @} */

#endif /* CONVERT_H_ */
