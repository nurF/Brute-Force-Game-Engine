// Boost.Geometry (aka GGL, Generic Geometry Library) Point concept test file
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/geometries/concepts/check.hpp>

struct ro_point
{
    float x, y;
};


struct rw_point
{
    float x, y;
};


namespace boost { namespace geometry { namespace traits {

template <> struct tag<ro_point> { typedef point_tag type; };
template <> struct coordinate_type<ro_point> { typedef float type; };
template <> struct coordinate_system<ro_point> { typedef boost::geometry::cs::cartesian type; };
template <> struct dimension<ro_point> { enum { value = 2 }; };

template <> struct access<ro_point, 0>
{
    static float get(ro_point const& p) { return p.x; }
};

template <> struct access<ro_point, 1>
{
    static float get(ro_point const& p) { return p.y; }
};




template <> struct tag<rw_point> { typedef point_tag type; };
template <> struct coordinate_type<rw_point> { typedef float type; };
template <> struct coordinate_system<rw_point> { typedef boost::geometry::cs::cartesian type; };
template <> struct dimension<rw_point> { enum { value = 2 }; };

template <> struct access<rw_point, 0>
{
    static float get(rw_point const& p) { return p.x; }
    static void set(rw_point& p, float value) { p.x = value; }
};

template <> struct access<rw_point, 1>
{
    static float get(rw_point const& p) { return p.y; }
    static void set(rw_point& p, float value) { p.y = value; }
};


}}} // namespace boost::geometry::traits


int main()
{
    boost::geometry::concept::check<const ro_point>();
    boost::geometry::concept::check<rw_point>();
}
