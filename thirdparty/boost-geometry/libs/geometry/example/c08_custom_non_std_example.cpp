// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Custom polygon example

#ifndef _MSC_VER
#warning "Currently only works for MSVC"
int main() { return 0; }
#else

#include <iostream>

#include <boost/iterator.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/iterator_categories.hpp>
#include <boost/iterator/iterator_facade.hpp>


#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/register/ring.hpp>
#include <boost/geometry/util/add_const_if_c.hpp>

// Sample point, having x/y
struct my_point
{
    my_point(double a = 0, double b = 0)
        : x(a), y(b)
    {}
    double x,y;
};

// Sample polygon, having legacy methods
// (similar to e.g. COM objects)
class my_polygon
{
    std::vector<my_point> points;
    public :
        void add_point(my_point const& p) { points.push_back(p); }

        my_point const& get_point(std::size_t i) const
        {
            assert(i < points.size());
            return points[i];
        }

        // Non const access
        my_point & get_point(std::size_t i)
        {
            assert(i < points.size());
            return points[i];
        }


        int point_count() const { return points.size(); }
        void erase_all() { points.clear(); }


        // Note: it IS possible to have different method names;
        // however, there should (probably) be two different
        // iterators then or an iterator with a specified policy).
        // Note 2: if there is a set_point function, the iterator
        // does not have a way to dereference and non-const
        // iterators will not work!
};

// ----------------------------------------------------------------------------
// Adaption: implement iterator and range-extension, and register with GGL

// 1) implement iterator (const and non-const versions)
template <bool IsConst>
struct custom_iterator : public boost::iterator_facade
                            <
                                custom_iterator<IsConst>,
                                my_point,
                                boost::random_access_traversal_tag,
                                typename boost::geometry::add_const_if_c<IsConst, my_point>::type&
                            >
{
    // Constructor for begin()
    explicit custom_iterator(typename boost::geometry::add_const_if_c<IsConst, my_polygon>::type& polygon)
        : m_polygon(&polygon)
        , m_index(0)
    {}

    // Constructor for end()
    explicit custom_iterator(bool, typename boost::geometry::add_const_if_c<IsConst, my_polygon>::type& polygon)
        : m_polygon(&polygon)
        , m_index(polygon.point_count())
    {}



private:
    friend class boost::iterator_core_access;

    typedef boost::iterator_facade
        <
            custom_iterator<IsConst>,
            my_point,
            boost::random_access_traversal_tag,
            typename boost::geometry::add_const_if_c<IsConst, my_point>::type&
        > facade;

    typename boost::geometry::add_const_if_c<IsConst, my_polygon>::type* m_polygon;
    int m_index;

    bool equal(custom_iterator const& other) const
    {
        return this->m_index == other.m_index;
    }
    typename facade::difference_type distance_to(custom_iterator const& other) const
    {
        return other.m_index - this->m_index;
    }

    void advance(typename facade::difference_type n)
    {
        m_index += n;
        if(m_polygon != NULL
            && (m_index >= m_polygon->point_count()
            || m_index < 0)
            )
        {
            m_index = m_polygon->point_count();
        }
    }

    void increment()
    {
        advance(1);
    }

    void decrement()
    {
        advance(-1);
    }

    // const and non-const dereference of this iterator
    typename boost::geometry::add_const_if_c<IsConst, my_point>::type& dereference() const
    {
        return m_polygon->get_point(m_index);
    }
};




// 2) Implement Boost.Range const functionality
//    using method 2, "provide free-standing functions and specialize metafunctions"
// 2a) meta-functions
namespace boost
{
    template<> struct range_iterator<my_polygon>
    {
        typedef custom_iterator<false> type;
    };

    template<> struct range_const_iterator<my_polygon>
    {
        typedef custom_iterator<true> type;
    };

    // RangeEx
    template<> struct range_size<my_polygon>
    {
        typedef std::size_t type;
    };

} // namespace 'boost'


