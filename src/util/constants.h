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

/**
 * @file         contants.h
 * @author       Remi Lebret
 * @brief		 some utilities
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

/**
 * @ingroup Utility
 * @{
 */
#define false 0
#define true 1

/* No error*/
#define NOERROR         0
#define ERROR         	1

/* Errors */
#define ERR_CREATE       10
#define ERR_SYNTAX       11
#define ERR_FILE         12
#define ERR_VAR          13
#define ERR_PAR 	 	 14
#define ERR_CLOSE_FILE 	 15
#define ERR_OPEN_FILE 	 16

/* Note that the max number of characters includes '\0' */
/* Thus the effective number of characters in a filename is */
/* MAX_FILENAME-1 */
#define MAX_FILE_NAME          65
#define MAX_PATH_NAME          100
#define MAX_TOKEN              200
#define MAX_TOKEN_PER_LINE     512
#define MAX_STRING_LENGTH      2000

/* default size for hashtable */
#define TSIZE	1048576

#define EPSILON 0.001

#define GIGAOCTET 1073741824

/** @} */

#endif /*CONSTANTS_H_*/
