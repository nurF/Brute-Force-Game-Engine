// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels, Geodan B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <geometry_test_common.hpp>


#include <boost/geometry/algorithms/correct.hpp>
#include <boost/geometry/algorithms/within.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/cartesian2d.hpp>
#include <boost/geometry/geometries/linear_ring.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/multi/algorithms/within.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>

int test_main( int , char* [] )
{
    typedef boost::geometry::point_xy<double> gl_point;
    typedef boost::geometry::circle gl_circle;
    typedef boost::geometry::linear_ring<gl_point> gl_ring;
    typedef boost::geometry::polygon<gl_point> gl_polygon;
    typedef boost::geometry::multi_polygon<gl_polygon> gl_multi_polygon;

    gl_circle circle(gl_point(1, 1), 2.5);

    gl_ring ring;
    ring.push_back(gl_point(0,0));
    ring.push_back(gl_point(1,0));
    ring.push_back(gl_point(1,1));
    ring.push_back(gl_point(0,1));
    boost::geometry::correct(ring);

    gl_polygon pol;
    pol.outer() = ring;
    gl_multi_polygon multi_polygon;
    multi_polygon.push_back(pol);

    // Multipolygon in circle
    BOOST_CHECK_EQUAL(within(multi_polygon, circle), true);

    return 0;
}
