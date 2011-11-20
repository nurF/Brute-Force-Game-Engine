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

#include <Controller/VIPXmlInitializer.h>

#include <boost/lexical_cast.hpp>
#include <boost/typeof/typeof.hpp>
#include <Base/CLogger.h>
#include <Controller/State.h>
#include <Controller/VIPEnumToType.h>

namespace BFG {
namespace Controller_ {
namespace VIP {

namespace Tag
{
	const std::string vip("vip");
	const std::string action("action");
}

namespace Attr
{
	const std::string axis("axis");
	const std::string axismode("axismode");
	const std::string button("button");
	const std::string device("device");
	const std::string filter("filter");
	const std::string flip("flip");
	const std::string frequency("frequency");
	const std::string hold("hold");
	const std::string lower("lower");
	const std::string mod("mod");
	const std::string raise("raise");
	const std::string range("range");
	const std::string sensitivity("sensitivity");
	const std::string sequence("sequence");
	const std::string step("step");
	const std::string stopatzero("stopatzero");
	const std::string swing("swing");
	const std::string type("type");
}

namespace Def
{
	const ID::DeviceType device   = ID::DT_Keyboard;
	const ID::ButtonState filter  = ID::BS_Pressed;
	const bool flip               = false;
	const long frequency          = 100L;
	const bool hold              = false;
	const ID::KeyboardButton mod  = ID::KeyboardButton();
	const f32 range               = 1.0f;
	const f32 sensitivity         = 1.0f;
	const f32 step                = 0.1f;
	const bool stopatzero         = false;
	const f32 swing               = 0.0f;
}

template <typename T>
void assign(T& target,
            const VipData& vipData,
            const std::string& attributeName,
            const T& defaultValue)
{
	VipData::SpecialAttributes::const_iterator it =
		vipData.mSpecialAttributes.find(attributeName);
	
	if (it == vipData.mSpecialAttributes.end())
		target = defaultValue;
	else
		target = boost::lexical_cast<T>(it->second);
}

template <typename T>
void assign(T& target,
            const VipData& vipData,
            const std::string& attributeName)
{
	VipData::SpecialAttributes::const_iterator it =
		vipData.mSpecialAttributes.find(attributeName);
	
	if (it == vipData.mSpecialAttributes.end())
	{
		std::stringstream ss;
		ss << "Controller: Attribute \""
		  << attributeName
		  << "\" not found in Vip \""
		  << vipData.mType
		  << "\".";
		throw std::runtime_error(ss.str());
	}
	else
		target = boost::lexical_cast<T>(it->second);
}


XmlInitializer::XmlInitializer(State* state,
                               const ActionMapT& actionMap) :
mState(state),
mActionMap(actionMap)
{
}

void XmlInitializer::traverse(const std::string& fullConfigPath,
                              std::vector<VipPtrT>& vips) const
{
	TiXmlDocument xmlDocument(fullConfigPath);
	xmlDocument.LoadFile();
	TiXmlElement* root = xmlDocument.RootElement();

	if (!root)
	{
		std::stringstream ss;
		ss << "Controller: Root element of \"" << fullConfigPath
		   << "\" not found.";
		throw std::runtime_error(ss.str());
	}

	TiXmlElement* element = root->FirstChild()->ToElement();
	while (element)
	{
		try
		{
			const std::string& currentTag = element->ValueStr();

			EventIdT eventId;
			if (currentTag == Tag::action)
			{
				onActionElement(element, eventId, vips);
			}
			else
				onUnknownElement(currentTag);
		}
		catch (std::runtime_error& ex)
		{
			warnlog << ex.what();
		}

		element = element->NextSiblingElement();
	}
}

void XmlInitializer::onActionElement(TiXmlElement* element,
                                     EventIdT& actionId,
                                     std::vector<VipPtrT>& vips) const
{
	assert(element);

	TiXmlAttribute* attribute = element->FirstAttribute();
	std::string action = attribute->Value();

#ifdef CONTROLLER_DEBUG
	dbglog << "Controller: Parsing \"" << action << "\" ...";
#endif
	
	bool found = false;
	BOOST_FOREACH(const ActionMapT::value_type& vt, mActionMap)
	{
		if (vt.second == action)
		{
			actionId = vt.first;
			found = true;
			break;
		}
	}
	
	if (!found)
	{
#ifdef CONTROLLER_DEBUG
		static s32 imaginaryId = 0;
		++imaginaryId;
		actionId = imaginaryId;
		dbglog << " -> giving ActionId " << actionId;
#else
		throw std::runtime_error
		(
			"Controller: Unable to serialize action \"" +
			 action +
			 "\"."
		);
#endif
	}

	element = element->FirstChildElement();
	while (element)
	{
		const std::string& currentTag = element->ValueStr();

		if (currentTag == Tag::vip)
		{
			VipPtrT vip = onVipElement(element, actionId);
			vips.push_back(vip);
		}
		else
			onUnknownElement(currentTag);
		
		element = element->NextSiblingElement();
	}
}

VipPtrT XmlInitializer::onVipElement(TiXmlElement* element,
                                     EventIdT actionId) const
{
	assert(element);

	const std::string* type = element->Attribute(Attr::type);
	const std::string* device = element->Attribute(Attr::device);
	const std::string* axismode = element->Attribute(Attr::axismode);

	VipData vipData;
	
	if (type)
		vipData.mType = *type;
	else
		throw std::runtime_error
			("Controller: Stumbled upon \"vip\" element without \"type\" attribute.");

	if (device)
		vipData.mDevice = *device;

	if (axismode)
		vipData.mAxisMode = *axismode;

	element = element->FirstChildElement();
	if (! element)
		throw std::runtime_error
			("Controller: Stumbled upon \"vip\" element without \"special\" sub-element.");

	onSpecialElement(element, vipData);

	return createVip(actionId, vipData);
}

void XmlInitializer::onSpecialElement(TiXmlElement* element, VipData& vipData) const
{
	TiXmlAttribute* attribute = element->FirstAttribute();

	while (attribute)
	{
		const std::string& name = attribute->NameTStr();
		const std::string& value = attribute->ValueStr();
		
		vipData.mSpecialAttributes[name] = value;
		
		attribute = attribute->Next();
	}
}

void XmlInitializer::onUnknownElement(const std::string& tagName) const
{
	warnlog << "Controller: Skipping unknown tag \""
	        << tagName
	        << "\"";
}

#define VIP_CASE(EnumID)                             \
	case ID::EnumID:                             \
	{                                            \
		typedef Get<ID::EnumID>::Type VipT;  \
		typedef VipT::EnvT EnvT;             \
		EnvT env;                            \
		env.mAction = actionId;              \
		env.mState = mState;                 \
		Init(env, vipData);                  \
		vip.reset(new VipT(env));            \
	}                                            \
	break;

VipPtrT XmlInitializer::createVip(EventIdT actionId,
				  const VipData& vipData) const
{
	VipPtrT vip;
	ID::VIPType vipType = ID::asVIPType(vipData.mType);

	switch (vipType)
	{
		VIP_CASE(VIP_CLICK)
		VIP_CASE(VIP_HOLD)
		VIP_CASE(VIP_STEER)
		VIP_CASE(VIP_THRUST_AXIS)
		VIP_CASE(VIP_LINEAR_AXIS)
		VIP_CASE(VIP_CHEAT)
		VIP_CASE(VIP_TOGGLE)

		default:
			// Must be logic_error, since vipType has been interpreted yet.
			throw std::logic_error
				("Controller: Missing case for this vip type");
	}

	return vip;
}

void XmlInitializer::Init(SteerEnv& env, const VipData& vipData) const
{
	assignDevice(env, vipData.mDevice);
	assignAxis(env, vipData, Attr::axis);

	if (env.mDevice == ID::DT_Joystick)
		assignAxisMode(env, vipData.mAxisMode, ID::AM_Absolute);
	else
		assignAxisMode(env, vipData.mAxisMode, ID::AM_Relative);

	assign(env.mSensitivity, vipData, Attr::sensitivity, Def::sensitivity);
	assign(env.mRange, vipData, Attr::range, Def::range);
	assign(env.mFrequency, vipData, Attr::frequency, Def::frequency);
	assign(env.mSwing, vipData, Attr::swing, Def::swing);
	assign(env.mFlip, vipData, Attr::flip, Def::flip);
}

void XmlInitializer::Init(ClickEnv& env, const VipData& vipData) const
{
	assignDevice(env, vipData.mDevice);
	assignFilter(env, vipData);
	assignButton(env, vipData, Attr::button);
	
	if (env.mDevice != ID::DT_Keyboard)
		assignAxis(env, vipData, Attr::axis);
}

void XmlInitializer::Init(ModClickEnv& env, const VipData& vipData) const
{
	Init(static_cast<ClickEnv&>(env), vipData);
	
	// There's no default value for a button, so either use it or don't.
	if (vipData.mSpecialAttributes.find(Attr::mod) !=
	    vipData.mSpecialAttributes.end())
	{
		env.mMod = assignButton(env, vipData, Attr::mod);
	}
}

void XmlInitializer::Init(FeedbackEnv& env, const VipData& vipData) const
{
	assignDevice(env, vipData.mDevice);
	assignButton(env, vipData, Attr::button);
	assign(env.mFrequency, vipData, Attr::frequency, Def::frequency);
}

void XmlInitializer::Init(VAxisEnv& env, const VipData& vipData) const
{
	assignDevice(env, vipData.mDevice);

	ButtonCodeT lower = assignButton(env, vipData, Attr::lower);
	ButtonCodeT raise = assignButton(env, vipData, Attr::raise);

	assign(env.mFrequency, vipData, Attr::frequency, Def::frequency);
	assign(env.mStep, vipData, Attr::step);
	assignAxisMode(env, vipData.mAxisMode, ID::AM_Absolute);
	assign(env.mStopAtZero, vipData, Attr::stopatzero, Def::stopatzero);

	if (env.mAxisMode == ID::AM_Relative)
	{
		env.mStopAtZero = false;
		warnlog << "Controller: StopAtZero and AM_Relative are"
		           " mutually exclusive. Deactivated the former.";
	}

	env.mLower = lower;
	env.mRaise = raise;
}

void XmlInitializer::Init(VAxisSwingEnv& env, const VipData& vipData) const
{
	Init(static_cast<VAxisEnv&>(env), vipData);
	assign(env.mSwing, vipData, Attr::swing, Def::swing);
}

void XmlInitializer::Init(SequenceEnv& env, const VipData& vipData) const
{
	assignDevice(env, vipData.mDevice);
	assignFilter(env, vipData);
	
	VipData::SpecialAttributes::const_iterator it =
		vipData.mSpecialAttributes.find(Attr::sequence);
	
	if (it == vipData.mSpecialAttributes.end())
		throw std::runtime_error
			("Controller: Sequence-Vip is missing the sequence.");

	env.mSequence = it->second;
	
	for (size_t i=0; i<env.mSequence.length(); ++i)
		env.mRelevantButtons.insert(env.mSequence.at(i));
}

void XmlInitializer::Init(ToggleEnv& env, const VipData& vipData) const
{
	assignDevice(env, vipData.mDevice);
	assignButton(env, vipData, Attr::button);
	assign(env.mHold, vipData, Attr::hold, Def::hold);
}

void XmlInitializer::assignAxis(DefaultEnv& env,
                                const VipData& vipData,
				const std::string& attrName) const
{
	s32 axis;
	assign(axis, vipData, attrName);
	env.mRelevantAxis.insert(axis);
}

void XmlInitializer::assignAxisMode(DefaultEnv& env,
                                    const std::string& axisMode,
                                    ID::AxisMode defaultValue) const
{
	if (axisMode.empty())
		env.mAxisMode = defaultValue;
	else
		env.mAxisMode = ID::asAxisMode(axisMode);
}

ButtonCodeT XmlInitializer::assignButton(DefaultEnv& env,
					 const VipData& vipData,
                                         const std::string& button) const
{
	VipData::SpecialAttributes::const_iterator it =
		vipData.mSpecialAttributes.find(button);
	
	if (it == vipData.mSpecialAttributes.end())
	{
		warnlog << "Controller: Value missing for \""
		        << button << "\".";
		return (ButtonCodeT) 0;
	}

	const std::string& buttonValue = it->second;
	ButtonCodeT code;

	if (env.mDevice == ID::DT_Keyboard)
	{
		if (buttonValue.substr(0,3) == "KB_")
			code = ID::asKeyboardButton(buttonValue);
		else
			code = buttonValue[0];
	}
	else
	if (env.mDevice == ID::DT_Mouse)
	{
		OIS::MouseButtonID buttonid = ID::asMouseButtonID(buttonValue);
		code = static_cast<ButtonCodeT>(buttonid);
	}
	else
	if (env.mDevice == ID::DT_Joystick)
	{
		ID::JoystickButtonID buttonid = ID::asJoystickButtonID(buttonValue);
		code = static_cast<ButtonCodeT>(buttonid);
	}

	env.mRelevantButtons.insert(code);
	return code;
}

void XmlInitializer::assignDevice(DefaultEnv& env,
                                  const std::string& device) const
{
	if (device.empty())
		env.mDevice = Def::device;
	else
		env.mDevice = ID::asDeviceType(device);
}

void XmlInitializer::assignFilter(ClickEnv& env,
                                  const VipData& vipData) const
{
	VipData::SpecialAttributes::const_iterator it =
		vipData.mSpecialAttributes.find(Attr::filter);

	if (it == vipData.mSpecialAttributes.end())
		env.mFilter = Def::filter;
	else
		env.mFilter = ID::asButtonState(it->second);
}

} // namespace VIP
} // namespace Controller_
} // namespace BFG
