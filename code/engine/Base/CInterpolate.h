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

#ifndef __C_INTERPOLATE_H_
#define __C_INTERPOLATE_H_

namespace BFG {
namespace Base {


class CEaseInOutInterpolation
{
public:
	CEaseInOutInterpolation(float from, float to)
	{
		mMin = from;
		mMax = to;

		mAccel = (to-from) * 2 ;
	}

	float Interpolate(float delta)
	{
		if (delta <= 0.0f)
		{
			return mMin;
		}
		else if (delta >= 1.0f)
		{
			return mMax;
		}
		else
		{
			if (delta < 0.5f)
			{
				return mMin + (mAccel * delta * delta);
			}
			else
			{
				delta = 1.0f - delta;
				return mMax - (mAccel * delta * delta);
			}
		}
	}
private:
	float mMin;
	float mMax;
	float mAccel;
};

class CEaseInOutInterpolationM
{
public:
	CEaseInOutInterpolationM(float minMax) : mMinMax(0.0f, minMax)
	{
	}

	float Interpolate(float delta)
	{
		if (delta < 0.0f)
		{
			return -(mMinMax.Interpolate(-delta));
		}
		else
		{
			return mMinMax.Interpolate(delta);
		}
	}
private:
	CEaseInOutInterpolation mMinMax;
};

}
}

#endif