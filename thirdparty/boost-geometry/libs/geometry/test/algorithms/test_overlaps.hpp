// Generic Geometry2 Library test file
//
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_OVERLAPS_HPP
#define BOOST_GEOMETRY_TEST_OVERLAPS_HPP


#include <geometry_test_common.hpp>

#include <boost/geometry/core/ring_type.hpp>
#include <boost/geometry/algorithms/overlaps.hpp>
#include <boost/geometry/strategies/strategies.hpp>
#include <boost/geometry/geometries/linear_ring.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>


template <typename Geometry1, typename Geometry2>
void test_geometry(std::string const& wkt1,
        std::string const& wkt2, bool expected)
{
    Geometry1 geometry1;
    Geometry2 geometry2;

    boost::geometry::read_wkt(wkt1, geometry1);
    boost::geometry::read_wkt(wkt2, geometry2);

    bool detected = boost::geometry::overlaps(geometry1, geometry2);

    BOOST_CHECK_MESSAGE(detected == expected,
        "overlaps: " << wkt1
        << " with " << wkt2
        << " -> Expected: " << expected
        << " detected: " << detected);
}


#endif
