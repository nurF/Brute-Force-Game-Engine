// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008-2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//#include <iostream>
#include <sstream>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/concept/requires.hpp>

#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/included/test_exec_monitor.hpp>

#include <boost/geometry/geometries/geometries.hpp>


#include <boost/geometry/multi/algorithms/area.hpp>
#include <boost/geometry/multi/algorithms/length.hpp>
#include <boost/geometry/multi/algorithms/num_points.hpp>
#include <boost/geometry/multi/algorithms/perimeter.hpp>
#include <boost/geometry/multi/core/point_type.hpp>
#include <boost/geometry/multi/core/topological_dimension.hpp>
#include <boost/geometry/multi/geometries/multi_point.hpp>
#include <boost/geometry/multi/geometries/multi_linestring.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>

#include <boost/geometry/extensions/gis/io/wkt/read_wkt_multi.hpp>
#include <boost/geometry/extensions/gis/io/wkt/write_wkt_multi.hpp>


template <typename T>
void test_all();


// Include the single test
#define GEOMETRY_TEST_MULTI
#include "extensions/gis/io/wkt/wkt.cpp"



template <typename T>
void test_all()
{
    using namespace boost::geometry;
    typedef point<T, 2, boost::geometry::cs::cartesian> P;

    test_wkt<multi_point<P> >("multipoint((1 2),(3 4))", 2);
    test_wkt<multi_linestring<linestring<P> > >("multilinestring((1 1,2 2,3 3),(4 4,5 5,6 6))", 6, 4 * sqrt(2.0));
    test_wkt<multi_polygon<polygon<P> > >("multipolygon(((0 0,0 2,2 2,2 0,0 0),(1 1,1 2,2 2,2 1,1 1)),((0 0,0 4,4 4,4 0,0 0)))", 15, 0, 19, 28);

    test_wrong_wkt<multi_polygon<polygon<P> > >(
        "MULTIPOLYGON(((0 0,0 2,2 2,2 0,0 0),(1 1,1 2,2 2,2 1,1 1)),(0 0,0 4,4 4,4 0,0 0)))",
        "expected '('");
}



/*

... see comments in "wkt.cpp"

union select 13,'# mpoint',npoints(geomfromtext('MULTIPOINT((1 2),(3 4))'))
union select 14,'length mpoint',length(geomfromtext('MULTIPOINT((1 2),(3 4))'))
union select 15,'peri mpoint',perimeter(geomfromtext('MULTIPOINT((1 2),(3 4))'))
union select 16,'area mpoint',area(geomfromtext('MULTIPOINT((1 2),(3 4))'))

union select 17,'# mls',npoints(geomfromtext('MULTILINESTRING((1 1,2 2,3 3),(4 4,5 5,6 6))'))
union select 18,'length mls',length(geomfromtext('MULTILINESTRING((1 1,2 2,3 3),(4 4,5 5,6 6))'))
union select 19,'peri mls',perimeter(geomfromtext('MULTILINESTRING((1 1,2 2,3 3),(4 4,5 5,6 6))'))
union select 20,'area mls',area(geomfromtext('MULTILINESTRING((1 1,2 2,3 3),(4 4,5 5,6 6))'))

union select 21,'# mpoly',npoints(geomfromtext('MULTIPOLYGON(((0 0,0 2,2 2,2 0,0 0),(1 1,1 2,2 2,2 1,1 1)),((0 0,0 4,4 4,4 0,0 0)))'))
union select 22,'length mpoly',length(geomfromtext('MULTIPOLYGON(((0 0,0 2,2 2,2 0,0 0),(1 1,1 2,2 2,2 1,1 1)),((0 0,0 4,4 4,4 0,0 0)))'))
union select 23,'peri mpoly',perimeter(geomfromtext('MULTIPOLYGON(((0 0,0 2,2 2,2 0,0 0),(1 1,1 2,2 2,2 1,1 1)),((0 0,0 4,4 4,4 0,0 0)))'))
union select 24,'area mpoly',area(geomfromtext('MULTIPOLYGON(((0 0,0 2,2 2,2 0,0 0),(1 1,1 2,2 2,2 1,1 1)),((0 0,0 4,4 4,4 0,0 0)))'))

*/
