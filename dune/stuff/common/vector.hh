// This file is part of the dune-stuff project:
//   https://users.dune-project.org/projects/dune-stuff
// Copyright holders: Rene Milk, Felix Schindler
// License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)

#ifndef DUNE_STUFF_TOOLS_COMMON_VEECTOR_HH
#define DUNE_STUFF_TOOLS_COMMON_VEECTOR_HH

#include <iostream>

#include <dune/common/densevector.hh>
#include <dune/common/dynvector.hh>
#include <dune/common/fvector.hh>
#include <dune/common/deprecated.hh>

#include <dune/stuff/la/container/interfaces.hh>
#include <dune/stuff/la/container/eigen.hh>
#include "float_cmp.hh"

namespace Dune {
namespace Stuff {
namespace Common {

template< class VectorImp >
inline void clear(Dune::DenseVector< VectorImp >& vector)
{
  vector *= typename Dune::DenseVector< VectorImp >::value_type(0);
}

template< class T >
inline void clear(LA::VectorInterface< T >& vector)
{
  vector *= typename LA::VectorInterface< T >::ScalarType(0);
}

/**
 *  \brief Compare x and y component-wise for almost equality.
 *
 *  Applies Dune::FloatCmp::eq() componentwise.
 */
template< class Field, int size >
bool DUNE_DEPRECATED_MSG("Use Dune::Stuff::Common::FloatCMP::eq() instead!") float_cmp(const Dune::FieldVector< Field, size >& x,
                                     const Dune::FieldVector< Field, size >& y,
                                     const Field tol)
{
  return FloatCmp::eq(x, y, tol);
}

template< class T >
DUNE_DEPRECATED_MSG("THIS WILL BE REMOVED ONCE ExtendedParameterTree::getVector() IS PROPERLY IMPLEMENTED!")
Dune::DynamicVector< T > resize(const Dune::DynamicVector< T >& inVector,
                                const size_t newSize,
                                const T fill = T(0))
{
  Dune::DynamicVector< T > outVector(newSize);
  for (size_t ii = 0; ii < std::min(inVector.size(), newSize); ++ii)
    outVector[ii] = inVector[ii];
  for (size_t ii = std::min(inVector.size(), newSize); ii < newSize; ++ii)
    outVector[ii] = fill;
  return outVector;
}

} // namespace Common
} // namespace Stuff
} // namespace Dune

#endif // DUNE_STUFF_TOOLS_COMMON_VEECTOR_HH
