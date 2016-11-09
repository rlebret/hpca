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

 // include redsvd headers
 #include "../redsvd/redsvd.h"

 // Partially read matrix from file
 void read_eigen_truncated_matrix(
      const char *filename
    , Eigen::MatrixXf& result
    , const int dim
);

// Read matrix from file
void read_eigen_matrix(
     const char *filename
   , Eigen::MatrixXf& result
);

 // Read vector from file
 void read_eigen_vector(
     const char *filename
    , Eigen::VectorXf& result
    , const int dim
    , const float eig
);

// Write matrix into file
void write_eigen_matrix(
      const char *output_name
    , Eigen::MatrixXf matrix
);
