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

#ifndef BFG_VIEW_LIGHTCREATION_H
#define BFG_VIEW_LIGHTCREATION_H

#include <Core/ExternalTypes.h>
#include <Core/v3.h>

#include <View/Enums.hh>

namespace BFG {
namespace View {

struct VIEW_API LightCreation
{
	LightCreation(GameHandle handle, 
	              ID::LightType type,
				  const cv4& diffuseColor = cv4::White,
				  const cv4& specularColor = cv4::White,
				  f32 power = 1.0f) :
	mHandle(handle),
	mType(type),
	mDiffuse(diffuseColor),
	mSpecular(specularColor),
	mPower(power)
	{}
	
	               
	GameHandle mHandle;
	ID::LightType mType;
	cv4 mDiffuse;
	cv4 mSpecular;
	f32 mPower;
};

struct VIEW_API DirectionalLightCreation : public LightCreation
{
	DirectionalLightCreation(GameHandle handle,
	                         const v3& direction = v3::UNIT_Z,
							 const cv4& diffuseColor = cv4::White,
							 const cv4& specularColor = cv4::White,
							 f32 power = 1.0f) :
	LightCreation(handle, ID::LT_Directional, diffuseColor, specularColor, power),
	mDirection(direction)
	{}
	
	v3 mDirection;
};

struct VIEW_API PointLightCreation : public LightCreation 
{
	PointLightCreation(GameHandle handle,
	                   const v3& position,
	                   f32 range,
	                   f32 constant,
	                   f32 linear,
	                   f32 quadric,
	                   const cv4& diffuseColor = cv4::White,
	                   const cv4& specularColor = cv4::White,
	                   f32 power = 1.0f) :
	LightCreation(handle, ID::LT_Point, diffuseColor, specularColor, power),
	mPosition(position),
	mRange(range),
	mConstant(constant),
	mLinear(linear),
	mQuadric(quadric){}
	
	v3 mPosition;                 
	f32 mRange;
	f32 mConstant;
	f32 mLinear;
	f32 mQuadric;
};

struct VIEW_API SpotLightCreation : public LightCreation 
{
	SpotLightCreation(GameHandle handle,
	                  const v3& position,
	                  const v3& direction,
	                  f32 range,
	                  f32 constant,
	                  f32 linear,
	                  f32 quadric,
	                  f32 falloff,
	                  f32 innerRadius,
	                  f32 outerRadius,
	                  const cv4& diffuseColor = cv4::White,
	                  const cv4& specularColor = cv4::White,
	                  f32 power = 1.0f) :
	LightCreation(handle, ID::LT_Spot, diffuseColor, specularColor, power),
	mPosition(position),
	mDirection(direction),
	mRange(range),
	mConstant(constant),
	mLinear(linear),
	mQuadric(quadric),
	mFalloff(falloff),
	mInnerRadius(innerRadius),
	mOuterRadius(outerRadius){}
	
	v3 mPosition;
	v3 mDirection;
	f32 mRange;
	f32 mConstant;
	f32 mLinear;
	f32 mQuadric;
	f32 mFalloff;
	f32 mInnerRadius;
	f32 mOuterRadius;
};

} // namespace View
} // namespace BFG

#endif