// 2b) free-standing function for Boost.Range ADP
inline custom_iterator<false> range_begin(my_polygon& polygon)
{
    return custom_iterator<false>(polygon);
}

inline custom_iterator<true> range_begin(my_polygon const& polygon)
{
    return custom_iterator<true>(polygon);
}

inline custom_iterator<false> range_end(my_polygon& polygon)
{
    return custom_iterator<false>(true, polygon);
}

inline custom_iterator<true> range_end(my_polygon const& polygon)
{
    return custom_iterator<true>(true, polygon);
}

// RangeEx
inline std::size_t range_size(my_polygon const& polygon)
{
    return polygon.point_count();
}


// 3) optional, for writable geometries only, implement back_inserter (=push_back)
class custom_insert_iterator
{
    my_polygon* m_polygon;
public:
    typedef std::output_iterator_tag iterator_category;

    // Not relevant for output iterator
    typedef void value_type;
    typedef void difference_type;
    typedef void pointer;
    typedef void reference;
    typedef void const_reference;

    explicit custom_insert_iterator(my_polygon& x)
        : m_polygon(&x)
    {}

    custom_insert_iterator& operator=(my_point const & p)
    {
        m_polygon->add_point(p);
        return *this;
    }

    custom_insert_iterator& operator*() { return *this; }
    custom_insert_iterator& operator++() { return *this; }
    custom_insert_iterator& operator++(int) { return *this; }
};


namespace std
{
    custom_insert_iterator back_inserter(my_polygon& polygon)
    {
        return custom_insert_iterator(polygon);
    }
}


// 4) register with GGL
BOOST_GEOMETRY_REGISTER_POINT_2D(my_point, double, cs::cartesian, x, y)
BOOST_GEOMETRY_REGISTER_RING(my_polygon)


// end adaption
// ----------------------------------------------------------------------------


void walk_using_iterator(my_polygon const& polygon)
{
    for (custom_iterator<true> it = custom_iterator<true>(polygon);
        it != custom_iterator<true>(true, polygon);
        ++it)
    {
        std::cout << boost::geometry::dsv(*it) << std::endl;
    }
    std::cout << std::endl;
}


void walk_using_range(my_polygon const& polygon)
{
    for (boost::range_iterator<const my_polygon>::type it
            = boost::begin(polygon);
        it != boost::end(polygon);
        ++it)
    {
        std::cout << boost::geometry::dsv(*it) << std::endl;
    }
    std::cout << std::endl;
}


int main()
{
    my_polygon container1;

    // Create (as an example) a regular polygon
    const int n = 5;
    const double d = (360 / n) * boost::geometry::math::d2r;
    double a = 0;
    for (int i = 0; i < n + 1; i++, a += d)
    {
        container1.add_point(my_point(sin(a), cos(a)));
    }

    std::cout << "Walk using Boost.Iterator derivative" << std::endl;
    walk_using_iterator(container1);

    std::cout << "Walk using Boost.Range extension" << std::endl << std::endl;
    walk_using_range(container1);

    std::cout << "Use it by GGL" << std::endl;
    std::cout << "Area: " << boost::geometry::area(container1) << std::endl;

    // Container 2 will be modified by GGL. Add all points but the last one.
    my_polygon container2;
    for (int i = 0; i < n; i++)
    {
        // Use here the std::/GGL way of inserting (but the my_polygon way of getting)
        *(std::back_inserter(container2)++) = container1.get_point(i);
    }

    std::cout << "Second container is not closed:" << std::endl;
    walk_using_range(container2);

    // Correct (= close it)
    boost::geometry::correct(container2);

    std::cout << "Now it is closed:" << std::endl;
    walk_using_range(container2);
    std::cout << "Area: " << boost::geometry::area(container2) << std::endl;

    // Use things from std:: using Boost.Range
    std::reverse(boost::begin(container2), boost::end(container2));
    std::cout << "Area reversed: " << boost::geometry::area(container2) << std::endl;

    return 0;
}


#endif
