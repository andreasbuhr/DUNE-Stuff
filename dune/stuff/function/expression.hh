#ifndef DUNE_STUFF_FUNCTION_EXPRESSION_HH
#define DUNE_STUFF_FUNCTION_EXPRESSION_HH

#include <sstream>
#include <vector>

#include <dune/common/fvector.hh>
#include <dune/common/exceptions.hh>

#include <dune/stuff/common/parameter/tree.hh>
#include <dune/stuff/common/string.hh>
#include <dune/stuff/common/color.hh>

#include "expression/base.hh"
#include "interface.hh"

namespace Dune {
namespace Stuff {


template< class DomainFieldImp, int domainDim, class RangeFieldImp, int rangeDim >
class FunctionExpression
  : public FunctionExpressionBase< DomainFieldImp, domainDim, RangeFieldImp, rangeDim >
  , public FunctionInterface< DomainFieldImp, domainDim, RangeFieldImp, rangeDim, 1 >
{
  typedef FunctionExpressionBase<  DomainFieldImp, domainDim, RangeFieldImp, rangeDim > BaseType;
  typedef FunctionInterface<  DomainFieldImp, domainDim, RangeFieldImp, rangeDim, 1 >   InterfaceType;
public:
  typedef FunctionExpression<  DomainFieldImp, domainDim, RangeFieldImp, rangeDim >     ThisType;

  typedef typename InterfaceType::DomainFieldType DomainFieldType;
  static const int                                dimDomain = InterfaceType::dimDomain;
  typedef typename InterfaceType::DomainType      DomainType;
  typedef typename InterfaceType::RangeFieldType  RangeFieldType;
  static const int                                dimRange = InterfaceType::dimRange;
  typedef typename InterfaceType::RangeType       RangeType;

  static const std::string id()
  {
    return InterfaceType::id() + ".expression";
  }

  FunctionExpression(const std::string _variable,
                     const std::string _expression,
                     const int _order = -1,
                     const std::string _name = id())
    : BaseType(_variable, _expression)
    , order_(_order)
    , name_(_name)
  {}

  FunctionExpression(const std::string _variable,
                     const std::vector< std::string > _expressions,
                     const int _order = -1,
                     const std::string _name = id())
    : BaseType(_variable, _expressions)
    , order_(_order)
    , name_(_name)
  {}

  static Dune::ParameterTree createSampleDescription(const std::string subName = "")
  {
    Dune::ParameterTree description;
    description["variable"] = "x";
    description["expression"] = "[x[0]; sin(x[0])]";
    description["order"] = "1";
    description["name"] = "function.expression";
    if (subName.empty())
      return description;
    else {
      Dune::Stuff::Common::ExtendedParameterTree extendedDescription;
      extendedDescription.add(description, subName);
      return extendedDescription;
    }
  } // ... createSampleDescription(...)

  static ThisType* create(const DSC::ExtendedParameterTree description)
  {
    // get necessary
    const std::string _variable = description.get< std::string >("variable", "x");
    std::vector< std::string > _expressions;
    // lets see, if there is a key or vector
    if (description.hasVector("expression")) {
      const std::vector< std::string > expr = description.getVector< std::string >("expression", 1);
      for (size_t ii = 0; ii < expr.size(); ++ii)
        _expressions.push_back(expr[ii]);
    } else if (description.hasKey("expression")) {
      const std::string expr = description.get< std::string >("expression");
      _expressions.push_back(expr);
    } else
      DUNE_THROW(Dune::IOError,
                 "\n" << Dune::Stuff::Common::colorStringRed("ERROR:")
                 << " neither key nor vector 'expression' found in the following description:\n"
                 << description.reportString("  "));
    // get optional
    const int _order = description.get< int >("order", -1);
    const std::string _name = description.get< std::string >("name", "function.expression");
    // create and return
    return new ThisType(_variable, _expressions, _order, _name);
  } // ... create(...)

  virtual int order() const
  {
    return order_;
  }

  virtual std::string name() const
  {
    return name_;
  }

  virtual void evaluate(const DomainType& _x, RangeType& _ret) const
  {
    BaseType::evaluate(_x, _ret);
  }

private:
  int order_;
  std::string name_;
}; // class FunctionExpression


} // namespace Stuff
} // namespace Dune

#endif // DUNE_STUFF_FUNCTION_EXPRESSION_HH
