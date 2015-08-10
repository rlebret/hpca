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
