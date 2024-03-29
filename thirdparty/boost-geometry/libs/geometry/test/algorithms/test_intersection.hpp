// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_INTERSECTION_HPP
#define BOOST_GEOMETRY_TEST_INTERSECTION_HPP

#include <fstream>
#include <iomanip>

#include <boost/foreach.hpp>
#include <geometry_test_common.hpp>

#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/length.hpp>
#include <boost/geometry/algorithms/num_points.hpp>
#include <boost/geometry/algorithms/unique.hpp>

#include <boost/geometry/geometries/geometries.hpp>

#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/extensions/gis/io/wkt/wkt.hpp>


#if defined(TEST_WITH_SVG)
#  include <boost/geometry/extensions/io/svg/svg_mapper.hpp>
#endif




template <typename OutputType, typename CalculationType, typename G1, typename G2>
double test_intersection(std::string const& caseid, G1 const& g1, G2 const& g2,
        std::size_t expected_count = 0, std::size_t expected_point_count = 0,
        double expected_length_or_area = 0,
        double percentage = 0.0001,
        bool make_unique = true)
{
    static const bool is_line = boost::geometry::geometry_id<OutputType>::type::value == 2;

    std::vector<OutputType> clip;

    typedef typename boost::geometry::coordinate_type<G1>::type coordinate_type;
    typedef typename boost::geometry::point_type<G1>::type point_type;

    typedef boost::geometry::strategy_intersection
        <
            typename boost::geometry::cs_tag<point_type>::type,
            G1,
            G2,
            point_type,
            CalculationType
        > strategy;

    boost::geometry::intersection_inserter<OutputType>(g1, g2, std::back_inserter(clip), strategy());

    double length_or_area = 0;
    std::size_t n = 0;
    for (typename std::vector<OutputType>::iterator it = clip.begin();
            it != clip.end();
            ++it)
    {
        if (expected_point_count > 0)
        {
            if (make_unique)
            {
                // Get a correct point-count without duplicate points
                // (note that overlay might be adapted to avoid duplicates)
                boost::geometry::unique(*it);
                n += boost::geometry::num_points(*it);
            }
            else
            {
                n += boost::geometry::num_points(*it);
            }
        }

        // instead of specialization we check it run-time here
        length_or_area += is_line
            ? boost::geometry::length(*it)
            : boost::geometry::area(*it);

        /*
        std::cout << std::endl << "case " << caseid << " ";
        std::cout
            << std::setprecision(20)
            << boost::geometry::dsv(*it) << std::endl;
        */
    }


#if ! defined(BOOST_GEOMETRY_NO_BOOST_TEST)
    if (expected_point_count > 0)
    {
        BOOST_CHECK_MESSAGE(n == expected_point_count,
                "intersection: " << caseid
                << " #points expected: " << expected_point_count
                << " detected: " << n
                << " type: " << string_from_type<coordinate_type>::name()
                );
    }

    if (expected_count > 0)
    {
        BOOST_CHECK_MESSAGE(clip.size() == expected_count,
                "intersection: " << caseid
                << " #outputs expected: " << expected_count
                << " detected: " << clip.size()
                << " type: " << string_from_type<coordinate_type>::name()
                );
    }

    BOOST_CHECK_CLOSE(length_or_area, expected_length_or_area, percentage);
#endif


#if defined(TEST_WITH_SVG)
    {
        std::ostringstream filename;
        filename << "intersection_"
            << caseid << "_"
            << string_from_type<coordinate_type>::name()
            << string_from_type<CalculationType>::name()
            << ".svg";

        std::ofstream svg(filename.str().c_str());

        boost::geometry::svg_mapper<point_type> mapper(svg, 500, 500);

        mapper.add(g1);
        mapper.add(g2);

        mapper.map(g1, is_line
            ? "opacity:0.6;stroke:rgb(0,0,255);stroke-width:5"
            : "opacity:0.6;fill:rgb(0,0,255);stroke:rgb(0,0,0);stroke-width:1");
        mapper.map(g2, "opacity:0.6;fill:rgb(0,255,0);stroke:rgb(0,0,0);stroke-width:1");

        for (typename std::vector<OutputType>::const_iterator it = clip.begin();
                it != clip.end(); ++it)
        {
            mapper.map(*it, "opacity:0.6;fill:none;stroke:rgb(255,0,0);stroke-width:5");
        }
    }
#endif

    return length_or_area;
}

template <typename OutputType, typename G1, typename G2>
double test_one(std::string const& caseid, std::string const& wkt1, std::string const& wkt2,
        std::size_t expected_count = 0, std::size_t expected_point_count = 0,
        double expected_length_or_area = 0,
        double percentage = 0.0001,
        bool make_unique = true)
{
    G1 g1;
    boost::geometry::read_wkt(wkt1, g1);

    G2 g2;
    boost::geometry::read_wkt(wkt2, g2);

    return test_intersection<OutputType, void>(caseid, g1, g2,
        expected_count, expected_point_count,
        expected_length_or_area, percentage, make_unique);
}



#endif
