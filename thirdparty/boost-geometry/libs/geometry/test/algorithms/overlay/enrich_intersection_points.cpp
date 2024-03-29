// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>

#include <geometry_test_common.hpp>

#include <boost/geometry/algorithms/intersection.hpp>

//#include <boost/geometry/algorithms/overlay/get_intersection_points.hpp>
//#include <boost/geometry/algorithms/overlay/merge_intersection_points.hpp>
#include <boost/geometry/algorithms/overlay/enrich_intersection_points.hpp>

#include <boost/geometry/strategies/strategies.hpp>

#define GEOMETRY_TEST_OVERLAY_NOT_REVERSED

#include <overlay_common.hpp>

#include <boost/algorithm/string/replace.hpp>


#if defined(TEST_WITH_SVG)
#  include <boost/geometry/extensions/io/svg/svg_mapper.hpp>
#endif

struct test_enrich_intersection_points
{
    static inline std::string dir(int d)
    {
        return d == 0 ? "-" : (d == 1 ? "L" : "R");
    }

    template <typename G1, typename G2>
    static void apply(std::string const& id,
            boost::tuple<int, std::string> const& expected_count_and_center,
            G1 const& g1, G2 const& g2, double precision)
    {
        //std::cout << "#" << id << std::endl;

        typedef boost::geometry::detail::intersection::intersection_point
            <typename boost::geometry::point_type<G2>::type> ip;
        typedef typename boost::range_const_iterator<std::vector<ip> >::type iterator;
        std::vector<ip> ips;

        boost::geometry::get_intersection_points(g1, g2, ips);
        boost::geometry::merge_intersection_points(ips);
        boost::geometry::enrich_intersection_points(ips, true);

        std::ostringstream out;
        out << std::setprecision(2);

        bool first = true;
        for (iterator it = boost::begin(ips); it != boost::end(ips); ++it, first = false)
        {
            out << (first ? "" : ",");
            for (unsigned int i = 0; i < it->info.size(); i++)
            {
                out << dir(it->info[i].direction);
            }
        }
        int n = boost::size(ips);
        //std::cout << n << " " << out.str() << std::endl;
        BOOST_CHECK_EQUAL(expected_count_and_center.get<0>(), n);
        BOOST_CHECK_EQUAL(expected_count_and_center.get<1>(), out.str());



#if defined(TEST_WITH_SVG)
        {
            std::ostringstream filename;
            filename << "enrich_ip" << id << ".svg";

            std::ofstream svg(filename.str().c_str());

            boost::geometry::svg_mapper<typename boost::geometry::point_type<G2>::type> mapper(svg, 500, 500);
            mapper.add(g1);
            mapper.add(g2);

            mapper.map(g1, "fill:rgb(0,255,0);stroke:rgb(0,0,0);stroke-width:1");
            mapper.map(g2, "opacity:0.8;fill:rgb(0,0,255);stroke:rgb(0,0,0);stroke-width:1");

            for (iterator it = boost::begin(ips); it != boost::end(ips); ++it)
            {
                mapper.map(it->point, "fill:rgb(255,128,0);stroke:rgb(0,0,100);stroke-width:1");
            }
        }
#endif
    }
};


