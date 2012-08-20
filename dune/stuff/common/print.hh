#ifndef DUNESTUFF_PRINTING_HH_INCLUDED
#define DUNESTUFF_PRINTING_HH_INCLUDED

#include <string>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <limits>
#include <boost/format.hpp>
#include <dune/stuff/fem/functions.hh>
#include <dune/stuff/common/parameter/container.hh>
#include <dune/istl/bcrsmatrix.hh>

namespace Dune {
namespace Stuff {
namespace Common {
namespace Print {

//! ensure matlab output is done with highest precision possible, otherwise weird effects are bound to happen
static const unsigned int matlab_output_precision = std::numeric_limits< double >::digits10 + 1;

/**
   *  \brief prints a Dune::FieldVector
   *
   *  or anything compatible in terms of Iterators
   *  \tparam T
   *          should be Dune::FieldVector or compatible
   *  \tparam stream
   *          std::ostream or compatible
   *  \param[in]  arg
   *          vector to be printed
   *  \param[in]  name
   *          name to be printed along
   *  \param[in]  out
   *          where to print
   *  \param[opt] prefix
   *          prefix to be printed before every line
   **/
template< class T, class stream >
void printFieldVector(T& arg, std::string name, stream& out, std::string prefix = "") {
  out << "\n" << prefix << "printing " << name << " (Dune::FieldVector)" << std::endl;
  typedef typename T::ConstIterator
  IteratorType;
  IteratorType itEnd = arg.end();
  out << prefix;
  for (IteratorType it = arg.begin(); it != itEnd; ++it)
  {
    out << std::setw(14) << std::setprecision(6) << *it;
  }
  out << '\n';
} // printFieldVector

/**
   *  \brief prints a Dune::FieldMatrix
   *
   *  or anything compatible in terms of Iterators
   *  \tparam T
   *          should be Dune::FieldVector or compatible
   *  \tparam stream
   *          std::ostream or compatible
   *  \param[in]  arg
   *          matrix to be printed
   *  \param[in]  name
   *          name to be printed along
   *  \param[in]  out
   *          where to print
   *  \param[opt] prefix
   *          prefix to be printed before every line
   **/
template< class T, class stream >
void printFieldMatrix(T& arg, std::string name, stream& out, std::string prefix = "") {
  out << "\n" << prefix << "printing " << name << " (Dune::FieldMatrix)";
  typedef typename T::ConstRowIterator
  RowIteratorType;
  typedef typename T::row_type::ConstIterator
  VectorInRowIteratorType;
  unsigned int row = 1;
  RowIteratorType rItEnd = arg.end();
  for (RowIteratorType rIt = arg.begin(); rIt != rItEnd; ++rIt)
  {
    out << "\n" << prefix << "  row " << row << ":";
    VectorInRowIteratorType vItEnd = rIt->end();
    for (VectorInRowIteratorType vIt = rIt->begin(); vIt != vItEnd; ++vIt)
    {
      out << std::setw(14) << std::setprecision(6) << *vIt;
    }
    row += 1;
  }
} // printFieldMatrix

/** \brief print a SparseRowMatrix (or any interface conforming object) to a given stream in matlab (laodable-) format
   * \ingroup Matlab
   **/
template< class T, class stream >
void printSparseRowMatrixMatlabStyle( const T& arg, std::string name, stream& out,
                                      const double eps = Dune::Stuff::Common::Parameter::Parameters().getParam("eps", 1e-14) ) {
  name = std::string("fem.") + name;
  const int I = arg.rows();
  const int J = arg.cols();
  out << boost::format("\n%s =sparse( %d, %d );") % name % I % J << std::endl;
  for (int row = 0; row < arg.rows(); row++)
  {
    for (int col = 0; col < arg.cols(); col++)
    {
      const typename T::Ttype val = arg(row, col);
      if (std::fabs(val) > eps)
        out << name << "(" << row + 1 << "," << col + 1 << ")=" << std::setprecision(matlab_output_precision) << val
            << ";\n";
    }
  }
} // printSparseRowMatrixMatlabStyle

/** \brief print a ISTLMatrix (or any interface conforming object) to a given stream in matlab (laodable-) format
   * \ingroup Matlab
   **/
template< class MatrixType, class stream >
void printISTLMatrixMatlabStyle( const MatrixType& arg, std::string name, stream& out,
                                 const double eps = Dune::Stuff::Common::Parameter::Parameters().getParam("eps", 1e-14) ) {
  name = std::string("istl.") + name;
  const int I = arg.N();
  const int J = arg.M();
  typedef typename MatrixType::block_type BlockType;
  out << boost::format("\n%s =sparse( %d, %d );") % name % (I* BlockType::rows) % (J* BlockType::cols) << std::endl;
  for (unsigned ii = 0; ii < I; ++ii)
  {
    for (unsigned jj = 0; jj < J; ++jj)
    {
      if ( arg.exists(ii, jj) )
      {
        const auto& block = arg[ii][jj];
        for (int i = 0; i < block.N(); ++i)
        {
          for (int j = 0; j < block.M(); ++j)
          {
            const auto value = block[i][j];
            if (std::fabs(value) > eps)
            {
              int real_row = BlockType::rows * ii + i + 1;
              int real_col = BlockType::cols * jj + j + 1;
              out << name << "(" << real_row << "," << real_col << ")="
                  << std::setprecision(matlab_output_precision) << value << ";\n";
            }
          }
        }
      }
    }
  }
} // printISTLMatrixMatlabStyle

/** \brief print a discrete function (or any interface conforming object) to a given stream in matlab (laodable-) format
   * \ingroup Matlab
   **/
template< class T, class stream >
void printDiscreteFunctionMatlabStyle(const T& arg, const std::string name, stream& out) {
  out << "\n" << name << " = [ " << std::endl;
  typedef typename T::ConstDofIteratorType
  ConstDofIteratorType;
  ConstDofIteratorType itEnd = arg.dend();
  for (ConstDofIteratorType it = arg.dbegin(); it != itEnd; ++it)
  {
    out << std::setprecision(matlab_output_precision) << *it;
    out << ";" << std::endl;
  }
  out << "];" << std::endl;
} // printDiscreteFunctionMatlabStyle

/** \brief print a double vector (or any interface conforming object) to a given stream in matlab (laodable-) format
   * \ingroup Matlab
   **/
template< class T, class stream >
void printDoubleVectorMatlabStyle(const T* arg, const int size, const std::string name, stream& out) {
  out << "\n" << name << " = [ " << std::endl;
  for (int i = 0; i < size; i++)
  {
    out << std::setprecision(matlab_output_precision) << arg[i];
    out << ";" << std::endl;
  }
  out << "];" << std::endl;
} // printDoubleVectorMatlabStyle

//! simple vector to stream print
template< class Type >
void printDoubleVec(std::ostream& stream, const Type* vec, const unsigned int N) {
  stream << "\n [ " << std::setw(5);
  for (unsigned int i = 0; i < N; ++i)
    stream << vec[i] << " ";

  stream << " ] " << std::endl;
} // printDoubleVec

//! simple discrete function to stream print
template< class DiscFunc >
void oneLinePrint(std::ostream& stream, const DiscFunc& func) {
  typedef typename DiscFunc::ConstDofIteratorType DofIteratorType;
  DofIteratorType it = func.dbegin();
  stream << "\n" << func.name() << ": \n[ ";
  for ( ; it != func.dend(); ++it)
  {
    // double d = 0.10;// + *it; //stupid hack cause setw/prec ain't working for me
    stream << std::setw(6) << std::setprecision(3) << *it << "  ";
  }
  stream << " ] " << std::endl;
} // oneLinePrint

/** \brief localmatrix printing functor for use in Stuff::GridWalk
   * putting this into Stuff::GridWalk::operator() will result in a local matrix being printed for each gird entity\n
   * Example:\n
   * Stuff::GridWalk<GridPartType> gw( gridPart_ );\n
   * Stuff::LocalMatrixPrintFunctor< RmatrixType,FunctorStream> f_R ( Rmatrix, functorStream, "R" );\n
   * gw( f_R );
   * \see Stuff::GridWalk
   * \ingroup GridWalk
   **/
template< class GlobalMatrix>
class LocalMatrixPrintFunctor
{
public:
  LocalMatrixPrintFunctor(const GlobalMatrix& m, std::ostream& stream, const std::string name)
    : matrix_(m)
      , stream_(stream)
      , name_(name)
  {}

