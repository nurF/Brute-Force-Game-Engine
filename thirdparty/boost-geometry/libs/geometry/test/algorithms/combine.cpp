// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <algorithms/test_combine.hpp>


#include <boost/geometry/algorithms/make.hpp>

#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>
#include <boost/geometry/geometries/adapted/std_pair_as_segment.hpp>
#include <test_common/test_point.hpp>



template <typename Point>
void test_point_3d()
{
    boost::geometry::box<Point> b = boost::geometry::make_inverse<boost::geometry::box<Point> >();

    test_combine<Point>(b, "POINT(1 2 5)", "(1,2,5),(1,2,5)");
    test_combine<Point>(b, "POINT(3 4 6)", "(1,2,5),(3,4,6)");

    test_combine<Point>(b, "POINT(4 4 5)", "(1,2,5),(4,4,6)");
    test_combine<Point>(b, "POINT(4 5 5)", "(1,2,5),(4,5,6)");
    test_combine<Point>(b, "POINT(10 10 4)", "(1,2,4),(10,10,6)");
    test_combine<Point>(b, "POINT(9 9 4)", "(1,2,4),(10,10,6)");

    test_combine<Point>(b, "POINT(0 2 7)", "(0,2,4),(10,10,7)");
    test_combine<Point>(b, "POINT(0 0 7)", "(0,0,4),(10,10,7)");
    test_combine<Point>(b, "POINT(-1 -1 5)", "(-1,-1,4),(10,10,7)");
    test_combine<Point>(b, "POINT(0 0 5)", "(-1,-1,4),(10,10,7)");

    test_combine<Point>(b, "POINT(15 -1 0)", "(-1,-1,0),(15,10,7)");
    test_combine<Point>(b, "POINT(-1 15 10)", "(-1,-1,0),(15,15,10)");
}

template <typename Point>
void test_box_3d()
{
    typedef boost::geometry::box<Point> box_type;
    box_type b = boost::geometry::make_inverse<box_type>();

    test_combine<box_type>(b, "BOX(0 2 5,4 4 6)",   "(0,2,5),(4,4,6)");
    test_combine<box_type>(b, "BOX(0 1 5,4 6 6)",   "(0,1,5),(4,6,6)");
    test_combine<box_type>(b, "BOX(-1 -1 6,10 10 5)", "(-1,-1,5),(10,10,6)");
    test_combine<box_type>(b, "BOX(3 3 6,3 3 5)",   "(-1,-1,5),(10,10,6)");

    test_combine<box_type>(b, "BOX(3 15 7,-1 3 4)", "(-1,-1,4),(10,15,7)");
    test_combine<box_type>(b, "BOX(-15 3 7,3 20 4)", "(-15,-1,4),(10,20,7)");
    test_combine<box_type>(b, "BOX(3 -20 8,3 20 3)", "(-15,-20,3),(10,20,8)");
    test_combine<box_type>(b, "BOX(-20 3 8,20 3 3)", "(-20,-20,3),(20,20,8)");
}



template <typename P>
void test_3d()
{
    test_point_3d<P>();
    test_box_3d<P>();
}

template <typename Point>
void test_2d()
{
    typedef boost::geometry::box<Point> box_type;
    typedef std::pair<Point, Point> segment_type;

    box_type b = boost::geometry::make_inverse<box_type>();

    test_combine<box_type>(b, "BOX(1 1,2 2)",   "(1,1),(2,2)");

    // Test an 'incorrect' box -> should also correctly update the bbox
    test_combine<box_type>(b, "BOX(3 4,0 1)",   "(0,1),(3,4)");

    // Test a segment
    test_combine<segment_type>(b, "SEGMENT(5 6,7 8)",   "(0,1),(7,8)");
}

template <typename Point>
void test_spherical_degree()
{
    boost::geometry::box<Point> b = boost::geometry::make_inverse<boost::geometry::box<Point> >();

    test_combine<Point>(b, "POINT(179.73 71.56)",
            "(179.73,71.56),(179.73,71.56)");
    test_combine<Point>(b, "POINT(177.47 71.23)",
            "(177.47,71.23),(179.73,71.56)");

    // It detects that this point is lying RIGHT of the others,
    //      and then it "combines" it.
    // It might be argued that "181.22" is displayed instead. However, they are
    // the same.
    test_combine<Point>(b, "POINT(-178.78 70.78)",
            "(177.47,70.78),(-178.78,71.56)");
}


template <typename Point>
void test_spherical_radian()
{
    boost::geometry::box<Point> b = boost::geometry::make_inverse<boost::geometry::box<Point> >();

    test_combine<Point>(b, "POINT(3.128 1.249)",
            "(3.128,1.249),(3.128,1.249)");
    test_combine<Point>(b, "POINT(3.097 1.243)",
            "(3.097,1.243),(3.128,1.249)");

    // It detects that this point is lying RIGHT of the others,
    //      and then it "combines" it.
    // It might be argued that "181.22" is displayed instead. However, they are
    // the same.
    test_combine<Point>(b, "POINT(-3.121 1.235)",
            "(3.097,1.235),(-3.121,1.249)");
}

int test_main(int, char* [])
{
    test_2d<boost::geometry::point<int, 2, boost::geometry::cs::cartesian> >();


    test_3d<test::test_point>();
    test_3d<boost::geometry::point<int, 3, boost::geometry::cs::cartesian> >();
    test_3d<boost::geometry::point<float, 3, boost::geometry::cs::cartesian> >();
    test_3d<boost::geometry::point<double, 3, boost::geometry::cs::cartesian> >();

    test_spherical_degree<boost::geometry::point<double, 2, boost::geometry::cs::spherical<boost::geometry::degree> > >();
    test_spherical_radian<boost::geometry::point<double, 2, boost::geometry::cs::spherical<boost::geometry::radian> > >();


/***
GMP/CLN still fails, due to boost::numeric_limits<T>, waiting for solution
#if defined(HAVE_CLN)
    test_3d<boost::geometry::point<boost::numeric_adaptor::cln_value_type,
            3, boost::geometry::cs::cartesian> >();
#endif
#if defined(HAVE_GMP)
    test_3d<boost::geometry::point<boost::numeric_adaptor::gmp_value_type,
            3, boost::geometry::cs::cartesian> >();
#endif
***/

    return 0;
}
