// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GEOMETRY_TEST_TEST_COMMON_TEST_POINT_HPP
#define GEOMETRY_TEST_TEST_COMMON_TEST_POINT_HPP

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/coordinate_system.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/tag.hpp>


namespace test
{

// Test point class

struct test_point
{
    float c1, c2, c3;
};


} // namespace test


namespace boost { namespace geometry { namespace traits {

template<>
struct tag<test::test_point> { typedef point_tag type; };

template<>
struct coordinate_type<test::test_point> { typedef float type; };

template<>
struct coordinate_system<test::test_point> { typedef cs::cartesian type; };

template<>
struct dimension<test::test_point>: boost::mpl::int_<3> {};

template<> struct access<test::test_point, 0>
{
    static inline const float& get(const test::test_point& p)
    {
        return p.c1;
    }

    static inline void set(test::test_point& p, const float& value)
    {
        p.c1 = value;
    }
};

template<> struct access<test::test_point, 1>
{
    static inline const float& get(const test::test_point& p)
    {
        return p.c2;
    }

    static inline void set(test::test_point& p, const float& value)
    {
        p.c2 = value;
    }
};

template<> struct access<test::test_point, 2>
{
    static inline const float& get(const test::test_point& p)
    {
        return p.c3;
    }

    static inline void set(test::test_point& p, const float& value)
    {
        p.c3 = value;
    }
};

}}} // namespace boost::geometry::traits

#endif // GEOMETRY_TEST_TEST_COMMON_TEST_POINT_HPP
