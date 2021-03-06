/**
 * @file exception.hpp
 *
 * @brief Utilities to handle exceptions and to pretty-print value
 *
 * @author Anthony Truchet <anthony.truchet@logilab.fr>
 *
 */

#ifndef TY_EXCEPTIONS
#define TY_EXCEPTIONS

#include <string>
#include <sstream>
#include <cassert>
#include <exception>
#include <deque>

#include <boost/exception/all.hpp>
#include <boost/current_function.hpp>
#include <boost/throw_exception.hpp>
#include <boost/config.hpp>


// Beware : Stuff::operator<< must be defined in the same namespace as Stuff

// For GTest related pretty-printing please see :
// http://code.google.com/p/googletest/wiki/AdvancedGuide#Teaching_Google_Test_How_to_Print_Your_Values

// For boost::exception diagnostic please see:
// http://www.boost.org/doc/libs/1_54_0/libs/exception/doc/diagnostic_information.html

template<typename T>
std::string tympan_to_string(const T& o)
{
    using namespace std;
    stringstream ss;
    ss << o;
    return ss.str();
}


namespace tympan
{

// We use the boost::exception to add information to exception classes.
// Cf. http://www.boost.org/doc/libs/1_54_0/libs/exception/doc/tutorial_transporting_data.html

/// The base exception class for all exceptions specific to Code_TYMPAN
struct exception: /* virtual std::exception,*/ virtual boost::exception {};

/// The base exception class for internal logic / algorithmic errors
struct logic_error: /*virtual*/ std::logic_error, virtual tympan::exception
{
    logic_error() : std::logic_error("Code_TYMPAN internal logic error") {};
    logic_error(const std::string& desc) : std::logic_error(desc) {};
};

/// The base exception class for errors due to invalid data
struct invalid_data: /*virtual*/ std::runtime_error, virtual tympan::exception
{
    invalid_data() : std::runtime_error("Code_TYMPAN invalid data encountered") {};
    invalid_data(const std::string& desc) : std::runtime_error(desc) {};
};

/* NB : the commented-out virtual inheritance in logic_error and invalid_data is
 * important : otherwise the classe derived from those would need to call the
 * std::logic_error or std::runtime_error constructor.
 * The point is std::logic_error and std::runtime_error inherits *non*-virtualy
 * from std::exception, so that the inheritance of tympan::exception from
 * has std::exception to be removed even if semantically it would be relevant.
 */


} //namespace tympan

/// This macro build a \c source_loc object to be attached to a \c tympan::Exception
#define tympan_source_loc  \
    ::boost::throw_function(BOOST_CURRENT_FUNCTION) <<\
                                                    ::boost::throw_file(__FILE__) <<\
                                                    ::boost::throw_line((int)__LINE__)


#endif // TY_EXCEPTIONS
