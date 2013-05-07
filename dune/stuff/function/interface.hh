﻿#ifndef DUNE_STUFF_FUNCTION_INTERFACE_HH
#define DUNE_STUFF_FUNCTION_INTERFACE_HH

#ifdef HAVE_CMAKE_CONFIG
  #include "cmake_config.h"
#elif defined (HAVE_CONFIG_H)
  #include <config.h>
#endif // ifdef HAVE_CMAKE_CONFIG

#include <vector>

#include <dune/common/shared_ptr.hh>
#include <dune/common/fvector.hh>
#include <dune/common/dynvector.hh>
#include <dune/common/function.hh>

#include <dune/stuff/common/color.hh>
#include <dune/stuff/common/parameter.hh>
#include <dune/stuff/localfunction/interface.hh>

namespace Dune {
namespace Stuff {


// forward, needed in the interface, included below
template< class RangeFieldImp >
class FunctionAffineSeparablCoefficient;


template< class DomainFieldImp, int domainDim, class RangeFieldImp, int rangeDim >
class FunctionInterface
{
public:
  typedef FunctionInterface<  DomainFieldImp, domainDim, RangeFieldImp, rangeDim > ThisType;

  typedef DomainFieldImp                                  DomainFieldType;
  static const int                                        dimDomain = domainDim;
  typedef Dune::FieldVector< DomainFieldType, dimDomain > DomainType;

  typedef RangeFieldImp                                 RangeFieldType;
  static const int                                      dimRange = rangeDim;
  typedef Dune::FieldVector< RangeFieldType, dimRange > RangeType;

  typedef Common::Parameter::FieldType  ParamFieldType;
  static const int                      maxParamDim = Common::Parameter::maxDim;
  typedef Common::Parameter::Type       ParamType;

  typedef ThisType                                            ComponentType;
  typedef FunctionAffineSeparablCoefficient< RangeFieldType > CoefficientType;

  virtual ~FunctionInterface() {}

  static std::string id()
  {
    return "function";
  }

  /** \defgroup type ´´This method has to be implemented for parametric functions and determines
   *                   which evaluate() is callable.''
   */
  /* @{ */
  virtual bool parametric() const
  {
    return false;
  }
  /* @} */

  /** \defgroup info ´´These methods should be implemented in order to identify the function.'' */
  /* @{ */
  virtual std::string name() const
  {
    return id();
  }

  virtual int order() const
  {
    return -1;
  }
  /* @} */

  /** \defgroup nonparametric-must ´´These methods have to be implemented, if parametric() == false.'' */
  /* @{ */
  virtual void evaluate(const DomainType& /*_x*/, RangeType& /*_ret*/) const
  {
    DUNE_THROW(Dune::NotImplemented,
               "\n" << Dune::Stuff::Common::colorStringRed("ERROR:") << " implement me if parametric() == false!");
  }
  /* @} */

  /** \defgroup parametric-must ´´These methods have to be implemented, if parametric() == true.'' */
  /* @{ */
  virtual void evaluate(const DomainType& /*_x*/, const ParamType& /*_mu*/, RangeType& /*_ret*/) const
  {
    DUNE_THROW(Dune::NotImplemented,
               "\n" << Dune::Stuff::Common::colorStringRed("ERROR:") << " implement me if parametric() == true!");
  }

  virtual size_t paramSize() const
  {
    if (!parametric())
      return 0;
    else
      DUNE_THROW(Dune::NotImplemented,
                 "\n" << Dune::Stuff::Common::colorStringRed("ERROR:") << " implement me if parametric() == true!");
  }

  virtual const std::vector< ParamType >& paramRange() const
  {
    DUNE_THROW(Dune::NotImplemented,
               "\n" << Dune::Stuff::Common::colorStringRed("ERROR:") << " implement me if parametric() == true!");
  }

  virtual const std::vector< std::string >& paramExplanation() const
  {
    DUNE_THROW(Dune::NotImplemented,
               "\n" << Dune::Stuff::Common::colorStringRed("ERROR:") << " implement me if parametric() == true!");
  }

  virtual bool affineparametric() const
  {
    return false;
  }
  /* @} */

  /** \defgroup affineparametric ´´These methods have to be implemented, if affineparametric() == true.'' */
  /* @{ */
  virtual size_t numComponents() const
  {
    DUNE_THROW(Dune::NotImplemented,
               "\n" << Dune::Stuff::Common::colorStringRed("ERROR:") << " implement me if affineparametric() == true!");
  }

  virtual const std::vector< Dune::shared_ptr< const ComponentType > >& components() const
  {
    DUNE_THROW(Dune::NotImplemented,
               "\n" << Dune::Stuff::Common::colorStringRed("ERROR:") << " implement me if affineparametric() == true!");
  }

  virtual size_t numCoefficients() const
  {
    DUNE_THROW(Dune::NotImplemented,
               "\n" << Dune::Stuff::Common::colorStringRed("ERROR:") << " implement me if affineparametric() == true!");
  }

  virtual const std::vector< Dune::shared_ptr< const CoefficientType > >& coefficients() const
  {
    DUNE_THROW(Dune::NotImplemented,
               "\n" << Dune::Stuff::Common::colorStringRed("ERROR:") << " implement me if affineparametric() == true!");
  }
  /* @} */

  /** \defgroup provided ´´These methods are provided by the interface itself, but may not be implemented optimal.'' */
  /* @{ */
  virtual RangeType evaluate(const DomainType& _x) const
  {
    assert(!parametric());
    RangeType ret;
    evaluate(_x, ret);
    return ret;
  }

  // forward, needed for the traits
  template< class EntityImp >
  class LocalFunctionAdapter;

  template< class EntityImp >
  class LocalFunctionAdapterTraits
  {
  public:
    typedef LocalFunctionAdapter< EntityImp > derived_type;
    typedef EntityImp                         EntityType;
  };

  template< class EntityImp >
  class LocalFunctionAdapter
    : public LocalFunctionInterface< DomainFieldType, dimDomain, RangeFieldType, dimRange, LocalFunctionAdapterTraits< EntityImp > >
  {
  public:
    typedef LocalFunctionAdapterTraits< EntityImp > Traits;

    typedef typename Traits::EntityType EntityType;

    LocalFunctionAdapter(const ThisType& function, const EntityType& en)
      : wrapped_(function)
      , entity_(en)
    {
      assert(!wrapped_.parametric());
    }

    virtual int order() const
    {
      return wrapped_.order();
    }

    const EntityType& entity() const
    {
      return entity_;
    }

    void evaluate(const DomainType& x, RangeType& ret) const
    {
      wrapped_.evaluate(entity_.geometry().global(x), ret);
    }

  private:
    const ThisType& wrapped_;
    const EntityType& entity_;
  };

  template< class EntityType >
  LocalFunctionAdapter< EntityType > localFunction(const EntityType& entity) const
  {
    return LocalFunctionAdapter< EntityType >(*this, entity);
  }
}; // class FunctionInterface


} // namespace Stuff
} // namespace Dune

#include "affineparametric/coefficient.hh"

#endif // DUNE_STUFF_FUNCTION_INTERFACE_HH
