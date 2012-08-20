/**
   *  \file   parametercontainer.hh
   *
   *  \brief  containing class ParameterContainer
   **/

#ifndef PARAMETERCONTAINER_HH_INCLUDED
#define PARAMETERCONTAINER_HH_INCLUDED

#include <dune/common/deprecated.hh>
#include <dune/fem/io/parameter.hh>

#include <dune/stuff/common/logging.hh>
#include <dune/stuff/common/filesystem.hh>
#include <dune/stuff/common/misc.hh>
#include <dune/stuff/common/parameter/validation.hh>
#include <dune/stuff/common/string.hh>

#include <vector>
#include <algorithm>
#include <fstream>

#include <boost/format.hpp>

namespace Dune {
namespace Stuff {
namespace Common {
namespace Parameter {

/**
   *  \brief  class containing global parameters
   *  \deprecated
   *  ParameterContainer contains all the needed global parameters getting them via Dune::Parameter
   *
   **/
class Container
{
public:
  /**
     *  \brief  destuctor
     *
     *  doing nothing
     **/
  ~Container()
  {}

  /**
     *  \brief  prints all parameters
     *
     *  \todo   implement me
     *
     *  \param  out stream to print to
     **/
  void Print(std::ostream& out) const {
    out << "\nthis is the ParameterContainer.Print() function" << std::endl;
  }

  /**
     *  \brief  checks command line parameters
     *
     *  \return true, if comman line arguments are valid
     **/
  bool ReadCommandLine(int argc, char** argv) {
    if (argc == 2)
    {
      parameter_filename_ = argv[1];
      Dune::Parameter::append(parameter_filename_);
    } else {
      Dune::Parameter::append(argc, argv);
    }
    const std::string datadir = Dune::Parameter::getValidValue(std::string("fem.io.datadir"),
                                                               std::string("data"),
                                                               ValidateAny< std::string >()
                                                               );
    Dune::Parameter::append("fem.prefix", datadir);
    if ( !Dune::Parameter::exists("fem.io.logdir") )
      Dune::Parameter::append("fem.io.logdir", "log");
    warning_output_ = Dune::Parameter::getValue("disableParameterWarnings", warning_output_);
    return CheckSetup();
  } // ReadCommandLine

  /** \brief checks for mandatory params
     *
     *  \return true, if all needed parameters exist
     **/
  bool CheckSetup() {
    typedef std::vector< std::string >::iterator
        Iterator;
    Iterator it = mandatory_params_.begin();
    Iterator new_end = std::remove_if(it, mandatory_params_.end(), Dune::Parameter::exists);
    all_set_up_ = (new_end == it);
    for ( ; new_end != it; ++it)
    {
      std::cerr << "\nError: " << parameter_filename_
                << " is missing parameter: "
                << *it << std::endl;
    }
    return all_set_up_;
  } // CheckSetup

  /**
     *  \brief  prints, how a parameterfile schould look like
     *
     *  \param out stream to print
     **/
  void PrintParameterSpecs(std::ostream& out) {
    out << "\na valid parameterfile should at least specify the following parameters:\n"
        << "Remark: the correspondig files have to exist!\n"
        << "(copy this into your parameterfile)\n";
    std::vector< std::string >::const_iterator it = mandatory_params_.begin();
    for ( ; it != mandatory_params_.end(); ++it)
      std::cerr << *it << ": VALUE\n";
    std::cerr << std::endl;
  } // PrintParameterSpecs

  std::string DgfFilename(unsigned int dim) const {
    assert(dim > 0 && dim < 4);
    assert(all_set_up_);
    std::string retval = Dune::Parameter::getValue< std::string >( (boost::format("dgf_file_%dd") % dim).str() );
    Dune::Parameter::append( (boost::format("fem.io.macroGridFile_%dd") % dim).str(), retval );
    return retval;
  } // DgfFilename

  /** \brief  passthrough to underlying Dune::Parameter
     *  \param  useDbgStream
     *          needs to be set to false when using this function in Logging::Create,
     *              otherwise an assertion will will cause streams aren't available yet
     **/
  template< typename T >
  T getParam(std::string name, T def, bool useDbgStream = true) {
    return getParam(name, def, ValidateAny< T >(), useDbgStream);
  }

