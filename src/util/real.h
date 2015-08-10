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
/*--------------------------------------------------------------------*/
/**
 * @file 		   real.h
 * @date		   29 déc. 2009
 * @author		 iovleff, serge.iovleff@stkpp.org
 * @brief 		 define the type REAL
 */


#ifndef REAL_H_
#define REAL_H_

#include <cfloat>
#include <limits>

/**
 * @ingroup Utility
 * @{
 */

/* Real type is float */
#define REALT float
#define REALT_EPSILON FLT_EPSILON
/*
#define REALT double
#define REALT_EPSILON DBL_EPSILON
*/
#ifdef __cplusplus
#define NaN std::numeric_limits<REALT>::quiet_NaN()
#else
static const unsigned long raw = 0x7fc00000;
#define NaN *((float*)&raw)
/* double */


#endif

/** @} */

#endif /* REAL_H_ */
