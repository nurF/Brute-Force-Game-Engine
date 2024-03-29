// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <sstream>
#include <string>

#include <geometry_test_common.hpp>

#include <boost/geometry/iterators/ever_circling_iterator.hpp>

#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>
#include <boost/geometry/geometries/cartesian2d.hpp>

template <typename G>
void test_geometry(const std::string& wkt)
{
    G geo;
    boost::geometry::read_wkt(wkt, geo);
    typedef typename boost::range_const_iterator<G>::type iterator_type;


    // Run 3 times through the geometry
    int n = boost::size(geo) * 3;

    {
        std::ostringstream out;
        boost::geometry::ever_circling_iterator<iterator_type> it(boost::begin(geo), boost::end(geo));
        for (int i = 0; i < n; ++i, ++it)
        {
            out << boost::geometry::get<0>(*it);
        }
        BOOST_CHECK_EQUAL(out.str(), "123451234512345");
    }

    {
        std::ostringstream out;
        // Start somewhere
        boost::geometry::ever_circling_iterator<iterator_type> it(
            boost::begin(geo), boost::end(geo), boost::begin(geo) + 1);
        for (int i = 0; i < n; ++i, ++it)
        {
            out << boost::geometry::get<0>(*it);
        }
        BOOST_CHECK_EQUAL(out.str(), "234512345123451");
    }

    {
        std::ostringstream out;

        // Navigate to somewhere
        boost::geometry::ever_circling_iterator<iterator_type> it(boost::begin(geo), boost::end(geo));
        for (int i = 0; i < n; ++i, ++it)
        {
            const int m = boost::size(geo);
            it.moveto(boost::begin(geo) + m - (i % m) - 1);
            out << boost::geometry::get<0>(*it);
        }
        BOOST_CHECK_EQUAL(out.str(), "543215432154321");
    }

}

template <typename P>
void test_all()
{
    test_geometry<boost::geometry::linestring<P> >("linestring(1 1,2 2,3 3,4 4,5 5)");
}

int test_main(int, char* [])
{
    test_all<boost::geometry::point_2d>();

    return 0;
}
