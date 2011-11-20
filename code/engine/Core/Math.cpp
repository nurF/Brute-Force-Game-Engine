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

#include <Core/Math.h>

#include <boost/geometry/arithmetic/cross_product.hpp>
#include <boost/geometry/arithmetic/dot_product.hpp>
#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/strategies/cartesian/distance_pythagoras.hpp>

namespace BFG {

void ceil(v3& lhs, const v3& rhs)
{
	if (rhs.x > lhs.x) lhs.x = rhs.x;
	if (rhs.y > lhs.y) lhs.y = rhs.y;
	if (rhs.z > lhs.z) lhs.z = rhs.z;
}

void norm(v3& vec)
{
	f32 len = length(vec);
	assert(len > 0);
	vec /= len;
}

void norm(qv4& quat)
{
	f32 len = quat.w*quat.w+quat.x*quat.x+quat.y*quat.y+quat.z*quat.z;
	f32 factor = 1.0f / std::sqrt(len);
	quat = quat * factor;
}

void fromAngleAxis(qv4& result,
                   const f32 rfAngle,
                   const v3& rkAxis)
{
	// assert:  axis[] is unit length
	//
	// The quaternion representing the rotation is
	//   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)

	f32 fHalfAngle(0.5f * rfAngle);
	f32 fSin = std::sin(fHalfAngle);
	result.w = std::cos(fHalfAngle);
	result.x = fSin*rkAxis.x;
	result.y = fSin*rkAxis.y;
	result.z = fSin*rkAxis.z;
}

qv4 rotationTo(const v3& src,
               const v3& dest,
               const v3& fallbackAxis)
{
	// Based on Stan Melax's article in Game Programming Gems
	qv4 q;
	// Copy, since cannot modify local
	v3 v0 = src;
	v3 v1 = dest;
	norm(v0);
	norm(v1);

	f32 d = boost::geometry::dot_product(v0, v1);
	// If dot == 1, vectors are the same
	if (d >= 1.0f)
	{
		return qv4::IDENTITY;
	}
	if (d < (1e-6f - 1.0f))
	{
		if (! boost::geometry::equals(fallbackAxis, v3::ZERO))
		{
			// rotate 180 degrees about the fallback axis
			fromAngleAxis(q, (f32)M_PI, fallbackAxis);
		}
		else
		{
			// Generate an axis
			v3 axis = boost::geometry::cross_product(v3::UNIT_X, src);
			if (boost::geometry::distance(v3::ZERO, axis) < EPSILON_F) // pick another if colinear
				axis = boost::geometry::cross_product(v3::UNIT_Y, src);
			norm(axis);
			fromAngleAxis(q, (f32)M_PI, axis);
		}
	}
	else
	{
		f32 s = std::sqrt( (1+d)*2 );
		f32 invs = 1 / s;

		v3 c = boost::geometry::cross_product(v0, v1);

		q.x = c.x * invs;
		q.y = c.y * invs;
		q.z = c.z * invs;
		q.w = s * 0.5f;
		norm(q);
	}
	return q;
}

bool equals(const qv4& lhs, const qv4& rhs)
{
	if(std::abs(lhs.w - rhs.w) < EPSILON_F &&
	   std::abs(lhs.x - rhs.x) < EPSILON_F &&
	   std::abs(lhs.y - rhs.y) < EPSILON_F &&
	   std::abs(lhs.z - rhs.z) < EPSILON_F)
	   return true;

	return false;
}

f32 length(const v3& vec)
{
	return boost::geometry::distance(v3::ZERO, vec);
}

f32 distance(const v3& first, const v3& second)
{
	return boost::geometry::distance(first, second);
}

f32 angleBetween(const v3& src, const v3& dest)
{
	f32 lenProduct = length(src) * length(dest);

	// Divide by zero check
	if(lenProduct < 1e-6f)
		lenProduct = 1e-6f;

	f32 f = boost::geometry::dot_product(src, dest) / lenProduct;

	f = clamp(f, -1.0f, 1.0f);
	return std::acos(f);
}

v3 clamp(const v3& original, const v3& minVec, const v3& maxVec)
{
	v3 result;
	result.x = clamp(original.x, minVec.x, maxVec.x);
	result.y = clamp(original.y, minVec.y, maxVec.y);
	result.z = clamp(original.z, minVec.z, maxVec.z);
	return result;
}

qv4 unitInverse(const qv4& quat)
{
	// assert:  quat is unit length
	qv4 inf;
	inf.w = quat.w;
	inf.x = -quat.x;
	inf.y = -quat.y;
	inf.z = -quat.z;
	return inf;
}

qv4 eulerToQuaternion(const v3& euler)
{
	qv4 pitch = qv4::IDENTITY;
	fromAngleAxis(pitch, euler.x, v3::UNIT_X); 

	qv4 roll = qv4::IDENTITY; 
	fromAngleAxis(roll, euler.y, v3::UNIT_Y);

	qv4 yaw = qv4::IDENTITY; 
	fromAngleAxis(yaw, euler.z, v3::UNIT_Z); 

	return (pitch * roll * yaw);
}

bool nearEnough(const v3& position1,
                const v3& position2,
                f32 radius)
{
	f32 distance_squared = boost::geometry::distance
	(
		position1,
		position2,
		boost::geometry::strategy::distance::pythagoras<v3,v3>()
	).squared_value();

	bool near_enough = ! (distance_squared > (radius * radius));

	return near_enough;
}

}
