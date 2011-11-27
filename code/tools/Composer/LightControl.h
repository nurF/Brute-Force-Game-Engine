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

#ifndef LIGHTCONTROL
#define LIGHTCONTROL

#include <BaseFeature.h>

#include <View/Light.h>

#include <SharedData.h>


namespace Tool
{

class LightControl : public BaseFeature
{
public:
	LightControl(boost::shared_ptr<SharedData> data) :
	BaseFeature("Light", true),
	mData(data)
	{

	}

	virtual void load()
	{
		if (mLoaded)
			return;

		MyGUI::LayoutManager* layMan = MyGUI::LayoutManager::getInstancePtr();
		mContainer = layMan->load("Light.layout");

		mDiffuse = mGui->findWidget<MyGUI::Edit>("diffuse");
		mDiffuse->eventEditTextChange = 
			MyGUI::newDelegate(this, &LightControl::onDiffuseChange);

		mSpecular = mGui->findWidget<MyGUI::Edit>("specular");
		mSpecular->eventEditTextChange =
			MyGUI::newDelegate(this, &LightControl::onSpecularChange);

		mPower = mGui->findWidget<MyGUI::Edit>("power");
		mPosition = mGui->findWidget<MyGUI::Edit>("position");
		mDirection = mGui->findWidget<MyGUI::Edit>("direction");
		mRange = mGui->findWidget<MyGUI::Edit>("range");
		mConstant = mGui->findWidget<MyGUI::Edit>("constant");
		mLinear = mGui->findWidget<MyGUI::Edit>("linear");
		mQuadric = mGui->findWidget<MyGUI::Edit>("quadric");
		mFalloff = mGui->findWidget<MyGUI::Edit>("falloff");
		mInnerRadius = mGui->findWidget<MyGUI::Edit>("innerRadius");
		mOuterRadius = mGui->findWidget<MyGUI::Edit>("outerRadius");

		mDif = mGui->findWidget<MyGUI::StaticText>("tDif");
		mSpe = mGui->findWidget<MyGUI::StaticText>("tSpe");
		mPow = mGui->findWidget<MyGUI::StaticText>("tPow");
		mPos = mGui->findWidget<MyGUI::StaticText>("tPos");
		mDir = mGui->findWidget<MyGUI::StaticText>("tDir");
		mRan = mGui->findWidget<MyGUI::StaticText>("tRan");
		mCon = mGui->findWidget<MyGUI::StaticText>("tCon");
		mLin = mGui->findWidget<MyGUI::StaticText>("tLin");
		mQua = mGui->findWidget<MyGUI::StaticText>("tQua");
		mFal = mGui->findWidget<MyGUI::StaticText>("tFal");
		mInn = mGui->findWidget<MyGUI::StaticText>("tInn");
		mOut = mGui->findWidget<MyGUI::StaticText>("tOut");

		mSetAmbient = mGui->findWidget<MyGUI::Button>("setAmbient");
		mSetAmbient->eventMouseButtonClick = 
			MyGUI::newDelegate(this, &LightControl::onSetAmbient);
		mSetAmbient->eventMouseSetFocus =
			MyGUI::newDelegate(this, &LightControl::onSetFocus);
		mSetAmbient->eventMouseLostFocus =
			MyGUI::newDelegate(this, &LightControl::onLostFocus);

		mCreateDirection = mGui->findWidget<MyGUI::Button>("createDirection");
		mCreateDirection->eventMouseButtonClick = 
			MyGUI::newDelegate(this, &LightControl::onCreateDirection);
		mCreateDirection->eventMouseSetFocus =
			MyGUI::newDelegate(this, &LightControl::onSetFocus);
		mCreateDirection->eventMouseLostFocus =
			MyGUI::newDelegate(this, &LightControl::onLostFocus);

		mCreatePoint = mGui->findWidget<MyGUI::Button>("createPoint");
		mCreatePoint->eventMouseButtonClick =
			MyGUI::newDelegate(this, &LightControl::onCreatePoint);
		mCreatePoint->eventMouseSetFocus =
			MyGUI::newDelegate(this, &LightControl::onSetFocus);
		mCreatePoint->eventMouseLostFocus =
			MyGUI::newDelegate(this, &LightControl::onLostFocus);

		mCreateSpot = mGui->findWidget<MyGUI::Button>("createSpot");
		mCreateSpot->eventMouseButtonClick =
			MyGUI::newDelegate(this, &LightControl::onCreateSpot);
		mCreateSpot->eventMouseSetFocus =
			MyGUI::newDelegate(this, &LightControl::onSetFocus);
		mCreateSpot->eventMouseLostFocus =
			MyGUI::newDelegate(this, &LightControl::onLostFocus);

		mColDif = mGui->findWidgetT("colDif");
		mColSpec = mGui->findWidgetT("colSpec");

		mLoaded = true;
		deactivate();
	}

