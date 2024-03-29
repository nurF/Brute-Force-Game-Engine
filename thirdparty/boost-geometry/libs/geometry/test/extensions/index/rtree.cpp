// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Mateusz Loskot <mateusz@loskot.net>, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <cstddef>

#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/included/test_exec_monitor.hpp>

#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/cartesian2d.hpp>

#include <boost/geometry/extensions/index/rtree/rtree.hpp>

int test_main(int, char* [])
{

    // TODO: mloskot - This is ONLY compilation test and
    // placeholder to implement real test.

    boost::geometry::index::rtree<boost::geometry::box_2d, std::size_t> si(1, 6);
    return 0;
}