  template< class Entity >
  void operator()(const Entity& en, const Entity& ne, const int en_idx, const int ne_idx) {
    typename GlobalMatrix::LocalMatrixType localMatrix
      = matrix_.localMatrix(en, ne);
    const int rows = localMatrix.rows();
    const int cols = localMatrix.columns();
    stream_ << "\nlocal_" << name_ << "_Matrix_" << en_idx << "_" << ne_idx << " = [" << std::endl;
    for (int i = 0; i < rows; ++i)
    {
      for (int j = 0; j < cols; ++j)
      {
        stream_ << std::setw(8) << std::setprecision(2) << localMatrix.get(i, j);
      }
      stream_ << ";" << std::endl;
    }
    stream_ << "];" << std::endl;
  } // ()

  void preWalk() {
    stream_ << "% printing local matrizes of " << name_ << std::endl;
  }

  void postWalk() {
    stream_ << "\n% done printing local matrizes of " << name_ << std::endl;
  }

private:
  const GlobalMatrix& matrix_;
  std::ostream& stream_;
  const std::string name_;
};

/** GridWalk functor to print all localfunctions of a given DiscreteFunction
   * \ingroup GridWalk
   **/
template< class DiscreteFunctionType, class QuadratureType >
class LocalFunctionPrintFunctor
{
public:
  LocalFunctionPrintFunctor(const DiscreteFunctionType& discrete_function, std::ostream& stream)
    : discrete_function_(discrete_function)
      , stream_(stream)
      , name_( discrete_function.name() )
  {}