	virtual void unload()
	{
		if (!mLoaded)
			return;

		if (mActive)
			deactivate();

		mLoaded = false;
	}

	virtual void activate()
	{
		MyGUI::VectorWidgetPtr::iterator it = mContainer.begin();
		for (; it != mContainer.end(); ++it)
		{
			(*it)->setVisible(true);
		}
		mActive = true;
	}
	virtual void deactivate()
	{
		MyGUI::VectorWidgetPtr::iterator it = mContainer.begin();
		for (; it != mContainer.end(); ++it)
		{
			(*it)->setVisible(false);
		}
		mActive = false;
	}

	virtual void eventHandler(BFG::Controller_::VipEvent* ve)
	{

	}

private:

	void onDiffuseChange(MyGUI::Edit* edit)
	{
		BFG::cv4 col = MyGUI::utility::parseValueEx3<BFG::cv4, float>(edit->getCaption());
		mColDif->setColour(MyGUI::Colour(col.r, col.g, col.b));
	}

	void onSpecularChange(MyGUI::Edit* edit)
	{
		BFG::cv4 col = MyGUI::utility::parseValueEx3<BFG::cv4, float>(edit->getCaption());
		mColSpec->setColour(MyGUI::Colour(col.r, col.g, col.b));
	}

	void onSetAmbient(MyGUI::Widget* button)
	{
		BFG::cv4 diffuse = MyGUI::utility::parseValueEx3<BFG::cv4, float>(mDiffuse->getCaption());
		
		Ogre::SceneManager* sceneMgr = 
			Ogre::Root::getSingletonPtr()->getSceneManager(BFG_SCENEMANAGER);

		sceneMgr->setAmbientLight(diffuse);
	}

	void onCreateDirection(MyGUI::Widget* button)
	{
		BFG::v3 direction = MyGUI::utility::parseValueEx3<BFG::v3, float>(mDirection->getCaption());
		BFG::cv4 diffuse = MyGUI::utility::parseValueEx3<BFG::cv4, float>(mDiffuse->getCaption());
		BFG::cv4 specular = MyGUI::utility::parseValueEx3<BFG::cv4, float>(mSpecular->getCaption());
		f32 power = MyGUI::utility::parseFloat(mPower->getCaption());

		BFG::View::DirectionalLightCreation dlc
		(
			generateHandle(),
			direction,
			diffuse,
			specular,
			power
		);

		mLights.push_back(new BFG::View::Light(dlc));
		deactivate();
	}

	void onCreatePoint(MyGUI::Widget* button)
	{
		BFG::v3 position = MyGUI::utility::parseValueEx3<BFG::v3, float>(mPosition->getCaption());
		BFG::cv4 diffuse = MyGUI::utility::parseValueEx3<BFG::cv4, float>(mDiffuse->getCaption());
		BFG::cv4 specular = MyGUI::utility::parseValueEx3<BFG::cv4, float>(mSpecular->getCaption());
		f32 power = MyGUI::utility::parseFloat(mPower->getCaption());
		f32 range = MyGUI::utility::parseFloat(mRange->getCaption());
		f32 constant = MyGUI::utility::parseFloat(mConstant->getCaption());
		f32 linear = MyGUI::utility::parseFloat(mLinear->getCaption());
		f32 quadric = MyGUI::utility::parseFloat(mQuadric->getCaption());

		BFG::View::PointLightCreation plc
		(
			generateHandle(),
			position,
			range,
			constant,
			linear,
			quadric,
			diffuse,
			specular,
			power
		);

		mLights.push_back(new BFG::View::Light(plc));
		deactivate();
	}

