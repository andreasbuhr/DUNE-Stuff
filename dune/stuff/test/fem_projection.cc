// This file is part of the dune-stuff project:
//   https://users.dune-project.org/projects/dune-stuff
// Copyright holders: Rene Milk, Felix Schindler
// License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)

#include "test_common.hh"

#if HAVE_DUNE_FEM

#include <dune/stuff/fem/namespace.hh>

#if DUNE_FEM_IS_LOCALFUNCTIONS_COMPATIBLE
#include <memory>

#include <dune/fem/misc/mpimanager.hh>
#include <dune/fem/space/fvspace.hh>
#include <dune/fem/space/dgspace.hh>
#include <dune/fem/gridpart/adaptiveleafgridpart.hh>
#include <dune/fem/function/adaptivefunction.hh>
#include <dune/fem/operator/projection/l2projection.hh>
#include <dune/fem/io/file/datawriter.hh>

#include <dune/stuff/aliases.hh>
#include <dune/stuff/functions.hh>
#include <dune/stuff/common/tuple.hh>
#include <dune/stuff/fem/functions/timefunction.hh>
#include <dune/stuff/fem/customprojection.hh>
#include <dune/stuff/grid/provider/cube.hh>
#include <dune/stuff/functions/expression.hh>
#include <dune/stuff/functions/affineparametric/coefficient.hh>
#include <dune/stuff/fem/namespace.hh>

template <int dimDomain, int rangeDim>
struct CustomFunction : public Dune::Stuff::FunctionInterface< double, dimDomain, double, rangeDim > {
  typedef Dune::Stuff::FunctionInterface< double, dimDomain, double, rangeDim > Base;

  void evaluate(const typename Base::DomainType& /*arg*/, typename Base::RangeType& ret) const
  {
    ret = typename Base::RangeType(0.0f);
  }

  template <class IntersectionType>
  void evaluate( const typename Base::DomainType& /*arg*/, typename Base::RangeType& ret, const IntersectionType& face ) const
  {
    ret = typename Base::RangeType(face.geometry().volume());
  }
};

template <int dimDomain, int rangeDim>
struct CustomFunctionT : public Dune::Stuff::FunctionInterface< double, dimDomain, double, rangeDim > {
  typedef Dune::Stuff::FunctionInterface< double, dimDomain, double, rangeDim > Base;
  using Base::evaluate;

  void evaluate( const double time, const typename Base::DomainType& /*arg*/,typename  Base::RangeType& ret ) const
  {
    ret = typename Base::RangeType(time);
  }

  void evaluate( const typename Base::DomainType& /*arg*/, typename Base::RangeType& ret ) const
  {
    ret = typename Base::RangeType(0.0f);
  }
};

template <class GridDim, class RangeDim>
struct ProjectionFixture {
public:
  static const int range_dim = RangeDim::value;
  static const int pol_order = 1;

  typedef Dune::Stuff::GridProviderCube< Dune::SGrid< GridDim::value, GridDim::value > > GridProviderType;
  typedef typename GridProviderType::GridType GridType;
  typedef Dune::Stuff::FunctionExpression< double, GridType::dimension, double, range_dim > FunctionType;
  typedef typename FunctionType::FunctionSpaceType FunctionSpaceType;
  typedef Dune::Fem::AdaptiveLeafGridPart< GridType > GridPartType;
  typedef Dune::Fem::DiscontinuousGalerkinSpace< FunctionSpaceType,
                                            GridPartType,
                                            pol_order>
    DiscreteFunctionSpaceType;
  typedef Dune::Fem::AdaptiveDiscreteFunction< DiscreteFunctionSpaceType > DiscreteFunctionType;
  GridProviderType gridProvider_;
  GridPartType gridPart_;
  DiscreteFunctionSpaceType disc_space_;
  DiscreteFunctionType disc_function;

  ProjectionFixture()
    : gridProvider_(GridProviderType(0.0f, 1.0f, 32u))
    , gridPart_(*(gridProvider_.grid()))
    , disc_space_(gridPart_)
    , disc_function("disc_function", disc_space_)
  {}
};

struct RunBetterL2Projection {
  template <class GridDim, class RangeDim>
  static void run()
  {
    typedef ProjectionFixture<GridDim, RangeDim> TestType;
    TestType test;
    CustomFunctionT<GridDim::value, RangeDim::value> f;
    DSFe::BetterL2Projection::project(f, test.disc_function);
    const double time = 0.0f;
    DSFe::BetterL2Projection::project(time, f, test.disc_function);
    DSFe::ConstTimeProvider tp(0.0f);
    DSFe::BetterL2Projection::project(tp, f, test.disc_function);
  }
};

struct RunCustomProjection {
  template <class GridDim, class RangeDim>
  static void run()
  {
    typedef ProjectionFixture<GridDim, RangeDim> TestType;
    TestType test;
    CustomFunction<GridDim::value, RangeDim::value> f;
    DSFe::CustomProjection::project(f, test.disc_function);
  }
};

template <class TestFunctor>
struct ProjectionTest : public ::testing::Test {
  typedef boost::mpl::vector< Int<1>, Int<2>, Int<3>> GridDims;
  typedef GridDims RangeDims;
  typedef typename DSC::TupleProduct::Combine< GridDims, RangeDims, TestFunctor
                  >::template Generate<> base_generator_type;
  void run() {
    base_generator_type::Run();
  }
};

typedef ::testing::Types<RunBetterL2Projection, RunCustomProjection> TestParameter;
TYPED_TEST_CASE(ProjectionTest, TestParameter);
TYPED_TEST(ProjectionTest, All) {
  this->run();
}

#endif
#endif //#if DUNE_FEM_IS_LOCALFUNCTIONS_COMPATIBLE

int main(int argc, char** argv)
{
  test_init(argc, argv);
  return RUN_ALL_TESTS();
}
