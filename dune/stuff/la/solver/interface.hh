#ifndef DUNE_STUFF_LA_SOLVER_INTERFACE_HH
#define DUNE_STUFF_LA_SOLVER_INTERFACE_HH

#if HAVE_EIGEN
  #include <Eigen/Core>
  #include <Eigen/IterativeLinearSolvers>
#endif

#include <dune/common/exceptions.hh>
#include <dune/common/parametertree.hh>
#include <dune/stuff/la/container/interfaces.hh>

namespace Dune {
namespace Stuff {
namespace LA {


template< class MatrixImp, class VectorImp >
class SolverInterface
{
  static_assert(std::is_base_of< typename Dune::Stuff::LA::MatrixInterface< typename MatrixImp::Traits >,
                                 MatrixImp >::value,
                "MatrixImp has to be derived from Dune::Stuff::LA::MatrixInterface!");
  static_assert(std::is_base_of< typename Dune::Stuff::LA::VectorInterface< typename VectorImp::Traits >,
                                 VectorImp >::value,
                "MatrixImp has to be derived from Dune::Stuff::LA::MatrixInterface!");
  static_assert(std::is_same< typename MatrixImp::ScalarType,
                              typename VectorImp::ScalarType >::value, "Types do not match!");
public:
  typedef MatrixImp MatrixType;
  typedef VectorImp VectorType;
  typedef typename MatrixType::ScalarType ScalarType;

  // deprecated
  typedef ScalarType ElementType;

  virtual ~SolverInterface()
  {}

  virtual size_t apply(const MatrixType& /*_systemMatrix*/,
                       const VectorType& /*_rhsVector*/,
                       VectorType& /*_solutionVector*/,
                       const Dune::ParameterTree /*description*/) const = 0;
#if HAVE_EIGEN
  size_t translateInfo(const ::Eigen::ComputationInfo& info) const
  {
    switch (info) {
      case ::Eigen::Success: return 0;
      case ::Eigen::NoConvergence: return 1;
      case ::Eigen::NumericalIssue: return 2;
      default: return 3;
    }
  }
#endif

  static Dune::ParameterTree defaultIterativeSettings()
  {
    Dune::ParameterTree description;
    description["maxIter"] = "5000";
    description["precision"] = "1e-12";
    description["verbose"] = "0";
    return description;
  }
}; // class Interface


template< class MatrixImp, class VectorImp >
class SolverNotImplementedForThisMatrixVectorCombination
  : public SolverInterface< MatrixImp, VectorImp >
{
public:
  typedef typename SolverInterface< MatrixImp, VectorImp >::MatrixType  MatrixType;
  typedef typename SolverInterface< MatrixImp, VectorImp >::VectorType  VectorType;
  typedef typename SolverInterface< MatrixImp, VectorImp >::ScalarType  ScalarType;

  SolverNotImplementedForThisMatrixVectorCombination(
      const std::string msg = "\nERROR: this solver is not implemented for this matrix/vector combination")
  {
    DUNE_THROW(Dune::NotImplemented, msg);
  }

  virtual size_t apply(const MatrixType& /*_systemMatrix*/,
                       const VectorType& /*_rhsVector*/,
                       VectorType& /*_solutionVector*/,
                       const Dune::ParameterTree /*description*/ = Dune::ParameterTree()) const
  {
    return size_t(3);
  }
}; // class SolverNotImplementedForThisMatrixVectorCombination


template< class MatrixImp, class VectorImp, class Enable = void >
class CgSolver
  : public SolverNotImplementedForThisMatrixVectorCombination< MatrixImp, VectorImp >
{};

template< class MatrixImp, class VectorImp, class Enable = void >
class CgDiagonalSolver
  : public SolverNotImplementedForThisMatrixVectorCombination< MatrixImp, VectorImp >
{};

template< class MatrixImp, class VectorImp, class Enable = void >
class BicgstabSolver
  : public SolverNotImplementedForThisMatrixVectorCombination< MatrixImp, VectorImp >
{};

template< class MatrixImp, class VectorImp, class Enable = void >
class BicgstabDiagonalSolver
  : public SolverNotImplementedForThisMatrixVectorCombination< MatrixImp, VectorImp >
{};

template< class MatrixImp, class VectorImp, class Enable = void >
class BicgstabILUTSolver
  : public SolverNotImplementedForThisMatrixVectorCombination< MatrixImp, VectorImp >
{};

template< class MatrixImp, class VectorImp, class Enable = void >
class DirectSuperLuSolver
  : public SolverNotImplementedForThisMatrixVectorCombination< MatrixImp, VectorImp >
{};

#if HAVE_FASP
template< class MatrixImp, class VectorImp, class Enable = void >
class AmgSolver
  : public SolverNotImplementedForThisMatrixVectorCombination< MatrixImp, VectorImp >
{};
#endif // HAVE_FASP


} // namespace LA
} // namespace Stuff
} // namespace Dune

#endif // DUNE_STUFF_LA_SOLVER_INTERFACE_HH