  template< typename T, class Validator >
  T getParam(std::string name, T def, const Validator& validator, bool UNUSED_UNLESS_DEBUG(useDbgStream) = true) {
    assert(all_set_up_);
    assert( validator(def) );
    #ifndef NDEBUG
    if ( warning_output_ && !Dune::Parameter::exists(name) )
    {
      if (useDbgStream)
        Dune::Stuff::Common::Logger().debug() << "WARNING: using default value for parameter \"" << name << "\"" << std::endl;
      else
        std::cerr << "WARNING: using default value for parameter \"" << name << "\"" << std::endl;
    }
    #endif // ifndef NDEBUG
    try {
      return Dune::Parameter::getValidValue(name, def, validator);
    } catch (Dune::ParameterInvalid& p) {
      std::cerr << boost::format("Dune::Fem::Parameter reports inconsistent parameter: %s\n") % p.what();
    }
    return def;
  } // getParam

  std::map< char, std::string > getFunction(const std::string& name, const std::string def = "0") {
    std::map< char, std::string > ret;
    ret['x'] = getParam(name + "_x", def);
    ret['y'] = getParam(name + "_y", def);
    ret['z'] = getParam(name + "_z", def);
    return ret;
  } // getFunction

  //! passthrough to underlying Dune::Parameter
  template< typename T >
  void setParam(std::string name, T val) {
    assert(all_set_up_);
    return Dune::Parameter::append( name, Dune::Stuff::Common::String::convertTo(val) );
  }

  //! extension to Fem::paramter that allows vector/list like paramteres from a single key
  template< class T >
  std::vector< T > getList(const std::string name, T def) {
    if ( !Dune::Parameter::exists(name) )
    {
      std::vector< T > ret;
      ret.push_back(def);
      return ret;
    }
    std::string tokenstring = getParam( name, std::string("dummy") );
    std::string delimiter = getParam(std::string("parameterlist_delimiter"), std::string(";"), false);
    return Dune::Stuff::Common::String::tokenize< T >(tokenstring, delimiter);
  } // getList

private:
  bool all_set_up_;
  bool warning_output_;
  std::string parameter_filename_;
  std::vector< std::string > mandatory_params_;

  /**
     *  \brief  constuctor
     *
     *  \attention  call ReadCommandLine() to set up parameterContainer
     **/
  Container()
    : all_set_up_(false)
      , warning_output_(true) {
    const std::string p[] = { "dgf_file_2d", "dgf_file_3d" };

    mandatory_params_ = std::vector< std::string >( p, p + ( sizeof(p) / sizeof(p[0]) ) );
  }

  friend Container& Parameters();
};

//! global ParameterContainer instance
Container& DUNE_DEPRECATED_MSG("use the Dune::ParameterTree based ConfigContainer instead") Parameters() {
  static Container parameters;
  return parameters;
}

//! get a path in datadir with existence guarantee (cannot be in filessytem.hh -- cyclic dep )
std::string getFileinDatadir(const std::string& fn) {
  boost::filesystem::path path( Parameters().getParam( "fem.io.datadir", std::string(".") ) );

  path /= fn;
  boost::filesystem::create_directories( path.parent_path() );
  return path.string();
} // getFileinDatadir

} // namespace Parameter
} // namespace Common
} // namespace Stuff
} // namespace Dune

#endif // end of PARAMETERHANDLER.HH

/** Copyright (c) 2012, Rene Milk
   * All rights reserved.
   *
   * Redistribution and use in source and binary forms, with or without
   * modification, are permitted provided that the following conditions are met:
   *
   * 1. Redistributions of source code must retain the above copyright notice, this
   *    list of conditions and the following disclaimer.
   * 2. Redistributions in binary form must reproduce the above copyright notice,
   *    this list of conditions and the following disclaimer in the documentation
   *    and/or other materials provided with the distribution.
   *
   * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
   * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
   *
   * The views and conclusions contained in the software and documentation are those
   * of the authors and should not be interpreted as representing official policies,
   * either expressed or implied, of the FreeBSD Project.
   **/