int test_main(int, char* [])
{
    std::vector<boost::tuple<int, std::string> > expected;

    /*
    TODO Will be refactored

    // NOTE: the order is sometimes not really important, in GCC it is sometimes
    // different from here...


    // 1-6
    expected.push_back(boost::make_tuple(6, "RL,LR,LR,RL,RL,LR"));
    expected.push_back(boost::make_tuple(8, "RL,LR,LR,RL,RL,LR,LR,RL"));
    expected.push_back(boost::make_tuple(4, "RLRR,RRRL,RRRL,RRRL"));
    expected.push_back(boost::make_tuple(12, "RL,LR,RL,LR,LR,RL,RL,LR,LR,RL,LR,RL"));
    expected.push_back(boost::make_tuple(17, "LR,RL,LR,RRLR,RL,LR,RL,RL,LR,LR,RL,LR,RL,RL,LR,RL,LR"));
    expected.push_back(boost::make_tuple(2, "--RR,LR"));

    // 7-12
    expected.push_back(boost::make_tuple(2, "LL,LL"));
    expected.push_back(boost::make_tuple(2, "RL--,LL--"));
    expected.push_back(boost::make_tuple(1, "RLLL"));
    expected.push_back(boost::make_tuple(2, "RL--,LL--"));
    expected.push_back(boost::make_tuple(1, "RRLR"));
    expected.push_back(boost::make_tuple(8, "RL,LR,RL,LR,RL,LR,RL,LR"));

    // 13-18
    expected.push_back(boost::make_tuple(2, "LL--,LL--"));
    expected.push_back(boost::make_tuple(2, "RL--,LL--"));
    expected.push_back(boost::make_tuple(2, "RL--,LL--"));
    expected.push_back(boost::make_tuple(2, "LL,--RL"));
    expected.push_back(boost::make_tuple(2, "RR--,--LR"));
    expected.push_back(boost::make_tuple(2, "RR--,--LR"));

    // 19-24
    expected.push_back(boost::make_tuple(2, "LL,LL"));
    expected.push_back(boost::make_tuple(0, ""));
    expected.push_back(boost::make_tuple(0, ""));
    expected.push_back(boost::make_tuple(1, "RLLLRRLR"));
    expected.push_back(boost::make_tuple(2, "RL,RLRRRRLR"));
    expected.push_back(boost::make_tuple(1, "LRRRRRLR"));

    // 25-30
    expected.push_back(boost::make_tuple(1, "LRRRLLRL"));
    expected.push_back(boost::make_tuple(1, "LRLLLLLR"));
    expected.push_back(boost::make_tuple(2, "LR,LRRRRRRL"));
    expected.push_back(boost::make_tuple(2, "LR,LRLLRRLR"));
    expected.push_back(boost::make_tuple(2, "RL,LRRRLLLR"));
    expected.push_back(boost::make_tuple(2, "LR,LRLLLLRL"));

    // 31-36
    expected.push_back(boost::make_tuple(1, "--LLLL--"));
    expected.push_back(boost::make_tuple(1, "LR--LLRL"));
    expected.push_back(boost::make_tuple(1, "LRLLLL--"));
    expected.push_back(boost::make_tuple(2, "LR,LRLLRR--"));
    expected.push_back(boost::make_tuple(1, "LRLLRRLR"));
    expected.push_back(boost::make_tuple(3, "RL,LR,RLLLRRLR"));

    // 37-42
    expected.push_back(boost::make_tuple(3, "LRRRRRLR,RL,LR"));
    expected.push_back(boost::make_tuple(3, "LR--RRRL,LR,RL"));
    expected.push_back(boost::make_tuple(3, "RL,LR,LRRRRRRL"));

    // 43-48
    expected.push_back(boost::make_tuple(4, "LR,RL,RL,LR"));

    // 49
    expected.push_back(boost::make_tuple(16, "--RL,RRLR,RRLR,RL,LLRL,RLLLRRLR,RR--,--LR,RLRR,--LL,RL--,RL,RRRL,RL,LR,RRRLRRRL"));

    // 101
    expected.push_back(boost::make_tuple(3, "RL,LR,RL"));

    // ticket#17
    expected.push_back(boost::make_tuple(6, "LR,RL,LR,RL,RL,LR"));

    //test_all<boost::geometry::point_xy<float>, test_enrich_intersection_points>(expected);
    test_all<boost::geometry::point_xy<double>, test_enrich_intersection_points>(expected);
    //test_all<boost::tuple<double, double>, test_enrich_intersection_points>(expected);

    */
    return 0;
}
