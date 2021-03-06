// This file is part of the dune-stuff project:
//   https://users.dune-project.org/projects/dune-stuff/
// Copyright holders: Rene Milk, Felix Schindler
// License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)

#ifndef DUNE_STUFF_LA_SOLVER_EIGEN_HH
#define DUNE_STUFF_LA_SOLVER_EIGEN_HH

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cmath>

#if HAVE_EIGEN
# include <dune/stuff/common/disable_warnings.hh>
#   include <Eigen/Dense>
#   include <Eigen/SparseCore>
#   include <Eigen/IterativeLinearSolvers>
#   include <Eigen/SparseCholesky>
#   include <Eigen/SparseLU>
#   include <Eigen/SparseQR>
//#   if HAVE_UMFPACK
//#     include <Eigen/UmfPackSupport>
//#   endif
//#   include <Eigen/SPQRSupport>
//#   include <Eigen/CholmodSupport>
//#   if HAVE_SUPERLU
//#     include <Eigen/SuperLUSupport>
//#   endif
# include <dune/stuff/common/reenable_warnings.hh>
#endif // HAVE_EIGEN

#if HAVE_FASP
extern "C" {
# include "fasp_functs.h"
}
#endif // HAVE_FASP

#include <dune/stuff/common/exceptions.hh>
#include <dune/stuff/common/configtree.hh>
#include <dune/stuff/la/container/eigen.hh>

#include "../solver.hh"

