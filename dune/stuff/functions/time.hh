// This file is part of the dune-stuff project:
//   https://users.dune-project.org/projects/dune-stuff
// Copyright holders: Rene Milk, Felix Schindler
// License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)

#ifndef DUNE_STUFF_FUNCTION_TIME_HH
#define DUNE_STUFF_FUNCTION_TIME_HH

#include <memory>
#include <string>

namespace Dune {
namespace Stuff {

/**
 * \brief Interface for scalar and vector valued timedependent functions.
 */
template< class DomainFieldImp, int domainDim, class RangeFieldImp, int rangeDim >
class TimedependentFunctionInterface
{
public:
  typedef DomainFieldImp                                  DomainFieldType;
  static const unsigned int                               dimDomain = domainDim;
  typedef Dune::FieldVector< DomainFieldType, dimDomain > DomainType;

  typedef RangeFieldImp                                 RangeFieldType;
  static const unsigned int                             dimRange = rangeDim;
  typedef Dune::FieldVector< RangeFieldType, dimRange > RangeType;

  virtual ~TimedependentFunctionInterface() {}

  static std::string static_id()
  {
    return "dune.stuff.timedependentfunction";
  }

  /** \defgroup info ´´These methods should be implemented in order to identify the function.'' */
  /* @{ */
  virtual std::string name() const
  {
    return static_id();
  }

  virtual int order() const
  {
    return -1;
  }
  /* @} */

  /** \defgroup must ´´This method has to be implemented.'' */
  /* @{ */
  virtual void evaluate(const DomainType& /*xx*/, const double& /*tt*/, RangeType& /*ret*/) const = 0;
  /* @} */
}; // class TimedependentFunctionInterface


//! use this to throw a stationary function into an algorithm that expects an instationary one
template< class EntityImp, class DomainFieldImp, int domainDim, class RangeFieldImp, int rangeDimRows >
struct TimeFunctionAdapter
  : public Dune::Stuff::TimedependentFunctionInterface< DomainFieldImp, domainDim, RangeFieldImp, rangeDimRows >
{
  typedef Dune::Stuff::GlobalFunction< EntityImp, DomainFieldImp, domainDim, RangeFieldImp, rangeDimRows > WrappedType;

  TimeFunctionAdapter(const WrappedType& wr)
    : wrapped_(wr)
  {}

  virtual void evaluate(const typename WrappedType::DomainType& x,
                        typename WrappedType::RangeType& ret) const
  {
    wrapped_(x, ret);
  }

  virtual void evaluate(const typename WrappedType::DomainType& x,
                        const double& /*t*/,
                        typename WrappedType::RangeType& ret) const
  {
    wrapped_(x, ret);
  }

  const WrappedType& wrapped_;
};

template< class EntityImp, class DomainFieldImp, int domainDim, class RangeFieldImp, int rangeDimRows >
TimeFunctionAdapter< EntityImp, DomainFieldImp, domainDim, RangeFieldImp, rangeDimRows >
timefunctionAdapted(const Dune::Stuff::GlobalFunction< EntityImp, DomainFieldImp, domainDim, RangeFieldImp, rangeDimRows >& wrapped)
{
  return TimeFunctionAdapter< EntityImp, DomainFieldImp, domainDim, RangeFieldImp, rangeDimRows >(wrapped);
}

} // namespace Stuff
} // namespace Dune


#endif // TIME_HH
