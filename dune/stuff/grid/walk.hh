// This file is part of the dune-stuff project:
//   https://users.dune-project.org/projects/dune-stuff
// Copyright holders: Rene Milk, Felix Schindler
// License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)

#ifndef DUNE_STUFF_WALK_HH_INCLUDED
#define DUNE_STUFF_WALK_HH_INCLUDED

#include <dune/common/static_assert.hh>
#include <dune/common/fvector.hh>
#include <dune/common/deprecated.hh>
#include <dune/stuff/aliases.hh>
#include <dune/stuff/common/math.hh>
#include <dune/stuff/common/misc.hh>
#include <dune/stuff/common/ranges.hh>
#include <dune/grid/common/geometry.hh>
#include <dune/stuff/aliases.hh>

#include <vector>
#include <boost/format.hpp>

namespace Dune {
namespace Stuff {
namespace Grid {

/** \brief Useful dummy functor if you don't have anything to do on entities/intersections
 **/
struct GridWalkDummyFunctor {
  GridWalkDummyFunctor(){}

  template < class Entity >
  void operator() ( const Entity&, const int) const
  {}
  template < class Entity, class Intersection >
  void operator() ( const Entity&, const Intersection&) const
  {}
};

namespace {
  /** \brief global \ref GridWalkDummyFunctor instance
   **/
  const GridWalkDummyFunctor gridWalkDummyFunctor;
}

/** \brief applies Functors on each \ref Entity/\ref Intersection of a given \ref GridView
 *  \todo allow stacking of functor to save gridwalks?
 *  \tparam GridViewImp any \ref GridView interface compliant type
 *  \tparam codim determines the codim of the Entities that are iterated on
 **/
template < class GridViewImp, int codim = 0 >
class GridWalk {
  typedef Dune::GridView<typename GridViewImp::Traits> GridViewType;
public:
  GridWalk ( const GridViewType& gp )
    : gridView_( gp )
  {}

  /** \param entityFunctor is applied on all codim 0 entities presented by \var gridView_
   *  \param intersectionFunctor is applied on all Intersections of all codim 0 entities
   *        presented by \var gridView_
   *  \note only instantiable for codim == 0
   */
  template < class EntityFunctor, class IntersectionFunctor >
  void operator () ( EntityFunctor& entityFunctor, IntersectionFunctor& intersectionFunctor ) const
  {
    dune_static_assert( codim == 0, "walking intersections is only possible for codim 0 entities" );
    for (const auto& entity : DSC::viewRange(gridView_)) {
      const int entityIndex = gridView_.indexSet().index(entity);
      entityFunctor( entity, entityIndex);
      for (const auto& intersection : DSC::intersectionRange(gridView_, entity)) {
        intersectionFunctor( entity, intersection);
      }
    }
  }

  /** \param entityFunctor is applied on all codim entities presented by \var gridView_
   *  \note only instantiable for codim < GridView::dimension
   */
  template < class EntityFunctor >
  void operator () ( EntityFunctor& entityFunctor ) const
  {
    dune_static_assert( codim <= GridViewType::dimension, "codim too high to walk" );
    for (const auto& entity : DSC::viewRange(gridView_)) {
      const int entityIndex = gridView_.indexSet().index(entity);
      entityFunctor( entity, entityIndex);
    }
  }

  template< class Functor >
  void walkCodim0(Functor& f) const DUNE_DEPRECATED_MSG("use operator()(Functor) instead ");


private:
  const GridViewType& gridView_;
};

template< class V, int i >
template< class Functor >
void GridWalk<V,i>::walkCodim0(Functor& f) const {
  this->operator()(f);
}

//!
template <class ViewImp, int codim = 0>
GridWalk< Dune::GridView<ViewImp>, codim > make_gridwalk(const Dune::GridView<ViewImp>& view) {
    return GridWalk< Dune::GridView<ViewImp>, codim >(view);
}

} // namespace Grid
} // namespace Stuff
} // namespace Dune

#endif // ifndef DUNE_STUFF_WALK_HH_INCLUDED