namespace Dune {
namespace Stuff {
namespace LA {


#if HAVE_EIGEN

template< class S >
class Solver< EigenDenseMatrix< S > >
  : protected SolverUtils
{
public:
  typedef EigenDenseMatrix< S > MatrixType;

  Solver(const MatrixType& matrix)
    : matrix_(matrix)
  {}

  static std::vector< std::string > options()
  {
    return {
             "lu.partialpiv"
           , "qr.householder"
           , "llt"
           , "ldlt"
           , "qr.colpivhouseholder"
           , "qr.fullpivhouseholder"
           , "lu.fullpiv"
           };
  } // ... options()

  static Common::ConfigTree options(const std::string& type)
  {
    SolverUtils::check_given(type, options());
    Common::ConfigTree default_options({"type", "post_check_solves_system"},
                                       {type,   "1e-5"});
    // * for symmetric matrices
    if (type == "ldlt" || type == "llt") {
      default_options.set("pre_check_symmetry", "1e-8");
    }
    return default_options;
  } // ... options(...)


  template< class T1, class T2 >
  void apply(const EigenBaseVector< T1, S >& rhs, EigenBaseVector< T2, S >& solution) const
  {
    apply(rhs, solution, options()[0]);
  }

  template< class T1, class T2 >
  void apply(const EigenBaseVector< T1, S >& rhs, EigenBaseVector< T2, S >& solution, const std::string& type) const
  {
    apply(rhs, solution, options(type));
  }

  template< class T1, class T2 >
  void apply(const EigenBaseVector< T1, S >& rhs,
             EigenBaseVector< T2, S >& solution,
             const Common::ConfigTree& opts) const
  {
    if (!opts.has_key("type"))
      DUNE_THROW_COLORFULLY(Exceptions::configuration_error,
                            "Given options (see below) need to have at least the key 'type' set!\n\n" << opts);
    const auto type = opts.get< std::string >("type");
    SolverUtils::check_given(type, options());
    const Common::ConfigTree default_opts = options(type);
    // check for symmetry (if solver needs it)
    if (type == "ldlt" || type == "llt") {
      const S pre_check_symmetry_threshhold = opts.get("pre_check_symmetry",
                                                       default_opts.get< S >("pre_check_symmetry"));
      if (pre_check_symmetry_threshhold > 0) {
        const MatrixType tmp(matrix_.backend() - matrix_.backend().transpose());
        // serialize difference to compute L^\infty error (no copy done here)
        const S error = std::max(std::abs(tmp.backend().minCoeff()), std::abs(tmp.backend().maxCoeff()));
        if (error > pre_check_symmetry_threshhold)
          DUNE_THROW_COLORFULLY(Exceptions::linear_solver_failed_bc_matrix_did_not_fulfill_requirements,
                                "Given matrix is not symmetric and you requested checking (see options below)!\n"
                                << "If you want to disable this check, set 'pre_check_symmetry = 0' in the options.\n\n"
                                << "  (A - A').sup_norm() = " << error << "\n\n"
                                << "Those were the given options:\n\n"
                                << opts);
      }
    }
    // solve
    if (type == "qr.colpivhouseholder") {
      solution.backend() = matrix_.backend().colPivHouseholderQr().solve(rhs.backend());
    } else if (type == "qr.fullpivhouseholder")
      solution.backend() = matrix_.backend().fullPivHouseholderQr().solve(rhs.backend());
    else if (type == "qr.householder")
      solution.backend() = matrix_.backend().householderQr().solve(rhs.backend());
    else if (type == "lu.fullpiv")
      solution.backend() = matrix_.backend().fullPivLu().solve(rhs.backend());
    else if (type == "llt")
      solution.backend() = matrix_.backend().llt().solve(rhs.backend());
    else if (type == "ldlt")
      solution.backend() = matrix_.backend().ldlt().solve(rhs.backend());
    else if (type == "lu.partialpiv")
      solution.backend() = matrix_.backend().partialPivLu().solve(rhs.backend());
    else
      DUNE_THROW_COLORFULLY(Exceptions::internal_error,
                            "Given type '" << type << "' is not supported, although it was reported by options()!");
    // check
    const S post_check_solves_system_theshhold = opts.get("post_check_solves_system",
                                                          default_opts.get< S >("post_check_solves_system"));
    if (post_check_solves_system_theshhold > 0) {
      auto tmp = rhs.copy();
      tmp.backend() = matrix_.backend() * solution.backend() - rhs.backend();
      const S sup_norm = tmp.sup_norm();
      if (sup_norm > post_check_solves_system_theshhold || std::isnan(sup_norm) || std::isinf(sup_norm))
        DUNE_THROW_COLORFULLY(Exceptions::linear_solver_failed_bc_the_solution_does_not_solve_the_system,
                              "The computed solution does not solve the system (although the eigen backend reported "
                              << "'Success') and you requested checking (see options below)!\n"
                              << "If you want to disable this check, set 'post_check_solves_system = 0' in the options."
                              << "\n\n"
                              << "  (A * x - b).sup_norm() = " << tmp.sup_norm() << "\n\n"
                              << "Those were the given options:\n\n"
                              << opts);
    }
  } // ... apply(...)

private:
  const MatrixType& matrix_;
}; // class Solver


/**
 *  \note lu.sparse will copy the matrix to column major
 *  \note qr.sparse will copy the matrix to column major
 *  \note ldlt.simplicial will copy the matrix to column major
 *  \note llt.simplicial will copy the matrix to column major
 */
template< class S >
class Solver< EigenRowMajorSparseMatrix< S > >
  : protected SolverUtils
{
  typedef ::Eigen::SparseMatrix< S, ::Eigen::ColMajor > ColMajorBackendType;
public:
  typedef EigenRowMajorSparseMatrix< S > MatrixType;


  Solver(const MatrixType& matrix)
    : matrix_(matrix)
  {}

  static std::vector< std::string > options()
  {
    return { "bicgstab.ilut"
           , "lu.sparse"
           , "llt.simplicial"          // <- does only work with symmetric matrices
           , "ldlt.simplicial"         // <- does only work with symmetric matrices
           , "bicgstab.diagonal"       // <- slow for complicated matrices
           , "bicgstab.identity"       // <- slow for complicated matrices
           , "qr.sparse"               // <- produces correct results, but is painfully slow
           , "cg.diagonal.lower"       // <- does only work with symmetric matrices, may produce correct results
           , "cg.diagonal.upper"       // <- does only work with symmetric matrices, may produce correct results
           , "cg.identity.lower"       // <- does only work with symmetric matrices, may produce correct results
           , "cg.identity.upper"       // <- does only work with symmetric matrices, may produce correct results
//           , "spqr"                  // <- does not compile
//           , "llt.cholmodsupernodal" // <- does not compile
#if HAVE_UMFPACK
//           , "lu.umfpack"            // <- untested
#endif
#if HAVE_SUPERLU
//           , "superlu"               // <- untested
#endif
    };
  } // ... options()

  static Common::ConfigTree options(const std::string& type)
  {
    // check
    SolverUtils::check_given(type, options());
    // default config
    Common::ConfigTree default_options({"type", "post_check_solves_system"},
                                       {type,   "1e-5"});
    Common::ConfigTree iterative_options({"max_iter", "precision"},
                                         {"10000",    "1e-10"});
    iterative_options += default_options;
    // direct solvers
    if (type == "lu.sparse" || type == "qr.sparse" || type == "lu.umfpack" || type == "spqr"
        || type == "llt.cholmodsupernodal" || type == "superlu")
      return default_options;
    // * for symmetric matrices
    if (type == "ldlt.simplicial" || type == "llt.simplicial") {
      default_options.set("pre_check_symmetry", "1e-8");
      return default_options;
    }
    // iterative solvers
    if (type == "bicgstab.ilut") {
      iterative_options.set("preconditioner.fill_factor", "10");
      iterative_options.set("preconditioner.drop_tol", "1e-4");
    } else if (type.substr(0, 3) == "cg.")
      iterative_options.set("pre_check_symmetry", "1e-8");
    return iterative_options;
  } // ... options(...)

  template< class T1, class T2 >
  void apply(const EigenBaseVector< T1, S >& rhs, EigenBaseVector< T2, S >& solution) const
  {
    apply(rhs, solution, options()[0]);
  }

  template< class T1, class T2 >
  void apply(const EigenBaseVector< T1, S >& rhs, EigenBaseVector< T2, S >& solution, const std::string& type) const
  {
    apply(rhs, solution, options(type));
  }

  template< class T1, class T2 >
  void apply(const EigenBaseVector< T1, S >& rhs,
             EigenBaseVector< T2, S >& solution,
             const Common::ConfigTree& opts) const
  {
    if (!opts.has_key("type"))
      DUNE_THROW_COLORFULLY(Exceptions::configuration_error,
                            "Given options (see below) need to have at least the key 'type' set!\n\n" << opts);
    const auto type = opts.get< std::string >("type");
    SolverUtils::check_given(type, options());
    const Common::ConfigTree default_opts = options(type);
    // check for symmetry (if solver needs it)
    if (type.substr(0, 3) == "cg." || type == "ldlt.simplicial" || type == "llt.simplicial") {
      const S pre_check_symmetry_threshhold = opts.get("pre_check_symmetry",
                                                       default_opts.get< S >("pre_check_symmetry"));
      if (pre_check_symmetry_threshhold > 0) {
        ColMajorBackendType colmajor_copy(matrix_.backend());
        colmajor_copy -= matrix_.backend().transpose();
        // serialize difference to compute L^\infty error (no copy done here)
        EigenMappedDenseVector< S > differences(colmajor_copy.valuePtr(), colmajor_copy.nonZeros());
        if (differences.sup_norm() > pre_check_symmetry_threshhold)
          DUNE_THROW_COLORFULLY(Exceptions::linear_solver_failed_bc_matrix_did_not_fulfill_requirements,
                                "Given matrix is not symmetric and you requested checking (see options below)!\n"
                                << "If you want to disable this check, set 'pre_check_symmetry = 0' in the options.\n\n"
                                << "  (A - A').sup_norm() = " << differences.sup_norm() << "\n\n"
                                << "Those were the given options:\n\n"
                                << opts);
      }
    }
    ::Eigen::ComputationInfo info;
    if (type == "cg.diagonal.lower") {
      typedef ::Eigen::ConjugateGradient< typename MatrixType::BackendType,
                                          ::Eigen::Lower,
                                          ::Eigen::DiagonalPreconditioner< S > > SolverType;
      SolverType solver(matrix_.backend());
      solver.setMaxIterations(opts.get("max_iter", default_opts.get< std::size_t >("max_iter")));
      solver.setTolerance(opts.get("precision", default_opts.get< S >("precision")));
      solution.backend() = solver.solve(rhs.backend());
      info = solver.info();
    } else if (type == "cg.diagonal.upper") {
      typedef ::Eigen::ConjugateGradient< typename MatrixType::BackendType,
                                          ::Eigen::Upper,
                                          ::Eigen::DiagonalPreconditioner< double > > SolverType;
      SolverType solver(matrix_.backend());
      solver.setMaxIterations(opts.get("max_iter", default_opts.get< std::size_t >("max_iter")));
      solver.setTolerance(opts.get("precision", default_opts.get< S >("precision")));
      solution.backend() = solver.solve(rhs.backend());
      info = solver.info();
    } else if (type == "cg.identity.lower") {
      typedef ::Eigen::ConjugateGradient< typename MatrixType::BackendType,
                                          ::Eigen::Lower,
                                          ::Eigen::IdentityPreconditioner > SolverType;
      SolverType solver(matrix_.backend());
      solver.setMaxIterations(opts.get("max_iter", default_opts.get< std::size_t >("max_iter")));
      solver.setTolerance(opts.get("precision", default_opts.get< S >("precision")));
      solution.backend() = solver.solve(rhs.backend());
      info = solver.info();
    } else if (type == "cg.identity.upper") {
      typedef ::Eigen::ConjugateGradient< typename MatrixType::BackendType,
                                          ::Eigen::Lower,
                                          ::Eigen::IdentityPreconditioner > SolverType;
      SolverType solver(matrix_.backend());
      solver.setMaxIterations(opts.get("max_iter", default_opts.get< std::size_t >("max_iter")));
      solver.setTolerance(opts.get("precision", default_opts.get< S >("precision")));
      solution.backend() = solver.solve(rhs.backend());
      info = solver.info();
    } else if (type == "bicgstab.ilut") {
      typedef ::Eigen::BiCGSTAB< typename MatrixType::BackendType, ::Eigen::IncompleteLUT< S > > SolverType;
      SolverType solver(matrix_.backend());
      solver.setMaxIterations(opts.get("max_iter", default_opts.get< std::size_t >("max_iter")));
      solver.setTolerance(opts.get("precision", default_opts.get< S >("precision")));
      solver.preconditioner().setDroptol(opts.get("preconditioner.drop_tol",
                                                  default_opts.get< S >("preconditioner.drop_tol")));
      solver.preconditioner().setFillfactor(opts.get("preconditioner.fill_factor",
                                                     default_opts.get< size_t >("preconditioner.fill_factor")));
      solution.backend() = solver.solve(rhs.backend());
      info = solver.info();
    } else if (type == "bicgstab.diagonal") {
      typedef ::Eigen::BiCGSTAB< typename MatrixType::BackendType, ::Eigen::DiagonalPreconditioner< S > > SolverType;
      SolverType solver(matrix_.backend());
      solver.setMaxIterations(opts.get("max_iter", default_opts.get< std::size_t >("max_iter")));
      solver.setTolerance(opts.get("precision", default_opts.get< S >("precision")));
      solution.backend() = solver.solve(rhs.backend());
      info = solver.info();
    } else if (type == "bicgstab.identity") {
      typedef ::Eigen::BiCGSTAB< typename MatrixType::BackendType, ::Eigen::IdentityPreconditioner > SolverType;
      SolverType solver(matrix_.backend());
      solver.setMaxIterations(opts.get("max_iter", default_opts.get< std::size_t >("max_iter")));
      solver.setTolerance(opts.get("precision", default_opts.get< S >("precision")));
      solution.backend() = solver.solve(rhs.backend());
      info = solver.info();
    } else if (type == "lu.sparse") {
      ColMajorBackendType colmajor_copy(matrix_.backend());
      colmajor_copy.makeCompressed();
      typedef ::Eigen::SparseLU< ColMajorBackendType > SolverType;
      SolverType solver;
      solver.analyzePattern(colmajor_copy);
      solver.factorize(colmajor_copy);
      solution.backend() = solver.solve(rhs.backend());
      info = solver.info();
    } else if (type == "qr.sparse") {
      ColMajorBackendType colmajor_copy(matrix_.backend());
      colmajor_copy.makeCompressed();
      typedef ::Eigen::SparseQR< ColMajorBackendType, ::Eigen::COLAMDOrdering< int > > SolverType;
      SolverType solver;
      solver.analyzePattern(colmajor_copy);
      solver.factorize(colmajor_copy);
      solution.backend() = solver.solve(rhs.backend());
      info = solver.info();
    } else if (type == "ldlt.simplicial") {
      ColMajorBackendType colmajor_copy(matrix_.backend());
      colmajor_copy.makeCompressed();
      typedef ::Eigen::SimplicialLDLT< ColMajorBackendType > SolverType;
      SolverType solver;
      solver.analyzePattern(colmajor_copy);
      solver.factorize(colmajor_copy);
      solution.backend() = solver.solve(rhs.backend());
      info = solver.info();
    } else if (type == "llt.simplicial") {
      ColMajorBackendType colmajor_copy(matrix_.backend());
      colmajor_copy.makeCompressed();
      typedef ::Eigen::SimplicialLLT< ColMajorBackendType > SolverType;
      SolverType solver;
      solver.analyzePattern(colmajor_copy);
      solver.factorize(colmajor_copy);
      solution.backend() = solver.solve(rhs.backend());
      info = solver.info();
#if HAVE_UMFPACK
    } else if (type == "lu.umfpack") {
      typedef ::Eigen::UmfPackLU< typename MatrixType::BackendType > SolverType;
      SolverType solver;
      solver.analyzePattern(matrix_.backend());
      solver.factorize(matrix_.backend());
      solution.backend() = solver.solve(rhs.backend());
      info = solver.info();
#endif // HAVE_UMFPACK
//    } else if (type == "spqr") {
//      ColMajorBackendType colmajor_copy(matrix_.backend());
//      colmajor_copy.makeCompressed();
//      typedef ::Eigen::SPQR< ColMajorBackendType > SolverType;
//      SolverType solver;
//      solver.analyzePattern(colmajor_copy);
//      solver.factorize(colmajor_copy);
//      solution.backend() = solver.solve(rhs.backend());
//      if (solver.info() != ::Eigen::Success)
//        return solver.info();
//    } else if (type == "cholmodsupernodalllt") {
//      typedef ::Eigen::CholmodSupernodalLLT< typename MatrixType::BackendType > SolverType;
//      SolverType solver;
//      solver.analyzePattern(matrix_.backend());
//      solver.factorize(matrix_.backend());
//      solution.backend() = solver.solve(rhs.backend());
//      if (solver.info() != ::Eigen::Success)
//        return solver.info();
#if HAVE_SUPERLU
    } else if (type == "superlu") {
      typedef ::Eigen::SuperLU< typename MatrixType::BackendType > SolverType;
      SolverType solver;
      solver.analyzePattern(matrix_.backend());
      solver.factorize(matrix_.backend());
      solution.backend() = solver.solve(rhs.backend());
      info = solver.info();
#endif // HAVE_SUPERLU
    } else
      DUNE_THROW_COLORFULLY(Exceptions::internal_error,
                            "Given type '" << type << "' is not supported, although it was reported by options()!");
    // handle eigens info
    if (info != ::Eigen::Success) {
      if (info == ::Eigen::NumericalIssue)
        DUNE_THROW_COLORFULLY(Exceptions::linear_solver_failed_bc_matrix_did_not_fulfill_requirements,
                              "The eigen backend reported 'NumericalIssue'!\n"
                              << "=> see http://eigen.tuxfamily.org/dox/group__enums.html#ga51bc1ac16f26ebe51eae1abb77bd037b for eigens explanation\n"
                              << "Those were the given options:\n\n"
                              << opts);
      else if (info == ::Eigen::NoConvergence)
        DUNE_THROW_COLORFULLY(Exceptions::linear_solver_failed_bc_it_did_not_converge,
                              "The eigen backend reported 'NoConvergence'!"
                              << "=> see http://eigen.tuxfamily.org/dox/group__enums.html#ga51bc1ac16f26ebe51eae1abb77bd037b for eigens explanation\n"
                              << "Those were the given options:\n\n"
                              << opts);
      else if (info == ::Eigen::InvalidInput)
        DUNE_THROW_COLORFULLY(Exceptions::linear_solver_failed_bc_it_was_not_set_up_correctly,
                              "The eigen backend reported 'InvalidInput'!"
                              << "=> see http://eigen.tuxfamily.org/dox/group__enums.html#ga51bc1ac16f26ebe51eae1abb77bd037b for eigens explanation\n"
                              << "Those were the given options:\n\n"
                              << opts);
      else
        DUNE_THROW_COLORFULLY(Exceptions::internal_error,
                              "The eigen backend reported an unknown status!\n"
                              << "Please report this to the dune-stuff developers!");
    }
    // check
    const S post_check_solves_system_theshhold = opts.get("post_check_solves_system",
                                                          default_opts.get< S >("post_check_solves_system"));
    if (post_check_solves_system_theshhold > 0) {
      auto tmp = rhs.copy();
      tmp.backend() = matrix_.backend() * solution.backend() - rhs.backend();
      const S sup_norm = tmp.sup_norm();
      if (sup_norm > post_check_solves_system_theshhold || std::isnan(sup_norm) || std::isinf(sup_norm))
        DUNE_THROW_COLORFULLY(Exceptions::linear_solver_failed_bc_the_solution_does_not_solve_the_system,
                              "The computed solution does not solve the system (although the eigen backend reported "
                              << "'Success') and you requested checking (see options below)!\n"
                              << "If you want to disable this check, set 'post_check_solves_system = 0' in the options."
                              << "\n\n"
                              << "  (A * x - b).sup_norm() = " << tmp.sup_norm() << "\n\n"
                              << "Those were the given options:\n\n"
                              << opts);
    }
  } // ... apply(...)

private:
  const MatrixType& matrix_;
}; // class Solver


#else // HAVE_EIGEN


template< class S >
class Solver< EigenDenseMatrix< S > >{ static_assert(Dune::AlwaysFalse< S >::value, "You are missing Eigen!"); };

template< class S >
class Solver< EigenRowMajorSparseMatrix< S > >{ static_assert(Dune::AlwaysFalse< S >::value,
                                                              "You are missing Eigen!"); };


#endif // HAVE_EIGEN

} // namespace LA
} // namespace Stuff
} // namespace Dune

#endif // DUNE_STUFF_LA_SOLVER_EIGEN_HH
