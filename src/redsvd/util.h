/*
 *  Copyright (c) 2011 Daisuke Okanohara
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *   1. Redistributions of source code must retain the above Copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above Copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the authors nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 */

#ifndef REDSVD_UTIL_HPP__
#define REDSVD_UTIL_HPP__

#define EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET

#include <vector>
#include "Eigen/Sparse"
#include "Eigen/Dense"
#include "Eigen/Eigenvalues"

namespace REDSVD {

typedef Eigen::SparseMatrix<float, Eigen::RowMajor> SMatrixXf;
typedef std::vector<std::pair<int, float> > fv_t;


class AscentCompareIndicesByAnotherVectorValues{
    Eigen::VectorXf& _values;
    public:
        AscentCompareIndicesByAnotherVectorValues(Eigen::VectorXf& values) : _values(values) {}
    public:
        bool operator() (const int& a, const int& b) const { return (_values)[a] < (_values)[b]; }
};
class DescentCompareIndicesByAnotherVectorValues{
    Eigen::VectorXf& _values;
    public:
        DescentCompareIndicesByAnotherVectorValues(Eigen::VectorXf& values) : _values(values) {}
    public:
        bool operator() (const int& a, const int& b) const { return (_values)[a] > (_values)[b]; }
};

class Util{
public:
  static void convertFV2Mat(const std::vector<fv_t>& fvs, SMatrixXf& A);
  static void sampleGaussianMat(Eigen::MatrixXf& x);
  static void processGramSchmidt(Eigen::MatrixXf& mat);
  static double getSec();
  static std::vector<int> ascending_order(Eigen::VectorXf& values);
  static std::vector<int> descending_order(Eigen::VectorXf& values);

private:
  static void sampleTwoGaussian(float& f1, float& f2);
};

}

#endif // REDSVD_UTIL_HPP_
