#ifndef DUNE_STUFF_GRID_SEARCH_HH
#define DUNE_STUFF_GRID_SEARCH_HH

#include <vector>

#include <boost/range/iterator_range.hpp>

#if HAVE_DUNE_GEOMETRY
  #include <dune/geometry/referenceelements.hh>
#else
  #include <dune/grid/common/genericreferenceelements.hh>
#endif
#include <dune/grid/common/gridview.hh>

namespace Dune {
namespace Stuff {
namespace Grid {


template< class GridViewType >
class EntitySearchBase
{
  typedef typename GridViewType::Traits ViewTraits;
  static_assert(std::is_base_of< GridView< ViewTraits >, GridViewType >::value,
                "GridViewType has to be derived from GridView!");
public:
  typedef typename ViewTraits::template Codim<0>::Entity EntityType;
  typedef typename EntityType::Geometry::LocalCoordinate LocalCoordinateType;
  typedef typename EntityType::Geometry::GlobalCoordinate GlobalCoordinateType;
  typedef std::vector< typename EntityType::EntityPointer > EntityPointerVectorType;
}; // class EntitySearchBase


template< class GridViewType >
class EntityInlevelSearch
  : public EntitySearchBase< GridViewType >
{
  typedef EntitySearchBase< GridViewType > BaseType;
  typedef GenericReferenceElements< typename  BaseType::LocalCoordinateType::value_type,
                                              BaseType::LocalCoordinateType::dimension > RefElementType;
  typedef typename GridViewType::template Codim< 0 >::Iterator IteratorType;
public:
  typedef typename BaseType::EntityPointerVectorType EntityPointerVectorType;

private:
  inline bool check_add(const typename BaseType::EntityType& entity,
                        const typename BaseType::GlobalCoordinateType& point,
                        EntityPointerVectorType& ret) const {
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
  EntityInlevelSearch(const GridViewType& gridview)
    : gridview_(gridview)
    , it_last_(gridview_.template begin< 0 >())
  {}

  template < class PointContainerType >
  EntityPointerVectorType operator() (const PointContainerType& points)
  {
    const auto max_size = points.size();

    const IteratorType begin = gridview_.template begin< 0 >();
    const IteratorType end = gridview_.template end< 0 >();
    EntityPointerVectorType ret;
    for(const auto& point : points)
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
  } // ... operator()

private:
  const GridViewType& gridview_;
  IteratorType it_last_;
}; // class EntityInlevelSearch


template< class GridViewType >
class EntityHierarchicSearch
  : public EntitySearchBase< GridViewType >
{
  typedef EntitySearchBase< GridViewType > BaseType;

  const GridViewType& gridview_;
  const int start_level_;

public:
  EntityHierarchicSearch(const GridViewType& gridview)
    : gridview_(gridview)
    , start_level_(0)
  {}

  typedef typename BaseType::EntityPointerVectorType EntityPointerVectorType;

  template< class PointContainerType >
  EntityPointerVectorType operator() (const PointContainerType& points) const
  {
    auto level = std::min(gridview_.grid().maxLevel(), start_level_);
    auto range = DSC::viewRange(gridview_.grid().levelView(level));
    return process(points, range);
  }

private:
  template <class QuadpointContainerType, class RangeType>
  EntityPointerVectorType process(const QuadpointContainerType& quad_points,
                                  const RangeType& range) const
  {
    typedef GenericReferenceElements< typename BaseType::LocalCoordinateType::value_type,
        BaseType::LocalCoordinateType::dimension > RefElementType;
    EntityPointerVectorType ret;

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
}; // class EntityHierarchicSearch


} // namespace Grid
} // namespace Stuff
} // namespace Dune

#endif // DUNE_STUFF_GRID_SEARCH_HH
