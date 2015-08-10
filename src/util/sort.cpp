/* ============================================================================
 * Name        : sort.cpp
 * Author      : Remi Lebret
 * Copyright   : Idiap Research Institute
 * Description : sort number functions
 * ============================================================================
 */

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