  template< class Entity >
  void operator()(const Entity& en, const Entity& /*ne*/, const int /*en_idx*/, const int /*ne_idx */) {
    typename DiscreteFunctionType::LocalFunctionType lf = discrete_function_.localFunction(en);
    QuadratureType quad(en, 2 * discrete_function_.space().order() + 2);
    for (size_t qp = 0; qp < quad.nop(); ++qp)
    {
      typename DiscreteFunctionType::RangeType eval(0);
      typename DiscreteFunctionType::DomainType xLocal = quad.point(qp);
      typename DiscreteFunctionType::DomainType xWorld = en.geometry().global(xLocal);
      lf.evaluate(xLocal, eval);
      stream_ << boost::format("xWorld %f \t %s value %f\n") % xWorld % name_ % eval;
    }
  } // ()

  void preWalk() {
    stream_ << "% printing local function values of " << name_ << std::endl;
  }

  void postWalk() {
    stream_ << "\n% done printing function values of " << name_ << std::endl;
  }

private:
  const DiscreteFunctionType& discrete_function_;
  std::ostream& stream_;
  const std::string name_;
};

/** GridWalk functor to print, w/o transformation, all localfunctions of a given DiscreteFunction
   * \ingroup GridWalk
   **/
template< class DiscreteFunctionType >
class LocalFunctionVerbatimPrintFunctor
{
public:
  LocalFunctionVerbatimPrintFunctor(const DiscreteFunctionType& discrete_function, std::ostream& stream)
    : discrete_function_(discrete_function)
      , stream_(stream)
      , name_( discrete_function.name() )
  {}

