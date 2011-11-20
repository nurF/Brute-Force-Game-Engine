// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_MULTI_ALGORITHMS_OVERLAY_SELF_TURN_POINTS_HPP
#define BOOST_GEOMETRY_MULTI_ALGORITHMS_OVERLAY_SELF_TURN_POINTS_HPP


#include <boost/geometry/multi/core/tags.hpp>
#include <boost/geometry/algorithms/overlay/self_turn_points.hpp>


namespace boost { namespace geometry
{




#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template
<
    typename MultiPolygon,
    typename IntersectionPoints,
    typename IntersectionStrategy,
    typename AssignPolicy,
    typename InterruptPolicy
>
struct self_get_turn_points
    <
        multi_polygon_tag, true, MultiPolygon,
        IntersectionPoints, IntersectionStrategy,
        AssignPolicy, InterruptPolicy
    >
    : detail::self_get_turn_points::get_turns
        <
            MultiPolygon,
            IntersectionPoints,
            IntersectionStrategy,
            AssignPolicy,
            InterruptPolicy
        >
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_MULTI_ALGORITHMS_OVERLAY_SELF_TURN_POINTS_HPP
