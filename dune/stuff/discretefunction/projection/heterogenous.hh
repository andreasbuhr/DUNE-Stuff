#ifndef DUNE_STUFF_DISCRETEFUNCTION_PROJECTION_DIRICHLET_HH
#define DUNE_STUFF_DISCRETEFUNCTION_PROJECTION_DIRICHLET_HH

#ifdef HAVE_CMAKE_CONFIG
 #include "cmake_config.h"
#else
 #include "config.h"
#endif // ifdef HAVE_CMAKE_CONFIG

#include <vector>

#if HAVE_DUNE_GEOMETRY
#include <dune/geometry/referenceelements.hh>
#else
#include <dune/grid/common/genericreferenceelements.hh>
#endif

#include <dune/common/shared_ptr.hh>
#include <dune/common/fvector.hh>
#include <dune/grid/common/backuprestore.hh>
#include <dune/grid/common/grid.hh>
#include <dune/grid/common/gridview.hh>
#include <dune/grid/common/entity.hh>
#include <dune/stuff/common/ranges.hh>
#include <dune/stuff/aliases.hh>

//#ifdef HAVE_DUNE_FEM
#include <dune/fem/function/common/discretefunction.hh>
#include <dune/fem/quadrature/cachingquadrature.hh>
#include <dune/fem/space/dgspace/localdgmassmatrix.hh>
//#endif

#include <boost/range/iterator_range.hpp>

namespace Dune {
namespace Stuff {

template <class ViewTraits>
class StrategyBase {
public:
  typedef typename ViewTraits::template Codim<0>::Entity EntityType;
  typedef typename EntityType::Geometry::LocalCoordinate LocalCoordinateType;
  typedef typename EntityType::Geometry::GlobalCoordinate GlobalCoordinateType;
  typedef std::vector<typename EntityType::EntityPointer> EntityPointerVector;
};

template <class ViewTraits>
class InlevelSearchStrategy : public StrategyBase<ViewTraits> {
  typedef StrategyBase<ViewTraits> BaseType;
  typedef GenericReferenceElements< typename  BaseType::LocalCoordinateType::value_type,
                                              BaseType::LocalCoordinateType::dimension >
        RefElementType;
  typedef typename Dune::GridView<ViewTraits>::template Codim< 0 >::Iterator IteratorType;


  inline bool check_add(const typename BaseType::EntityType& entity,
                        const typename BaseType::GlobalCoordinateType& point,
                        typename BaseType::EntityPointerVector& ret) const {
    const auto& geometry = entity.geometry();
    const auto& refElement = RefElementType::general(geometry.type());
    if(refElement.checkInside(geometry.local(point)))
    {
      ret.emplace_back(entity);
      return true;
    }
    return false;
  }
public:
  InlevelSearchStrategy(const Dune::GridView<ViewTraits>& gridview)
    : gridview_(gridview)
    , it_last_(gridview_.template begin< 0 >())
  {}

  template <class QuadpointContainerType>
  typename BaseType::EntityPointerVector operator() (const QuadpointContainerType& quad_points)
  {
    const auto max_size = quad_points.size();

    const IteratorType begin = gridview_.template begin< 0 >();
    const IteratorType end = gridview_.template end< 0 >();
    std::vector<typename BaseType::EntityType::EntityPointer> ret;
    for(const auto& point : quad_points)
    {
      IteratorType it_current = it_last_;
      bool it_reset = true;
      for(; it_current != end && ret.size() < max_size; ++it_current)
      {
        if(check_add(*it_current, point, ret)) {
          it_reset = false;
          it_last_ = it_current;
          break;
        }
      }
      if(!it_reset)
        continue;
      for(it_current = begin;
          it_current != it_last_ && ret.size() < max_size;
          ++it_current)
      {
        if(check_add(*it_current, point, ret)) {
          it_reset = false;
          it_last_ = it_current;
          break;
        }
      }
      assert(!it_reset);
    }
    return ret;
  }

private:
  const Dune::GridView<ViewTraits>& gridview_;
  IteratorType it_last_;
};

template <class ViewTraits>
class HierarchicSearchStrategy : public StrategyBase<ViewTraits> {
  typedef StrategyBase<ViewTraits> BaseType;

  const Dune::GridView<ViewTraits>& gridview_;
  const int start_level_;

public:
  HierarchicSearchStrategy(const Dune::GridView<ViewTraits>& gridview)
    : gridview_(gridview)
    , start_level_(0)
  {}

