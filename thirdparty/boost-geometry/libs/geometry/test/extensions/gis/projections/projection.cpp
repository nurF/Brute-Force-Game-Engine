// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#if defined(_MSC_VER)
#pragma warning( disable : 4305 ) // truncation double -> float
#pragma warning( disable : 4244 ) // loss of data
#endif // defined(_MSC_VER)




#include <geometry_test_common.hpp>

#include <boost/geometry/extensions/gis/projections/parameters.hpp>
#include <boost/geometry/extensions/gis/projections/projection.hpp>
#include <boost/geometry/extensions/gis/projections/proj/aea.hpp>

#include <boost/geometry/algorithms/transform.hpp>
#include <boost/geometry/core/coordinate_type.hpp>

#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/adapted/c_array.hpp>
#include <boost/geometry/extensions/gis/latlong/point_ll.hpp>
#include <test_common/test_point.hpp>


template <typename Prj, typename P1, typename P2>
void test_one(double lon, double lat,
              typename boost::geometry::coordinate_type<P2>::type x,
              typename boost::geometry::coordinate_type<P2>::type y,
              const std::string& parameters)
{
    boost::geometry::projection::parameters par = boost::geometry::projection::detail::pj_init_plus(parameters);
    Prj prj(par);

    P1 ll;
    ll.lon(lon);
    ll.lat(lat);

    P2 xy;
    prj.forward(ll, xy);

    BOOST_CHECK_CLOSE(boost::geometry::get<0>(xy), x, 0.001);
    BOOST_CHECK_CLOSE(boost::geometry::get<1>(xy), y, 0.001);
}

template <typename P>
void test_all()
{
    typedef typename boost::geometry::coordinate_type<P>::type coord_type;
    typedef boost::geometry::point_ll<coord_type, boost::geometry::cs::geographic<boost::geometry::degree> > point_type;

    // aea
    test_one<boost::geometry::projection::aea_ellipsoid<point_type, P>, point_type, P>
        (4.897000, 52.371000, 334609.583974, 5218502.503686,
         "+proj=aea +ellps=WGS84 +units=m +lat_1=55 +lat_2=65");
}

int test_main(int, char* [])
{
    //test_all<int[2]>();
    test_all<float[2]>();
    test_all<double[2]>();
    test_all<test::test_point>();
    //test_all<boost::geometry::point_xy<int> >();
    test_all<boost::geometry::point_xy<float> >();
    test_all<boost::geometry::point_xy<double> >();
    test_all<boost::geometry::point_xy<long double> >();

    return 0;
}