	void onCreateSpot(MyGUI::Widget* button)
	{
		BFG::v3 position = MyGUI::utility::parseValueEx3<BFG::v3, float>(mPosition->getCaption());
		BFG::v3 direction = MyGUI::utility::parseValueEx3<BFG::v3, float>(mDirection->getCaption());
		BFG::cv4 diffuse = MyGUI::utility::parseValueEx3<BFG::cv4, float>(mDiffuse->getCaption());
		BFG::cv4 specular = MyGUI::utility::parseValueEx3<BFG::cv4, float>(mSpecular->getCaption());
		f32 power = MyGUI::utility::parseFloat(mPower->getCaption());
		f32 range = MyGUI::utility::parseFloat(mRange->getCaption());
		f32 constant = MyGUI::utility::parseFloat(mConstant->getCaption());
		f32 linear = MyGUI::utility::parseFloat(mLinear->getCaption());
		f32 quadric = MyGUI::utility::parseFloat(mQuadric->getCaption());
		f32 falloff = MyGUI::utility::parseFloat(mFalloff->getCaption());
		f32 inner = MyGUI::utility::parseFloat(mInnerRadius->getCaption());
		f32 outer = MyGUI::utility::parseFloat(mOuterRadius->getCaption());

		BFG::View::SpotLightCreation slc
		(
			generateHandle(),
			position,
			direction,
			range,
			constant,
			linear,
			quadric,
			falloff,
			inner,
			outer,
			diffuse,
			specular,
			power
		);

		mLights.push_back(new BFG::View::Light(slc));
		deactivate();
	}

	void onLostFocus(MyGUI::Widget* button, MyGUI::Widget*)
	{
		MyGUI::LanguageManager* langMan = MyGUI::LanguageManager::getInstancePtr();
		std::string tag = langMan->getTag("BFE_Text_ColourNormal");
		MyGUI::Colour col(tag);

		setTextColour(button, col);
	}

	void onSetFocus(MyGUI::Widget* button, MyGUI::Widget*)
	{
		MyGUI::LanguageManager* langMan = MyGUI::LanguageManager::getInstancePtr();
		std::string tag = langMan->getTag("BFE_Text_ColourFocused");
		MyGUI::Colour col(tag);

		setTextColour(button, col);
	}

	void setTextColour(MyGUI::Widget* button, const MyGUI::Colour& colour)
	{
		if (button == mSetAmbient)
		{
			mDif->setTextColour(colour);
		}
		else if (button == mCreateDirection)
		{
			mDif->setTextColour(colour);
			mSpe->setTextColour(colour);
			mPow->setTextColour(colour);
			mDir->setTextColour(colour);
		}
		else if (button == mCreatePoint)
		{
			mDif->setTextColour(colour);
			mSpe->setTextColour(colour);
			mPow->setTextColour(colour);
			mPos->setTextColour(colour);
			mRan->setTextColour(colour);
			mCon->setTextColour(colour);
			mLin->setTextColour(colour);
			mQua->setTextColour(colour);
		}
		else if (button == mCreateSpot)
		{
			mDif->setTextColour(colour);
			mSpe->setTextColour(colour);
			mPow->setTextColour(colour);
			mPos->setTextColour(colour);
			mDir->setTextColour(colour);
			mRan->setTextColour(colour);
			mCon->setTextColour(colour);
			mLin->setTextColour(colour);
			mQua->setTextColour(colour);
			mFal->setTextColour(colour);
			mInn->setTextColour(colour);
			mOut->setTextColour(colour);
		}
	}

	boost::shared_ptr<SharedData> mData;

	MyGUI::Edit* mDiffuse;
	MyGUI::Edit* mSpecular;
	MyGUI::Edit* mPower;
	MyGUI::Edit* mPosition;
	MyGUI::Edit* mDirection;
	MyGUI::Edit* mRange;
	MyGUI::Edit* mConstant;
	MyGUI::Edit* mLinear;
	MyGUI::Edit* mQuadric;
	MyGUI::Edit* mFalloff;
	MyGUI::Edit* mInnerRadius;
	MyGUI::Edit* mOuterRadius;

	MyGUI::StaticText* mDif;
	MyGUI::StaticText* mSpe;
	MyGUI::StaticText* mPow;
	MyGUI::StaticText* mPos;
	MyGUI::StaticText* mDir;
	MyGUI::StaticText* mRan;
	MyGUI::StaticText* mCon;
	MyGUI::StaticText* mLin;
	MyGUI::StaticText* mQua;
	MyGUI::StaticText* mFal;
	MyGUI::StaticText* mInn;
	MyGUI::StaticText* mOut;

	MyGUI::Button* mSetAmbient;
	MyGUI::Button* mCreateDirection;
	MyGUI::Button* mCreatePoint;
	MyGUI::Button* mCreateSpot;

	MyGUI::Widget* mColDif;
	MyGUI::Widget* mColSpec;

	std::vector<BFG::View::Light* > mLights;


}; // class LightControl

} // namespace Tool
#endif