  template <class QuadpointContainerType>
  typename BaseType::EntityPointerVector
  operator() (const QuadpointContainerType& quad_points) const
  {
    auto level = std::min(gridview_.grid().maxLevel(), start_level_);
    auto range = DSC::viewRange(gridview_.grid().levelView(level));
    return process(quad_points, range);
  }

private:

  template <class QuadpointContainerType, class RangeType>
  std::vector<typename BaseType::EntityType::EntityPointer>
  process(const QuadpointContainerType& quad_points,
          const RangeType& range) const
  {
    typedef GenericReferenceElements< typename BaseType::LocalCoordinateType::value_type,
        BaseType::LocalCoordinateType::dimension > RefElementType;
    std::vector<typename BaseType::EntityType::EntityPointer> ret;

    for(const auto& my_ent : range) {
      const int my_level = my_ent.level();
      const auto& geometry = my_ent.geometry();
      const auto& refElement = RefElementType::general(geometry.type());
      for(const auto& point : quad_points)
      {
        if(refElement.checkInside(geometry.local(point)))
        {
          //if I cannot descend further add this entity even if it's not my view
          if(gridview_.grid().maxLevel() <= my_level || gridview_.contains(my_ent)) {
            ret.emplace_back(my_ent);
          }
          else {
            const auto h_end = my_ent.hend(my_level+1);
            const auto h_begin = my_ent.hbegin(my_level+1);
            const auto h_range = boost::make_iterator_range(h_begin, h_end);
            const auto kids = process(QuadpointContainerType(1, point), h_range);
            ret.insert(ret.end(), kids.begin(), kids.end());
          }
        }
      }
    }
    return ret;
  }
};

template <template <class> class SearchStrategy = InlevelSearchStrategy>
class HeterogenousProjection {

public:
  template < class SourceDFImp, class TargetDFImp >
  static void project(const Dune::DiscreteFunctionInterface<SourceDFImp>& source,
                      Dune::DiscreteFunctionInterface<TargetDFImp>& target)
{
  typedef SearchStrategy<typename SourceDFImp::GridType::LeafGridView::Traits> SearchStrategyType;
  typedef typename TargetDFImp::DiscreteFunctionSpaceType TargetDiscreteFunctionSpaceType;
  typedef typename TargetDFImp::DofType TargetDofType;
  static const int target_dimRange = TargetDiscreteFunctionSpaceType::dimRange;

  const auto& space =  target.space();
  const auto infinity = std::numeric_limits< TargetDofType >::infinity();

  // set all DoFs to infinity
  const auto dend = target.dend();
  for( auto dit = target.dbegin(); dit != dend; ++dit )
    *dit = infinity;

  SearchStrategyType search(source.gridPart().grid().leafView());
  const auto endit = space.end();
  for(auto it = space.begin(); it != endit ; ++it)
  {
    const auto& target_entity = *it;
    const auto& target_geometry = target_entity.geometry();
    auto target_local_function = target.localFunction(target_entity);
    const auto& target_lagrangepoint_set = space.lagrangePointSet(target_entity);
    const int quadNop = target_lagrangepoint_set.nop();

    typename TargetDiscreteFunctionSpaceType::RangeType source_value;

    std::vector<typename TargetDiscreteFunctionSpaceType::DomainType> global_quads(quadNop);
    for(int qP = 0; qP < quadNop ; ++qP) {
      global_quads[qP] = target_geometry.global(target_lagrangepoint_set.point(qP));
    }
    const auto evaluation_entities = search(global_quads);
    assert(evaluation_entities.size() == global_quads.size());

    int k = 0;
    for(int qP = 0; qP < quadNop ; ++qP)
    {
      if(target_local_function[ k ] == infinity)
      {
        const auto& global_point = global_quads[qP];
        // evaluate source function
        const auto source_entity = evaluation_entities[qP];
        const auto& source_geometry = source_entity->geometry();
        const auto& source_local_point = source_geometry.local(global_point);
        const auto& source_local_function = source.localFunction(*source_entity);
        source_local_function.evaluate(source_local_point, source_value);
        for(int i = 0; i < target_dimRange; ++i, ++k)
          target_local_function[k] = source_value[i];
      }
      else
        k += target_dimRange;
    }
  }
}
};

} // namespace Stuff
} // namespace Dune

#endif // DUNE_STUFF_DISCRETEFUNCTION_PROJECTION_DIRICHLET_HH