// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_EQUALS_HPP
#define BOOST_GEOMETRY_TEST_EQUALS_HPP


#include <geometry_test_common.hpp>

#include <boost/geometry/core/ring_type.hpp>
#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>


template <typename Geometry1, typename Geometry2>
void test_geometry(std::string const& caseid,
            std::string const& wkt1,
            std::string const& wkt2, bool expected)
{
    //std::cout << caseid << " expected: " << int(expected) << std::endl;
    //std::cout << wkt1 << std::endl;
    Geometry1 geometry1;
    Geometry2 geometry2;

    boost::geometry::read_wkt(wkt1, geometry1);
    boost::geometry::read_wkt(wkt2, geometry2);

    bool detected = boost::geometry::equals(geometry1, geometry2);

    BOOST_CHECK_MESSAGE(detected == expected,
        "case: " << caseid
        << " equals: " << wkt1
        << " to " << wkt2
        << " -> Expected: " << expected
        << " detected: " << detected);
}

#endif
