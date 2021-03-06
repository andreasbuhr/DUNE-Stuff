// This file is part of the dune-stuff project:
//   https://users.dune-project.org/projects/dune-stuff
// Copyright holders: Rene Milk, Felix Schindler
// License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)

/**
   *  \file   stuff.hh
   *  \brief  contains some stuff
   **/
#ifndef DUNE_STUFF_COMMON_STRING_HH
#define DUNE_STUFF_COMMON_STRING_HH

#include <cstring>
#include <ctime>
#include <map>
#include <assert.h>
#include <algorithm>

#include <dune/common/array.hh>
#include <dune/common/deprecated.hh>
#include <dune/common/static_assert.hh>
#include <ostream>
#include <iomanip>
#include <vector>
#include <string>
#include <ctime>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace Dune {
namespace Stuff {
namespace Common {

//! simple and dumb std::string to anything conversion
template< class ReturnType >
inline ReturnType fromString(const std::string s) {
  return boost::lexical_cast<ReturnType, std::string>(s);
} // fromString

#define DSC_FRSTR(tn,tns) \
template<> \
inline tn fromString<tn>(const std::string s) { \
  return std::sto##tns(s); \
}

DSC_FRSTR(int, i)
DSC_FRSTR(long, l)
DSC_FRSTR(long long, ll)
DSC_FRSTR(unsigned long, ul)
DSC_FRSTR(unsigned long long, ull)
DSC_FRSTR(float,f)
DSC_FRSTR(double,d)
DSC_FRSTR(long double,ld)

#undef DSC_FRSTR


inline std::string toString(const char* s) {
  return std::string(s);
} // toString

inline std::string toString(const std::string s) {
  return std::string(s);
} // toString

//! simple and dumb anything to std::string conversion
template< class InType >
inline std::string toString(const InType& s) {
  return std::to_string(s);
} // toString


/**
  \brief      Returns a string of lengths s' whitespace (or c chars).
  \param[in]  s
              std::string, defines the length of the return string
  \param[in]  whitespace
              char, optional argument, defines entries of return string
  \return     std::string
              Returns a string of lengths s' whitespace (or c chars).
  **/
template< class T >
std::string whitespaceify(const T& t, const char whitespace = ' ')
{
  const std::string s = toString(t);
  std::string ret = "";
  for (unsigned int i = 0; i < s.size(); ++i) {
    ret += whitespace;
  }
  return ret;
} // std::string whitespaceify(const std::string s, const char whitespace = ' ')

/** \brief convenience wrapper around boost::algorithm::split to split one string into a vector of strings
 * \param msg the spring to be split
 * \param seperators a list of seperaors, duh
 * \param mode token_compress_off --> potentially empty strings in return,
              token_compress_on --> empty tokens are discarded
 * \return all tokens in a vector, if msg contains no seperators, this'll contain msg as its only element
 **/
template < class T = std::string >
inline std::vector<T> tokenize( const std::string& msg,
                             const std::string& seperators,
                             const boost::algorithm::token_compress_mode_type mode = boost::algorithm::token_compress_off )
{
    std::vector<std::string> strings;
    boost::algorithm::split( strings, msg, boost::algorithm::is_any_of(seperators),
                             mode );
    std::vector<T> ret(strings.size());
    size_t i = 0;
    //special case for empty strings to avoid non-default init
    std::generate(std::begin(ret), std::end(ret), [&] (){ return strings[i++].empty() ? T() : fromString<T>(strings[i-1]); });
    return ret;
}

template < >
inline std::vector<std::string> tokenize( const std::string& msg,
                             const std::string& seperators,
                             const boost::algorithm::token_compress_mode_type mode )
{
    std::vector<std::string> strings;
    boost::algorithm::split( strings, msg, boost::algorithm::is_any_of(seperators),
                             mode );
    return strings;
}


//! returns string with local time in current locale's format
inline std::string stringFromTime(time_t cur_time = time(NULL)) {
  return ctime(&cur_time);
}

//! helper struct for lexical cast
template <typename ElemT>
struct HexToString {
  // see http://stackoverflow.com/a/2079728
  ElemT value;
  operator ElemT() const {return value;}
  friend std::istream& operator>>(std::istream& in, HexToString& out) {
    in >> std::hex >> out.value;
    return in;
  }
};

// some legacy stuff I'd like to keep for a while
namespace String {

template< class T >
DUNE_DEPRECATED_MSG("use DSC::fromString instead, removal with stuff 2.3") T from(const std::string& s)
{
  std::stringstream ss;
  ss << s;
  T t;
  ss >> t;
  return t;
}

template< class T >
DUNE_DEPRECATED_MSG("use DSC::toString instead, removal with stuff 2.3") std::string to(const T& t)
{
  std::stringstream ss;
  ss << t;
  std::string s;
  ss >> s;
  return s;
}

DUNE_DEPRECATED_MSG("use the constructor call directly")
inline std::vector< std::string > mainArgsToVector(int argc, char** argv)
{
  return std::vector< std::string >(argv, argv + argc);
}

inline char** vectorToMainArgs(const std::vector< std::string > args)
{
  char** argv = new char* [args.size()];
  for (unsigned int ii = 0; ii < args.size(); ++ii) {
    argv[ii] = new char[args[ii].length() + 1];
    strcpy(argv[ii], args[ii].c_str());
  }
  return argv;
} // char** vectorToMainArgs(const std::vector< std::string > args)

} // namespace String

} // namespace Common
} // namespace Stuff
} // namespace Dune

#endif // DUNE_STUFF_COMMON_STRING_HH
