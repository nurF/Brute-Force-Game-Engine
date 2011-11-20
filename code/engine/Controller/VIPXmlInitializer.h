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

#ifndef _VIPXMLINITIALIZER_H_
#define _VIPXMLINITIALIZER_H_

#include <vector>
#include <boost/foreach.hpp>
#include <boost/noncopyable.hpp>
#include <tinyxml.h>

#include <Controller/Action.h>
#include <Controller/VIPBase.h>
#include <Controller/VIPEnvironment.h>

namespace BFG {
namespace Controller_ {

class State;

namespace VIP {

struct VipData
{
	std::string mType;
	std::string mDevice;
	std::string mAxisMode;

	typedef std::map
	<
		std::string,
		std::string
	> SpecialAttributes;

	SpecialAttributes mSpecialAttributes;
	
	friend std::ostream& operator << (std::ostream& lhs, const VipData& rhs)
	{
		lhs << rhs.mType << "\n";
		lhs << rhs.mDevice << "\n";
		lhs << rhs.mAxisMode << "\n";
		lhs << "\n";
		
		typedef SpecialAttributes::value_type AttributeT;

		BOOST_FOREACH(const AttributeT& a, rhs.mSpecialAttributes)
		{
			lhs << a.first << " = " << a.second << "\n";
		}
		
		return lhs;
	}
};

class XmlInitializer : boost::noncopyable
{
public:
	XmlInitializer(State* state,
	               const ActionMapT& actionMap);

	//! Gets called when the parser stumbles upon an action element.
	//! \param[in] element Full path to the file containing vip configurations
	//! \param[out] vips   Adds all constructed Vips to this container
	void traverse(const std::string& fullConfigPath,
	              std::vector<VipPtrT>& vips) const;

private:
	//! Gets called when the parser stumbles upon an action element.
	//! \param[in] element   TinyXML pointer to an action element
	//! \param[out] actionId ID of the action
	//! \param[out] vips     Adds all constructed Vips to this container
	void onActionElement(TiXmlElement* element,
	                     EventIdT& actionId,
	                     std::vector<VipPtrT>& vips) const;

	//! Gets called when the parser stumbles upon a vip element.
	//! \param[in] element  TinyXML pointer to a vip element
	//! \param[in] actionId ID of the action
	//! \return    Fully constructed VIP
	VipPtrT onVipElement(TiXmlElement* element, EventIdT actionId) const;

	//! Gets called when the parser stumbles upon a special element.
	//! Currently, every vip element has a special element.
	//! \param[in] element  TinyXML pointer to a special element
	//! \param[out] vipData mSpecialAttributes gets filled with raw
	//!                     special attributes
	void onSpecialElement(TiXmlElement* element, VipData& vipData) const;
	
	//! Gets called when the parser stumbles upon an unknown element.
	void onUnknownElement(const std::string& tagName) const;

	//! Gets called when the parser stumbles upon a vip element.
	//! \param[in] element  TinyXML pointer to a vip element
	//! \param[in] actionId ID of the action
	//! \param[in] vipData  Must contain all parsed vip data
	//! \return    Fully constructed VIP
	VipPtrT createVip(EventIdT actionId, const VipData& vipData) const;

	void Init(SteerEnv& env, const VipData& vipData) const;
	void Init(ClickEnv& env, const VipData& vipData) const;
	void Init(ModClickEnv& env, const VipData& vipData) const;
	void Init(FeedbackEnv& env, const VipData& vipData) const;
	void Init(VAxisEnv& env, const VipData& vipData) const;
	void Init(VAxisSwingEnv& env, const VipData& vipData) const;
	void Init(SequenceEnv& env, const VipData& vipData) const;
	void Init(ToggleEnv& env, const VipData& vipData) const;

	void assignAxis(DefaultEnv& env,
	                const VipData& vipData,
	                const std::string& attrName) const;

	void assignAxisMode(DefaultEnv& env,
	                    const std::string& axisMode,
	                    ID::AxisMode defaultvalue) const;

	ButtonCodeT assignButton(DefaultEnv& env,
	                         const VipData& vipData,
	                         const std::string& code_str) const;

	void assignDevice(DefaultEnv& env, const std::string& device) const;

	void assignFilter(ClickEnv& env,
	                  const VipData& vipData) const;

	State*            mState;
	const ActionMapT& mActionMap;
};

} // namespace VIP
} // namespace Controller_
} // namespace BFG

#endif
