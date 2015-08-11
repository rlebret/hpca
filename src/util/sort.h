// Sorting numbers functions
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
 * @file        sort.h
 * @author      Remi Lebret
 * @brief       sort number public interface.
 */
// C++ header
#include <vector>
// utilities header
#include "real.h"

/**
 *  @ingroup Utility
 *  @{
 *
 *  @brief Swap two reals within a vector and swap their indexes.
 *
 *  @param tab vector containing reals
 *  @param ind vector containing indexes
 *  @param i the first real index
 *  @param j the second real index
 */
void swap (std::vector<REALT> & tab, std::vector<int> & ind, const int i, const int j);

/**
 *  @brief Initialize indexes vector.
 *
 *  @param ind vector containing indexes
 */
void initialize_index( std::vector<int> & ind );

/**
 *  @brief Sort a vector of reals and their indexes.
 *
 *  @param tab vector containing reals
 *  @param ind vector containing indexes
 *  @param D the first real index
 *  @param G the last real index
 */
void sort_real (std::vector<REALT> & tab, std::vector<int> & ind, const int G, const int D);