  template< class Entity >
  void operator()(const Entity& en, const Entity& /*ne*/, const int /*en_idx*/, const int /*ne_idx */) {
    typename DiscreteFunctionType::LocalFunctionType lf = discrete_function_.localFunction(en);

    for (size_t qp = 0; qp < lf.numDofs(); ++qp)
    {
      stream_ << boost::format("%s dof %d value value %f\n") % name_ % qp % lf[qp];
    }
  } // ()

  void preWalk() {
    stream_ << "% printing local function values of " << name_ << std::endl;
  }

  void postWalk() {
    stream_ << "\n% done printing function values of " << name_ << std::endl;
  }

private:
  const DiscreteFunctionType& discrete_function_;
  std::ostream& stream_;
  const std::string name_;
};

/** print min/max of a given DiscreteFucntion obtained by Stuff::getMinMaxOfDiscreteFunction
   * \note hardcoded mult of values by sqrt(2)
   **/
template< class Function >
void printFunctionMinMax(std::ostream& stream, const Function& func) {
  double min = 0.0;
  double max = 0.0;

  Dune::Stuff::Fem::getMinMaxOfDiscreteFunction(func, min, max);
  stream << "  - " << func.name() << std::endl
         << "    min: " << std::sqrt(2.0) * min << std::endl
         << "    max: " << std::sqrt(2.0) * max << std::endl;
} // printFunctionMinMax

//! useful for visualizing sparsity patterns of matrices
template< class Matrix >
void matrixToGnuplotStream(const Matrix& matrix, std::ostream& stream) {
  unsigned long nz = 0;

  for (size_t row = 0; row < matrix.rows(); ++row)
  {
    for (size_t col = 0; col < matrix.cols(); ++col)
    {
      if ( matrix.find(row, col) )
        stream << row << "\t" << col << "\t" << matrix(row, col) << std::endl;
    }
    nz += matrix.numNonZeros(row);
    stream << "#non zeros in row " << row << " " << matrix.numNonZeros(row) << " (of " << matrix.cols() << " cols)\n";
  }
  stream << "#total non zeros " << nz << " of " << matrix.rows() * matrix.cols() << " entries\n";
} // matrixToGnuplotStream

//! proxy to Stuff::matrixToGnuplotStream that redirects its output to a file
template< class Matrix >
void matrixToGnuplotFile(const Matrix& matrix, std::string filename) {
  std::string dir(Dune::Stuff::Common::Parameter::Parameters().getParam( "fem.io.datadir", std::string("data") ) + "/gnuplot/");

  Dune::Stuff::Common::Filesystem::testCreateDirectory(dir);
  std::ofstream file( (dir + filename).c_str() );
  matrixToGnuplotStream(matrix, file);
  file.flush();
  file.close();
} // matrixToGnuplotFile

std::string dimToAxisName(const unsigned int dim, const bool capitalize = false) {
  char c = 'x';

  c += dim;
  if (capitalize)
    c -= 32;
  return std::string() += c;
} // dimToAxisName

} // namespace Print
} // namespace Common
} // namespace Stuff
} // namespace Dune

#endif // ifndef DUNESTUFF_PRINTING_HH_INCLUDED

/** Copyright (c) 2012, Felix Albrecht, Rene Milk     , Sven Kaulmann
   * All rights reserved.
   *
   * Redistribution and use in source and binary forms, with or without
   * modification, are permitted provided that the following conditions are met:
   *
   * 1. Redistributions of source code must retain the above copyright notice, this
   *    list of conditions and the following disclaimer.
   * 2. Redistributions in binary form must reproduce the above copyright notice,
   *    this list of conditions and the following disclaimer in the documentation
   *    and/or other materials provided with the distribution.
   *
   * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
   * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
   *
   * The views and conclusions contained in the software and documentation are those
   * of the authors and should not be interpreted as representing official policies,
   * either expressed or implied, of the FreeBSD Project.
   **/
