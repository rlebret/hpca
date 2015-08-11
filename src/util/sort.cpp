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


// C++ header
#include "sort.h"

/**
 * Swap two reals within a Matrix and swap the indexes
 */
void swap ( std::vector<REALT> & tab, std::vector<int> & ind, const int i, const int j )
{
	REALT val_buffer = tab[i];
	int ind_buffer = ind[i];

	//swap the values
	tab[i] = tab[j];
	tab[j] = val_buffer;

	//swap index
	ind[i] = ind[j];
	ind[j] = ind_buffer;
}

/** Initialize indexes vector
 **/
void initialize_index( std::vector<int> & ind )
{
  for( int i = 0; i<ind.size(); i++)
    ind[i] = i;
}

/**
 * Sort a vector of reals within a vector object.
 * Return the new indexes vector.
 */
void sort_real ( std::vector<REALT> & tab, std::vector<int> & ind, const int G, const int D )
{
  int g, d;
  REALT val;

  if (D <= G)
  {
	  return;
  }
  val = tab[D];
  g = G;
  d = D - 1;
  do
  {
    while ( tab[g] < val )
    {
    	g++;
    }
    while ( (tab[d] > val) && (d>G) )
    {
    	d--;
    }
    if (g < d)
    {
    	swap (tab, ind, g, d);
    }
  } while (g < d);
  swap (tab, ind, g, D);
  sort_real (tab, ind, G, g - 1);
  sort_real (tab, ind, g + 1, D);
}
