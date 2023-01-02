#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE SortingCppTests

#include <iostream>
#include <boost/test/unit_test.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

namespace logging = boost::log;

/*! Global testing definitions. */
struct GlobalFixture
{
    GlobalFixture()
    {
        logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::info);
    }

    ~GlobalFixture() {}
};

BOOST_GLOBAL_FIXTURE(GlobalFixture);
