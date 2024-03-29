// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Custom pointer-to-point example

#include <iostream>

#include <boost/foreach.hpp>

#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/algorithms/length.hpp>
#include <boost/geometry/algorithms/make.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/geometries/cartesian2d.hpp>
#include <boost/geometry/geometries/adapted/std_as_linestring.hpp>
#include <boost/geometry/strategies/strategies.hpp>

// Sample point, having x/y
struct my_point
{
    double x,y;
};


namespace boost { namespace geometry { namespace traits {

template<> struct tag<my_point*>
{ typedef point_tag type; };

template<> struct coordinate_type<my_point*>
{ typedef double type; };

template<> struct coordinate_system<my_point*>
{ typedef cs::cartesian type; };

template<> struct dimension<my_point*> : boost::mpl::int_<2> {};

template<>
struct access<my_point*, 0>
{
    static double get(my_point const* p)
    {
        return p->x;
    }

    static void set(my_point* p, double const& value)
    {
        p->x = value;
    }
};

template<>
struct access<my_point*, 1>
{
    static double get(my_point const* p)
    {
        return p->y;
    }

    static void set(my_point* p, double const& value)
    {
        p->y = value;
    }
};

}}} // namespace boost::geometry::traits



int main()
{
    typedef std::vector<my_point*> ln;
    ln myline;
    for (float i = 0.0; i < 10.0; i++)
    {
        my_point* p = new my_point;
        p->x = i;
        p->y = i + 1;
        myline.push_back(p);
    }

    std::cout << boost::geometry::length(myline) << std::endl;

    boost::geometry::box_2d cb(boost::geometry::point_2d(1.5, 1.5), boost::geometry::point_2d(4.5, 4.5));

    // This will NOT work because would need dynamicly allocating memory for point* in algorithms:
    // std::vector<ln> clipped;
    //boost::geometry::intersection(cb, myline, std::back_inserter(clipped));

    // This works because outputs to a normal struct point, no point*
    std::vector<boost::geometry::linestring_2d> clipped;
    boost::geometry::strategy::intersection::liang_barsky<boost::geometry::box_2d, boost::geometry::point_2d> strategy;
    boost::geometry::detail::intersection::clip_linestring_with_box<boost::geometry::linestring_2d>(cb,
                    myline, std::back_inserter(clipped), strategy);


    std::cout << boost::geometry::length(clipped.front()) << std::endl;

    // free
    BOOST_FOREACH(my_point* p, myline)
    {
        delete p;
    }



    return 0;
}
