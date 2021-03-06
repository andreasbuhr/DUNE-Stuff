// This file is part of the dune-stuff project:
//   https://users.dune-project.org/projects/dune-stuff
// Copyright holders: Rene Milk, Felix Schindler
// License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)
//
// Contributors: Kirsten Weber

#ifndef DUNE_STUFF_GRID_PROVIDER_HH
#define DUNE_STUFF_GRID_PROVIDER_HH

#include <memory>

#include <dune/grid/sgrid.hh>

#include <dune/stuff/common/configtree.hh>

#include "provider/interface.hh"
#include "provider/cube.hh"
//#include "provider/gmsh.hh"
//#include "provider/starcd.hh"

namespace Dune {
namespace Stuff {

//#if HAVE_DUNE_GRID


template< class GridType = Dune::SGrid< 2, 2 > >
class GridProviders
{
public:
  typedef Stuff::Grid::ProviderInterface< GridType > InterfaceType;

  static std::vector< std::string > available()
  {
    namespace Providers = Stuff::Grid::Providers;
    return {
        Providers::Cube< GridType >::static_id()
//#if HAVE_ALUGRID || HAVE_ALBERTA || HAVE_UG
//#if defined ALUGRID_CONFORM || defined ALUGRID_CUBE || defined ALUGRID_SIMPLEX || defined ALBERTAGRID || defined UGGRID
//      , "gridprovider.gmsh"
//#endif
//#endif
//      , "gridprovider.starcd"
    };
  } // ... available()

  static Common::ConfigTree default_config(const std::string type, const std::string subname = "")
  {
    namespace Providers = Stuff::Grid::Providers;
    if (type == Providers::Cube< GridType >::static_id()) {
      return Providers::Cube< GridType >::default_config(subname);
//    }
//#if HAVE_ALUGRID || HAVE_ALBERTA || HAVE_UG
//#if defined ALUGRID_CONFORM || defined ALUGRID_CUBE || defined ALUGRID_SIMPLEX || defined ALBERTAGRID || defined UGGRID
//      else if (type == "gridprovider.gmsh") {
//      return GridProviderGmsh< GridType >::default_config(subname);
//    }
//#endif
//#endif
//    else if (type == "gridprovider.starcd") {
//      return GridProviderStarCD< GridType >::default_config(subname);
    } else
      DUNE_THROW_COLORFULLY(Exceptions::wrong_input_given,
                            "'" << type << "' is not a valid " << InterfaceType::static_id() << "!");
  } // ... default_config(...)

      static std::unique_ptr< InterfaceType >
  create(const std::string& type = available()[0], const Common::ConfigTree config = default_config(available()[0]))
  {
    namespace Providers = Stuff::Grid::Providers;
    if (type == Providers::Cube< GridType >::static_id())
      return Providers::Cube< GridType >::create(config);
//#if HAVE_ALUGRID || HAVE_ALBERTA || HAVE_UG
//#if defined ALUGRID_CONFORM || defined ALUGRID_CUBE || defined ALUGRID_SIMPLEX || defined ALBERTAGRID || defined UGGRID
//    else if (type == "gridprovider.gmsh")
//      return GridProviderGmsh< GridType >::create(config);
//#endif
//#endif
//    else if (type == "gridprovider.starcd")
//      return GridProviderStarCD< GridType >::create(config);
    else
      DUNE_THROW_COLORFULLY(Exceptions::wrong_input_given,
                            "'" << type << "' is not a valid " << InterfaceType::static_id() << "!");
  } // ... create(...)
}; // class GridProviders


//#endif // HAVE_DUNE_GRID

} // namespace Stuff
} // namespace Dune

#endif // DUNE_STUFF_GRID_PROVIDER_HH
