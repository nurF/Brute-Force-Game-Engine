/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2011 Brute-Force Games GbR

The BFG-Engine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

The BFG-Engine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the BFG-Engine. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef BFG_V3_H
#define BFG_V3_H

#include <cmath>
#include <ostream>

#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/util/write_dsv.hpp>

#include <Core/Types.h>
#include <Core/Defs.h>

namespace BFG {

template <typename T>
class vector3
{
public:
	vector3()
	{}

	vector3(const vector3<T>& rhs) :
	x(rhs.x),
	y(rhs.y),
	z(rhs.z)
	{}

	explicit vector3(T s) :
	x(s),
	y(s),
	z(s)
	{}

	vector3(T x, T y, T z) :
	x(x),
	y(y),
	z(z)
	{}
	
	vector3(const T* ptr) :
	x(ptr[0]),
	y(ptr[1]),
	z(ptr[2])
	{}
	
	const T* ptr() const
	{
		return &x;
	}
	
	T* ptr()
	{
		return &x;
	}
	
	vector3 operator + (const vector3& rhs) const
	{
		return vector3(
			x + rhs.x,
			y + rhs.y,
			z + rhs.z);
	}

	vector3 operator - (const vector3& rhs) const
	{
		return vector3(
			x - rhs.x,
			y - rhs.y,
			z - rhs.z);
	}

	vector3 operator * (const T scalar) const
	{
		return vector3(
			x * scalar,
			y * scalar,
			z * scalar);
	}
	
	vector3 operator * (const vector3& rhs) const
	{
		return vector3(
			x * rhs.x,
			y * rhs.y,
			z * rhs.z);
	}
	
	vector3 operator / (const T scalar) const
	{
		assert(scalar != 0.0);

		T inv = 1 / scalar;

		return vector3(
			x * inv,
			y * inv,
			z * inv);
	}
	
	vector3& operator += (const vector3& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;

		return *this;
	}
	
	vector3& operator -= (const vector3& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		return *this;
	}
	
	vector3& operator *= (const T scalar)
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}
	
	vector3& operator /= (const T scalar)
	{
		assert(scalar != 0.0f);

		T inv = 1.0f / scalar;

		x *= inv;
		y *= inv;
		z *= inv;

		return *this;
	}
	
	vector3<T> operator - ()
	{
		return vector3<T>(-x, -y, -z);
	}
	
	T x,y,z;

	static const vector3 ZERO;
	static const vector3 UNIT_X;
	static const vector3 UNIT_Y;
	static const vector3 UNIT_Z;
	static const vector3 NEGATIVE_UNIT_X;
	static const vector3 NEGATIVE_UNIT_Y;
	static const vector3 NEGATIVE_UNIT_Z;
	static const vector3 UNIT_SCALE;
};

template<typename T> const vector3<T> vector3<T>::ZERO(0.0f, 0.0f, 0.0f);
template<typename T> const vector3<T> vector3<T>::UNIT_X(1.0f, 0.0f, 0.0f);
template<typename T> const vector3<T> vector3<T>::UNIT_Y(0.0f, 1.0f, 0.0f);
template<typename T> const vector3<T> vector3<T>::UNIT_Z(0.0f, 0.0f, 1.0f);
template<typename T> const vector3<T> vector3<T>::NEGATIVE_UNIT_X(-1.0f,  0.0f,  0.0f);
template<typename T> const vector3<T> vector3<T>::NEGATIVE_UNIT_Y( 0.0f, -1.0f,  0.0f);
template<typename T> const vector3<T> vector3<T>::NEGATIVE_UNIT_Z( 0.0f,  0.0f, -1.0f);
template<typename T> const vector3<T> vector3<T>::UNIT_SCALE(1.0f, 1.0f, 1.0f);

typedef vector3<BFG::f32> v3;

template <typename T>
std::ostream& operator << (std::ostream& lhs, const vector3<T>& rhs)
{
	lhs << boost::geometry::dsv(rhs);
	return lhs;
}

//! Converts three separated values within a string into a v3. Examples:
//! \see stringToQuaternion4
//! "123.0, 614, .12"        // Ok!
//! "19521.9f, 0.001 111.09" // Ok!
//! \exception std::runtime_error If the string couldn't be parsed
BFG_CORE_API void stringToVector3(const std::string& input, v3& output);

} // namespace BFG

using BFG::v3;

BOOST_GEOMETRY_REGISTER_POINT_3D(v3, BFG::f32, cs::cartesian, x, y, z)


#endif
