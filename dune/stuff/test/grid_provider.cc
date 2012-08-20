#include "test_common.hh"

// system
#include <iostream>
#include <fstream>
#include <utility>

// boost
#include <boost/filesystem.hpp>

// dune-common
#include <dune/common/mpihelper.hh>
#include <dune/common/parametertreeparser.hh>
#include <dune/common/timer.hh>

#include <dune/stuff/common/parameter/tree.hh>
#include <dune/stuff/grid/provider/cornerpoint.hh>
#include <dune/stuff/grid/provider/cube.hh>

using namespace Dune::Stuff;

const std::string id = "grid_provider";

/**
  \brief      Creates a parameter file if it does not exist.

              Nothing is done if the file already exists. If not, a parameter file will be created with all neccessary
              keys and values.
  \param[in]  filename
              (Relative) path to the file.
  **/
void ensureParamFile(std::string filename)
{
  // only write param file if there is none
  if (!boost::filesystem::exists(filename)) {
    std::ofstream file;
    file.open(filename);
    file << "[stuff.grid.provider.cube]" << std::endl;
    file << "level = 2" << std::endl;
    file << "visualize.grid = rb_grid_provider_cube_grid" << std::endl;
    file << "visualize.msGrid = rb_grid_provider_cube_msGrid" << std::endl;
    file << std::endl;
    file << "[stuff.grid.provider.cornerpoint]" << std::endl;
    file << "filename = /dune-stuff/data/grid/johansen_formation.grdecl" << std::endl; // has to be an absolute path
    file.close();
  } // only write param file if there is none
} // void ensureParamFile()

/**
  \brief      Fills a Dune::ParameterTree given a parameter file or command line arguments.
  \param[in]  argc
              From \c main()
  \param[in]  argv
              From \c main()
  \param[out] paramTree
              The Dune::ParameterTree that is to be filled.
  **/
void initParamTree(int argc, char** argv, Dune::ParameterTree& paramTree)
{
  paramTree = Common::Parameter::Tree::init(argc, argv, "provider.param");
}

template< class GridViewType >
int walkGridView(const GridViewType& gridView)
{
  return gridView.size(0);
} // void walkGrid(const GridType& grid)

/*template< class MDGridType >
int walkMDGrid(MDGridType& mdGrid)
{
  int numElements = 0;
  // loop over all subdomains
  for (typename MDGridType::SubDomainIndexType subdomainIndex = 0;
       subdomainIndex <= mdGrid.maxSubDomainIndex;
       ++subdomainIndex) {
    // subdomain grid
    typedef typename MDGridType::SubDomainGrid SDGridType;
    const SDGridType& sdGrid = mdGrid.subDomain(subdomainIndex);
    // subdomain grid view
    typedef typename SDGridType::LeafGridView SDGridViewType;
    const SDGridViewType& sdGridView = sdGrid.leafView();
    // walk the subdomain grid
    typedef typename SDGridViewType::template Codim<0>::Iterator SDElementIteratorType;
    typedef typename SDGridViewType::template Codim<0>::Entity SDElementType;
    for (SDElementIteratorType sdElementIterator = sdGridView.template begin<0>();
         sdElementIterator != sdGridView.template end<0>();
         ++sdElementIterator) {
      const SDElementType& sdElement = *sdElementIterator;
      ++numElements;
    } // walk the subdomain grid
  } // loop over all subdomains
  return numElements;
} // void walkMDGrid(const GridType& grid)*/

template< class GridProviderType >
void measureTiming(GridProviderType& gridProvider)
{
  Dune::Timer timer;
  typename GridProviderType::GridType::LeafGridView gridView = gridProvider.grid().leafView();
  const int numHostGridElements = walkGridView(gridView);
  std::cout << "  host grid:        " << timer.elapsed() << " sec, " << numHostGridElements << " elements" << std::endl;
  timer.reset();
}

/**
  \brief  Main routine.
  **/
int GNAH(int argc, char** argv)
{
  try {
    // mpi
    Dune::MPIHelper::instance(argc, argv);
    // parameter
    ensureParamFile(id + ".param");
    Common::Parameter::Tree::Extended paramTree = Common::Parameter::Tree::init(argc, argv, id + ".param");
    std::ofstream new_param(id + ".param_new");
    paramTree.report(new_param);
    // timer
    Dune::Timer timer;
    // unitcube

    typedef Grid::Provider::Cube< Dune::GridSelector::GridType > CubeProviderType;
    CubeProviderType cubeProvider(paramTree.sub(cubeProvider.id));
    cubeProvider.visualize(id);
    // cornerpoint
#ifdef HAVE_DUNE_CORNERPOINT
    typedef Grid::Provider::Cornerpoint CornerpointGridProviderType;
    CornerpointGridProviderType cornerpointGridProvider(paramTree);
    cornerpointGridProvider.visualize(paramTree);
#endif
    // measure timing
    std::cout << std::endl;
    measureTiming(cubeProvider);

  } catch (Dune::Exception& e) {
    std::cout << e.what() << std::endl;
    return 1;
  }
  return 0;
}

static const int dim = 2;
typedef testing::Types< Dune::YaspGrid< dim >
#if HAVE_ALUGRID
  , Dune::ALUCubeGrid< dim, dim >
  , Dune::ALUConformGrid< dim, dim >
  , Dune::ALUSimplexGrid< dim, dim >
#endif
#if HAVE_ALBERTA
  , Dune::AlbertaGrid< dim >
#endif
#if HAVE_UG
  , Dune::UGGrid< dim >
#endif
  , Dune::SGrid< dim, dim > > GridTypes;

template <class GridType>
struct CubeTest : public testing::Test{
  typedef Dune::FieldVector< typename GridType::ctype, dim > CoordinateType;
  CubeTest()
  {}

  void test_cube(typename GridType::ctype lower,
                 typename GridType::ctype upper,
                 const std::vector<u_int16_t>& elements_per_dimension) {
    test_cube(CoordinateType(lower), CoordinateType(upper), elements_per_dimension);
  }

  void test_cube(const CoordinateType lower,
                 const CoordinateType upper,
                 const std::vector<u_int16_t>& elements_per_dimension) {
    Grid::Provider::Cube< GridType > cube(lower, upper, elements_per_dimension);
    EXPECT_GE(cube.grid().size(0), 0);
    EXPECT_GE(cube.grid().size(1), 0);
  }
};

TYPED_TEST_CASE(CubeTest, GridTypes);
TYPED_TEST(CubeTest, All){
  std::vector<u_int16_t> f = {1u,2u};
  this->test_cube(0,1,f);
}

TEST(OLD, gnah){
  char* gnah[] = {"koko"};
  EXPECT_EQ(0, GNAH(1,gnah));
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  Dune::MPIHelper::instance(argc, argv);
  return RUN_ALL_TESTS();
}
