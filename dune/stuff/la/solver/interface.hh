#ifndef DUNE_STUFF_LA_SOLVER_INTERFACE_HH
#define DUNE_STUFF_LA_SOLVER_INTERFACE_HH

#ifdef HAVE_CMAKE_CONFIG
  #include "cmake_config.h"
#elif defined (HAVE_CONFIG_H)
  #include "config.h"
#endif // ifdef HAVE_CMAKE_CONFIG

#include <dune/stuff/la/container/interface.hh>

namespace Dune {
namespace Stuff {
namespace LA {
namespace Solver {


template< class MatrixImp, class VectorImp >
class Interface
{
public:
  typedef typename Dune::Stuff::LA::Container::MatrixInterface< typename MatrixImp::Traits > MatrixType;

  typedef typename MatrixType::ElementType ElementType;

  typedef typename Dune::Stuff::LA::Container::VectorInterface< typename VectorImp::Traits > VectorType;

  typedef typename MatrixType::size_type size_type;

  virtual void init(const MatrixType& /*systemMatrix*/) = 0;

  virtual bool apply(const VectorType& /*rhsVector*/,
                     VectorType& /*solutionVector*/,
                     const size_type /*maxIter*/,
                     const ElementType /*precision*/) const = 0;
}; // class Interface


} // namespace Solver
} // namespace LA
} // namespace Stuff
} // namespace Dune

#endif // DUNE_STUFF_LA_SOLVER_INTERFACE_HH