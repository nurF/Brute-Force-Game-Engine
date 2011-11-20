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

#include <OgreException.h>

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/time.hpp>

#include <limits>

#include <OgreBlendMode.h>
#include <OgreEntity.h>
#include <OgreMaterialManager.h>
#include <OgreMeshManager.h>
#include <OgreParticleSystem.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreSubEntity.h>

#include <MyGUI.h>

#include <tinyxml.h>

#include <Base/CEntryPoint.h>
#include <Base/CLogger.h>
#include <Base/Cpp.h>
#include <Base/Pause.h>
#include <Controller/Action.h>
#include <Controller/ControllerEvents.h>
#include <Controller/ControllerInterface.h>
#include <EventSystem/Core/EventLoop.h>
#include <EventSystem/Emitter.h>
#include <EventSystem/EventFactory.h>
#include <Core/ClockUtils.h>
#include <Core/Location.h>
#include <Core/Math.h>
#include <Core/Mesh.h>
#include <Core/Path.h>
#include <Core/ShowException.h>
#include <Core/Types.h>
#include <Core/Utils.h>
#include <Model/Environment.h>
#include <Model/GameObject.h>
#include <Model/Property/SpacePlugin.h>
#include <View/Camera.h>
#include <View/CameraCreation.h>
#include <View/ControllerMyGuiAdapter.h>
#include <View/Convert.h>
#include <View/Event.h>
#include <View/Explosion.h>
#include <View/Interface.h>
#include <View/Light.h>
#include <View/LoadMesh.h>
#include <View/Main.h>
#include <View/Owner.h>
#include <View/RenderObject.h>
#include <View/SkyCreation.h>
#include <View/Skybox.h>
#include <View/State.h>

#include <OpenSaveDialog.h>

using namespace BFG;
using namespace boost::units;

const s32 A_QUIT = 10000;
const s32 A_LOADING_MESH = 10001;
const s32 A_LOADING_MATERIAL = 10002;
const s32 A_CAMERA_AXIS_X = 10003;
const s32 A_CAMERA_AXIS_Y = 10004;
const s32 A_CAMERA_AXIS_Z = 10005;
const s32 A_CAMERA_MOVE = 10006;
const s32 A_CAMERA_RESET = 10007;
const s32 A_CAMERA_ORBIT = 10008;
const s32 A_SCREENSHOT = 10009;
const s32 A_LOADING_SKY = 10010;

const s32 A_CREATE_LIGHT = 10011;
const s32 A_DESTROY_LIGHT = 10012;
const s32 A_PREV_LIGHT = 10013;
const s32 A_NEXT_LIGHT = 10014;
const s32 A_FIRST_LIGHT = 10015;
const s32 A_LAST_LIGHT = 10016;
const s32 A_INFO_WINDOW = 10017;

const s32 A_CAMERA_MOUSE_X = 10018;
const s32 A_CAMERA_MOUSE_Y = 10019;
const s32 A_CAMERA_MOUSE_Z = 10020;
const s32 A_CAMERA_MOUSE_MOVE = 10021;

const s32 A_SUB_MESH = 10022;
const s32 A_TEX_UNIT = 10023;
const s32 A_ANIMATION = 10024;
const s32 A_ADAPTER = 10025;

const s32 A_MOUSE_MIDDLE_PRESSED = 10026;

const f32 DEFAULT_CAM_DISTANCE = 10.0f;
const f32 DEFAULT_CAM_VELOCITY = 2.0f; // m/s

const std::string DEFAULT_TEXTURE_MAP = "white.png";
const std::string DEFAULT_NORMAL_MAP = "flat_n.png";
const std::string DEFAULT_ILLUMINATION_MAP = "black.png";


struct TextureUnitChange
{
	std::string mMaterial;
	std::string mAlias;
	std::string mTexture;
	bool mEnabled;
};

struct Settings
{
	std::string mMesh;
	std::string mMaterial;
	std::string mSkybox;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & mMesh;
		ar & mMaterial;
		ar & mSkybox;
	}
};

struct ConnectionData
{
	u32 mAdapterID;
	u32 mParentAdapterID;
};

struct ComposerState : Emitter
{
	ComposerState(GameHandle handle, EventLoop* loop) :
	Emitter(loop),
	mClock(new Clock::StopWatch(Clock::milliSecond)),
	mExitNextTick(false)
	{
		mClock->start();
	}

	void ControllerEventHandler(Controller_::VipEvent* iCE)
	{
		switch(iCE->getId())
		{
			case A_QUIT:
			{
				mExitNextTick = true;
				emit<BFG::View::Event>(BFG::ID::VE_SHUTDOWN, 0);
				break;
			}
			case A_SCREENSHOT:
			{
				emit<BFG::View::Event>(BFG::ID::VE_SCREENSHOT, 0);
				break;
			}
		}
	}

	void LoopEventHandler(LoopEvent* iLE)
	{
		if (mExitNextTick)
		{
			// Error happened, while doing stuff
			iLE->getData().getLoop()->setExitFlag();
		}

		long timeSinceLastFrame = mClock->stop();
		if (timeSinceLastFrame)
			mClock->start();

		f32 timeInSeconds = static_cast<f32>(timeSinceLastFrame) / Clock::milliSecond;
		tick(timeInSeconds);
	}
		
	void tick(const f32 timeSinceLastFrame)
	{
		if (timeSinceLastFrame < EPSILON_F)
			return;

		quantity<si::time, f32> TSLF = timeSinceLastFrame * si::seconds;
	}

	boost::scoped_ptr<Clock::StopWatch> mClock;
	
	bool mExitNextTick;
};

void allFilesOfDirectory(std::vector<std::string>& result,
						 const std::string& location)
{
	boost::filesystem::directory_iterator it(location), end;

	for (; it != end; ++it)
	{
		if (boost::filesystem::is_directory(*it))
			continue;

		const std::string directoryName = it->path().filename().string();
		result.push_back(it->path().filename().string());
	}
}

void loadLayout(const std::string& fileName, MyGUI::VectorWidgetPtr& container)
{
	BFG::Path path;
	std::string layout = path.Expand(fileName);
	container = MyGUI::LayoutManager::getInstance().load(layout);
}

struct ViewComposerState : public View::State
{
public:

	typedef std::map<std::string, std::vector<BFG::Adapter> > AdapterMapT;

	ViewComposerState(GameHandle handle, EventLoop* loop) :
	State(handle, loop),
	mLoop(loop),
	mMainObject(NULL),
	mObject(generateHandle()),
	mCamHandle(generateHandle()),
	mControllerAdapter(handle, loop),
	mCameraPosition(NULL),
	mCameraRotation(NULL),
	mCameraDistance(NULL),
	mSelectedSubEntity(NULL),
	mDeltaRot(v3::ZERO),
	mDeltaDis(0.0f),
	mCamDistance(DEFAULT_CAM_DISTANCE),
	mCamOrbit(false),
	mInfoChanged(false),
	mMouseCamPitchYaw(false),
	mMouseCamRoll(false),
	mIsZooming(false),
	mShowingMeshLoadingMenu(false),
	mShowingMaterialLoadingMenu(false),
	mShowingSkyLoadingMenu(false),
	mShowingLightCreatingMenu(false),
	mShowingInfoWindow(false),
	mShowingSubMeshMenu(false),
	mShowingTextureUnitMenu(false),
	mShowingAnimationMenu(false),
	mShowingAdapterMenu(false),
	mShowingPreviewMenu(false),
	mShowingLoadPreviewMeshMenu(false),
	mShowingLoadPreviewAdapterMenu(false),
	mGui(MyGUI::Gui::getInstance()),
	mAnimationLoop(false),
	mAnimationState(NULL),
	mPositionAdapter(false),
	mAdapterSelected(0),
	mMenuFocus(false)
	{
		mEnvironment.reset(new Environment);

		registerEventHandler();
		createDefaultCamera();

		createGui();

		// create picking values
		Ogre::SceneManager* sceneMan = 
			Ogre::Root::getSingleton().getSceneManager(BFG_SCENEMANAGER);
		Ogre::SceneNode* rootSceneNode = sceneMan->getRootSceneNode();
		mPickingNode = rootSceneNode->createChildSceneNode("PickingNode");
		Ogre::Entity* ent = sceneMan->createEntity("Sphere", "Marker.mesh");
		mPickingNode->attachObject(ent);
		mPickingNode->setDirection(Ogre::Vector3::UNIT_Y);
		mPickingNode->setVisible(false);
	}

	~ViewComposerState()
	{
		unregisterEventHandler();
		
		BOOST_FOREACH(TextureUnitChange* tex, mTextureChanges)
		{
			delete tex;
		}

		mTextureChanges.clear();
	}

	void createDefaultCamera()
	{
		Ogre::SceneManager* sceneMgr = 
			Ogre::Root::getSingleton().getSceneManager(BFG_SCENEMANAGER);

		mCameraPosition = sceneMgr->getRootSceneNode()->createChildSceneNode();
		mCameraPosition->setPosition(0, 0, 0);

		mCameraRotation = mCameraPosition->createChildSceneNode();

		mCameraDistance = mCameraRotation->createChildSceneNode(stringify(mCamHandle));

		View::CameraCreation cc
		(
			mCamHandle,
			mCamHandle,
			true,
			0,
			0
		);
		
		createCamera(cc);

		Ogre::Vector3 pos(0.0f, 0.0f, -mCamDistance);
		mCameraDistance->setPosition(pos);
	}

	void lostFocus(MyGUI::Widget*, MyGUI::Widget*)
	{
		mMenuFocus = false;
	}

	void gotFocus(MyGUI::Widget*, MyGUI::Widget*)
	{
		mMenuFocus = true;
	}

	void createGui()
	{
		loadLayout("Composer.layout", mGuiButtons);

		MyGUI::Widget* button = mGui.findWidgetT("Mesh");
		button->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::pressEvent);
		button->eventMouseLostFocus = MyGUI::newDelegate(this, &ViewComposerState::lostFocus);
		button->eventMouseSetFocus = MyGUI::newDelegate(this, &ViewComposerState::gotFocus);
		
		button = mGui.findWidget<MyGUI::Button>("Material");
		button->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::pressEvent);
		button->eventMouseLostFocus = MyGUI::newDelegate(this, &ViewComposerState::lostFocus);
		button->eventMouseSetFocus = MyGUI::newDelegate(this, &ViewComposerState::gotFocus);
		
		button = mGui.findWidget<MyGUI::Button>("Skybox");
		button->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::pressEvent);
		button->eventMouseLostFocus = MyGUI::newDelegate(this, &ViewComposerState::lostFocus);
		button->eventMouseSetFocus = MyGUI::newDelegate(this, &ViewComposerState::gotFocus);
		
		button = mGui.findWidget<MyGUI::Button>("Light");
		button->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::pressEvent);
		button->eventMouseLostFocus = MyGUI::newDelegate(this, &ViewComposerState::lostFocus);
		button->eventMouseSetFocus = MyGUI::newDelegate(this, &ViewComposerState::gotFocus);

		button = mGui.findWidget<MyGUI::Button>("SubEntity");
		button->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::pressEvent);
		button->eventMouseLostFocus = MyGUI::newDelegate(this, &ViewComposerState::lostFocus);
		button->eventMouseSetFocus = MyGUI::newDelegate(this, &ViewComposerState::gotFocus);

		button = mGui.findWidget<MyGUI::Button>("TexUnits");
		button->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::pressEvent);
		button->eventMouseLostFocus = MyGUI::newDelegate(this, &ViewComposerState::lostFocus);
		button->eventMouseSetFocus = MyGUI::newDelegate(this, &ViewComposerState::gotFocus);

		button = mGui.findWidget<MyGUI::Button>("Animation");
		button->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::pressEvent);
		button->eventMouseLostFocus = MyGUI::newDelegate(this, &ViewComposerState::lostFocus);
		button->eventMouseSetFocus = MyGUI::newDelegate(this, &ViewComposerState::gotFocus);

		button = mGui.findWidget<MyGUI::Button>("Adapter");
		button->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::pressEvent);
		button->eventMouseLostFocus = MyGUI::newDelegate(this, &ViewComposerState::lostFocus);
		button->eventMouseSetFocus = MyGUI::newDelegate(this, &ViewComposerState::gotFocus);

		button = mGui.findWidget<MyGUI::Button>("Preview");
		button->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::pressEvent);
		button->eventMouseLostFocus = MyGUI::newDelegate(this, &ViewComposerState::lostFocus);
		button->eventMouseSetFocus = MyGUI::newDelegate(this, &ViewComposerState::gotFocus);
	}

	void pressEvent(MyGUI::Widget* w)
	{
		std::string name = w->getName();
		if (name == "Mesh")
		{
			showLoadingMesh(!mShowingMeshLoadingMenu);
		}
		else if (name == "Material")
		{
			showLoadingMaterial(!mShowingMaterialLoadingMenu);
		}
		else if (name == "Skybox")
		{
			showLoadingSky(!mShowingSkyLoadingMenu);
		}
		else if (name == "Light")
		{
			showCreatingLight(!mShowingLightCreatingMenu);
		}
		else if (name == "SubEntity")
		{
			showSubMesh(!mShowingSubMeshMenu);
		}
		else if (name == "TexUnits")
		{
			showTextureUnits(!mShowingTextureUnitMenu);
		}
		else if (name == "Animation")
		{
			showAnimation(!mShowingAnimationMenu);
		}
		else if (name == "Adapter")
		{
			showAdapter(!mShowingAdapterMenu);
		}
		else if (name == "Preview")
		{
			showPreview(!mShowingPreviewMenu);
		}
	}

	bool isMenuShowing()
	{
		return !mLoadedMenu.empty();
	}

	void controllerEventHandler(Controller_::VipEvent* ve)
	{
		switch(ve->getId())
		{
		case A_LOADING_MESH:
		{
			showLoadingMesh(!mShowingMeshLoadingMenu);
			break;
		}
		case A_LOADING_MATERIAL:
		{
			showLoadingMaterial(!mShowingMaterialLoadingMenu);
			break;
		}
		case A_INFO_WINDOW:
		{
			showInfoWindow(!mShowingInfoWindow);
			break;
		}
		case A_CAMERA_AXIS_X:
		{
			onCamX(boost::get<f32>(ve->getData()));
			break;
		}
		case A_CAMERA_MOUSE_X:
		{
			if (mMouseCamPitchYaw)
			{
				onCamX(boost::get<f32>(ve->getData()));
			}
			break;
		}
		case A_CAMERA_AXIS_Y:
		{
			onCamY(boost::get<f32>(ve->getData()));
			break;
		}
		case A_CAMERA_MOUSE_Y:
		{
			if (mMouseCamPitchYaw)
			{
				onCamY(boost::get<f32>(ve->getData()));
			}
			break;
		}
		case A_CAMERA_AXIS_Z:
		{
			onCamZ(boost::get<f32>(ve->getData()));
			break;
		}
		case A_CAMERA_MOUSE_Z:
		{
			if (mMouseCamRoll)
			{
				onCamZ(boost::get<f32>(ve->getData()));
			}
			break;
		}
		case A_CAMERA_MOVE:
		{
			f32 value = boost::get<f32>(ve->getData());
			if (value > EPSILON_F || value < -EPSILON_F)
			{
				mIsZooming = true;
				mDeltaDis += value;
			}
			else
			{
				mIsZooming = false;
				mDeltaDis = 0.0f;
			}
			break;
		}
		case A_CAMERA_MOUSE_MOVE:
		{
			if (isMenuShowing())
				break;

			mIsZooming = false;
			mDeltaDis = boost::get<f32>(ve->getData());
			break;
		}
		case A_CAMERA_RESET:
		{
			onReset();
			break;
		}
		case A_CAMERA_ORBIT:
		{
			mCamOrbit = boost::get<bool>(ve->getData());
			dbglog << "OrbitCamera: " << mCamOrbit;
			break;
		}
		case A_LOADING_SKY:
		{
			showLoadingSky(!mShowingSkyLoadingMenu);
			break;
		}
		case A_CREATE_LIGHT:
		{
			showCreatingLight(!mShowingLightCreatingMenu);
			break;
		}
		case A_DESTROY_LIGHT:
		{
			std::vector<boost::shared_ptr<View::Light> >::iterator it = 
				std::find(mLights.begin(), mLights.end(), mActiveLight);
			if (it != mLights.end())
			{
				mLights.erase(it);
			}
			mActiveLight.reset();
			break;
		}
		case A_PREV_LIGHT:
		{
			std::vector<boost::shared_ptr<View::Light> >::iterator it = 
				std::find(mLights.begin(), mLights.end(), mActiveLight);
			if (it != mLights.end())
			{
				if (it != mLights.begin())
				{
					--it;
					mActiveLight = (*it);
				}
			}
			break;
		}
		case A_NEXT_LIGHT:
		{
			std::vector<boost::shared_ptr<View::Light> >::iterator it = 
				std::find(mLights.begin(), mLights.end(), mActiveLight);
			if (it != mLights.end())
			{
				++it;
				if (it != mLights.end())
				{
					mActiveLight = (*it);
				}
			}
			break;
		}
		case A_FIRST_LIGHT:
		{
			if (!mLights.empty())
			{
				mActiveLight = (*mLights.begin());
			}
			break;
		}
		case A_LAST_LIGHT:
		{
			if (!mLights.empty())
			{
				mActiveLight = (*mLights.rbegin());
			}
			break;
		}
		case A_MOUSE_MIDDLE_PRESSED:
		{
			if (isMenuShowing())
			{
				mMouseCamPitchYaw = false;
				break;
			}

			if ( boost::get<bool>(ve->getData()) )
				mMouseCamPitchYaw = true;
			else
				mMouseCamPitchYaw = false;
			
			break;
		}
		case BFG::ID::A_MOUSE_LEFT_PRESSED:
		{
			bool pressed = boost::get<bool>(ve->getData());
			if (!pressed)
				break;

			if (mMeshName == "")
				break;

			if (!mPositionAdapter)
				break;
			
			if (mMenuFocus)
				break;

			pickPosition();

			break;
		}
		case BFG::ID::A_MOUSE_RIGHT_PRESSED:
		{
			if (isMenuShowing())
			{
				mMouseCamRoll = false;
				break;
			}

			if ( boost::get<bool>(ve->getData()) )
				mMouseCamRoll = true;
			else
				mMouseCamRoll = false;
			
			break;
		}
		case A_SUB_MESH:
		{
			showSubMesh(!mShowingSubMeshMenu);
			break;
		}
		case A_TEX_UNIT:
		{
			showTextureUnits(!mShowingTextureUnitMenu);
			break;
		}
		case A_ANIMATION:
		{
			showAnimation(!mShowingAnimationMenu);
			break;
		}
		case A_ADAPTER:
		{
			showAdapter(!mShowingAdapterMenu);
			break;
		}
		default:
			throw std::logic_error("ViewComposerState::controllerEventHandler: Unknown event received");
		}
	}

	void onCamX(f32 x)
	{
		mDeltaRot.x = M_PI * x;
	}

	void onCamY(f32 y)
	{
		mDeltaRot.y = M_PI * y;
	}

	void onCamZ(f32 z)
	{
		mDeltaRot.z = M_PI * z;
	}

	// universal stuff
	void unloadLoadingMenu()
	{
		if (!mLoadedMenu.empty())
		{
			MyGUI::LayoutManager::getInstance().unloadLayout(mLoadedMenu);
			mLoadedMenu.clear();

			mShowingMeshLoadingMenu = false;
			mShowingMaterialLoadingMenu = false;
			mShowingSkyLoadingMenu = false;
			mShowingLightCreatingMenu = false;
			mShowingSubMeshMenu = false;
			mShowingTextureUnitMenu = false;
			mShowingAnimationMenu = false;
			mShowingAdapterMenu = false;
			mShowingPreviewMenu = false;
			mShowingLoadPreviewMeshMenu = false;
			mShowingLoadPreviewAdapterMenu = false;

			if (mAnimationState)
			{
				mAnimationState->setEnabled(false);
				mAnimationState = NULL;
			}
		}
	}

	void onCancelPressed(MyGUI::Widget* _widget)
	{
		unloadLoadingMenu();
	}

	void fullsizePanel(const std::string& panelName)
	{
		MyGUI::WidgetPtr backgroundPanel = mGui.findWidgetT(panelName);
		MyGUI::IntSize size = mGui.getViewSize();
		backgroundPanel->setSize(size);
	}

	// Mesh stuff
	void showLoadingMesh(bool show)
	{
		unloadLoadingMenu();

		if (show)
		{
			showPredefinedFiles
			(
				mLoadedMenu,
				ID::P_GRAPHICS_MESHES,
			    MyGUI::newDelegate(this, &ViewComposerState::onMeshOk),
			    MyGUI::newDelegate(this, &ViewComposerState::onMeshSelected)
			);
			mShowingMeshLoadingMenu = true;
		}
	}

	void showPredefinedFiles(MyGUI::VectorWidgetPtr& container,
	                         ID::PathType id,
	                         MyGUI::delegates::IDelegate1<MyGUI::Widget*>* okHandler,
	                         MyGUI::delegates::IDelegate2<MyGUI::List*, size_t>* listSelected)
	{
		loadLayout("ChooseListItem.layout", container);

		fullsizePanel("MainMenuPanel");

		MyGUI::List* list = mGui.findWidget<MyGUI::List>("ItemList");
		if (list)
		{
			list->eventListSelectAccept = listSelected;

			BFG::Path path;
			
			mFiles.clear();
			allFilesOfDirectory(mFiles, path.Get(id));

			list->removeAllItems();

			for (size_t l = 0; l < mFiles.size(); ++l)
			{
				list->addItem(mFiles[l]);
			}
		}

		MyGUI::WidgetPtr buttonOk = mGui.findWidgetT("ListOk");
		buttonOk->eventMouseButtonClick = okHandler;

		MyGUI::WidgetPtr buttonCancel = mGui.findWidgetT("ListCancel");
		buttonCancel->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::onCancelPressed);
	}

	void onMeshOk(MyGUI::Widget* w)
	{
		MyGUI::List* list = mGui.findWidget<MyGUI::List>("ItemList");
		size_t index = list->getIndexSelected();
		if (index != MyGUI::ITEM_NONE)
		{
			onMeshSelected(list, index);
		}
	}

	void onMeshSelected(MyGUI::List* list, size_t index)
	{
		useMesh(mFiles[index]);
		showLoadingMesh(false);
	}

	void useMesh(const std::string& meshName)
	{
		mRenderObject.reset();
		mRenderObject.reset(new View::RenderObject
		(
			NULL_HANDLE,
			mObject,
			meshName,
			v3::ZERO,
			qv4::IDENTITY
		));
		mCurrentSettings.mMesh = meshName;
		mMeshName = meshName;

		Ogre::SceneManager* sceneMgr = Ogre::Root::getSingleton().getSceneManager(BFG_SCENEMANAGER);
		Ogre::Entity* ent = sceneMgr->getEntity(stringify(mObject));
		
		onReset();
		mCamDistance = ent->getBoundingRadius() * 1.5f;

		mSelectedSubEntity = ent->getSubEntity(0);

		mMaterialName = mSelectedSubEntity->getMaterialName();
		updateAllInfo();

		clearAdapters();
	}

	// Material stuff
	void showLoadingMaterial(bool show)
	{
		unloadLoadingMenu();

		if (show)
		{
			loadMaterial();
			mShowingMaterialLoadingMenu = true;
		}
	}

	void loadMaterial()
	{
		loadLayout("ChooseListItem.layout", mLoadedMenu);

		fullsizePanel("MainMenuPanel");

		MyGUI::List* list = mGui.findWidget<MyGUI::List>("ItemList");
		if (list)
		{
			list->eventListSelectAccept = MyGUI::newDelegate(this, &ViewComposerState::onMatSelected);

			Ogre::MaterialManager* matMan = Ogre::MaterialManager::getSingletonPtr();

			Ogre::MaterialManager::ResourceMapIterator resIt = matMan->getResourceIterator();

			mFiles.clear();

			list->removeAllItems();

			while(resIt.hasMoreElements())
			{
				Ogre::MaterialPtr mat = matMan->getByHandle(resIt.peekNextKey());
				list->addItem(mat->getName());
				mFiles.push_back(mat->getName());
				resIt.getNext();
			}
		}

		MyGUI::WidgetPtr buttonOk = mGui.findWidgetT("ListOk");
		buttonOk->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::onMatOk);

		MyGUI::WidgetPtr buttonCancel = mGui.findWidgetT("ListCancel");
		buttonCancel->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::onCancelPressed);
	}

	void onMatOk(MyGUI::Widget* w)
	{
		MyGUI::List* list = mGui.findWidget<MyGUI::List>("ItemList");
		size_t index = list->getIndexSelected();

		if (index != MyGUI::ITEM_NONE)
		{
			onMatSelected(list, index);
		}
	}

	void onMatSelected(MyGUI::List* list, size_t index)
	{
		const std::string& matName = list->getItemNameAt(index);
		useMaterial(matName);
		unloadLoadingMenu();
	}

	void useMaterial(const std::string& matName)
	{
		Ogre::SceneManager* sceneMgr = Ogre::Root::getSingleton().getSceneManager(BFG_SCENEMANAGER);

		if (mSelectedSubEntity)
		{
			mSelectedSubEntity->setMaterialName(matName);
		}
		else if (sceneMgr->hasEntity(stringify(mObject)))
		{
			Ogre::Entity* ent = sceneMgr->getEntity(stringify(mObject));

			ent->setMaterialName(matName);
		}

		mMaterialName = matName;
		updateAllInfo();
		mCurrentSettings.mMaterial = matName;
	}

	// Skybox stuff
	void showLoadingSky(bool show)
	{
		unloadLoadingMenu();

		if (show)
		{
			loadSky();
			mShowingSkyLoadingMenu = true;
		}
	}

	void loadSky()
	{
		loadLayout("ChooseListItem.layout", mLoadedMenu);

		fullsizePanel("MainMenuPanel");

		MyGUI::List* list = mGui.findWidget<MyGUI::List>("ItemList");
		if (list)
		{
			list->eventListSelectAccept = MyGUI::newDelegate(this, &ViewComposerState::onSkySelected);

			Ogre::MaterialManager* matMan = Ogre::MaterialManager::getSingletonPtr();

			Ogre::MaterialManager::ResourceMapIterator resIt = matMan->getResourceIterator();

			mFiles.clear();

			list->removeAllItems();

			std::string matName;

			while(resIt.hasMoreElements())
			{
				matName = "";
				Ogre::MaterialPtr mat = matMan->getByHandle(resIt.peekNextKey());
				for (size_t index = 0; index < mat->getNumTechniques(); ++index)
				{
					Ogre::Technique* tech = mat->getTechnique(index);
					for (size_t passIndex = 0; passIndex < tech->getNumPasses(); ++passIndex)
					{
						Ogre::Pass* pass = tech->getPass(passIndex);
						for (size_t texIndex = 0; texIndex < pass->getNumTextureUnitStates(); ++texIndex)
						{
							Ogre::TextureUnitState* tex = pass->getTextureUnitState(texIndex);
							if(tex->isCubic())
								matName = mat->getName();
						}
					}
				}
				if ( (matName != "") && (matName.substr(0, 16) != BFG_SCENEMANAGER) )
				{
					list->addItem(mat->getName());
					mFiles.push_back(mat->getName());
				}
				resIt.getNext();
			}
		}

		MyGUI::WidgetPtr buttonOk = mGui.findWidgetT("ListOk");
		buttonOk->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::onSkyOk);

		MyGUI::WidgetPtr buttonCancel = mGui.findWidgetT("ListCancel");
		buttonCancel->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::onCancelPressed);
	}

	void onSkyOk(MyGUI::Widget* w)
	{
		MyGUI::List* list = mGui.findWidget<MyGUI::List>("ItemList");
		size_t index = list->getIndexSelected();
		if (index != MyGUI::ITEM_NONE)
		{
			onSkySelected(list, index);
		}
	}

	void onSkySelected(MyGUI::List* list, size_t index)
	{
		mSkyName = mFiles[index];
		updateAllInfo();
		useSky(mSkyName);
		unloadLoadingMenu();
	}

	void useSky(const std::string& skyName)
	{
		View::SkyCreation sc(skyName);
		setSky(sc);
		mCurrentSettings.mSkybox = skyName;

		Ogre::TextureUnitState* skyTexUnit = findAliasInMaterial("sky", skyName);

		if (!skyTexUnit)
			return;

		std::vector<Ogre::TextureUnitState*> texVec = findTextureAlias("Environment");

		std::vector<Ogre::TextureUnitState*>::iterator it = texVec.begin();
		for (; it != texVec.end(); ++it)
		{
			Ogre::TextureUnitState* tex = *it;
			
			std::string texName = skyTexUnit->getTextureName();
			std::string extension = texName.substr(texName.find_last_of("."));
			std::string name = texName.substr(0, texName.find("_fr"));

			tex->setCubicTextureName(name + extension, true);
			tex->setColourOperationEx(Ogre::LBX_ADD,
			                          Ogre::LBS_TEXTURE,
			                          Ogre::LBS_CURRENT);
			tex->setColourOpMultipassFallback(Ogre::SBF_ONE,
			                                  Ogre::SBF_ONE);
			tex->setEnvironmentMap(true, Ogre::TextureUnitState::ENV_REFLECTION);
		}
	}

	std::vector<Ogre::TextureUnitState*> findTextureAlias(const std::string alias)
	{
		Ogre::MaterialManager* matMan = Ogre::MaterialManager::getSingletonPtr();
		Ogre::MaterialManager::ResourceMapIterator::iterator resIt =
			matMan->getResourceIterator().begin();

		std::vector<Ogre::TextureUnitState*> result;
		for (; resIt != matMan->getResourceIterator().end(); ++resIt)
		{
			Ogre::MaterialPtr mat = resIt->second;

			Ogre::Material::TechniqueIterator techIt = mat->getTechniqueIterator();
			while(techIt.hasMoreElements())
			{
				Ogre::Technique* tech = techIt.getNext();

				Ogre::Technique::PassIterator passIt = tech->getPassIterator();
				while(passIt.hasMoreElements())
				{
					Ogre::Pass* pass = passIt.getNext();

					Ogre::Pass::TextureUnitStateIterator texIt = pass->getTextureUnitStateIterator();

					while (texIt.hasMoreElements())
					{
						Ogre::TextureUnitState* tex = texIt.getNext();

						if (tex->getTextureNameAlias() == alias)
						{
							result.push_back(tex);
						}
					}
				}
			}
		}
		return result;
	}

	Ogre::TextureUnitState* findAliasInMaterial(const std::string alias, const std::string matName)
	{
		Ogre::MaterialManager* matMan = Ogre::MaterialManager::getSingletonPtr();
		Ogre::MaterialPtr mat = matMan->getByName(matName);

		Ogre::Material::TechniqueIterator techIt = mat->getTechniqueIterator();
		while(techIt.hasMoreElements())
		{
			Ogre::Technique* tech = techIt.getNext();

			Ogre::Technique::PassIterator passIt = tech->getPassIterator();
			while(passIt.hasMoreElements())
			{
				Ogre::Pass* pass = passIt.getNext();

				Ogre::Pass::TextureUnitStateIterator texIt = pass->getTextureUnitStateIterator();

				while (texIt.hasMoreElements())
				{
					Ogre::TextureUnitState* tex = texIt.getNext();

					if (tex->getTextureNameAlias() == alias)
					{
						return tex;
					}
				}
			}
		}
		return NULL;
	}

	// Light stuff
	void showCreatingLight(bool show)
	{
		unloadLoadingMenu();

		if (show)
		{
			loadLight("Light.layout");
			mShowingLightCreatingMenu = true;
		}
	}

	void loadLight(const std::string& fileName)
	{
		loadLayout(fileName, mLoadedMenu);

		fullsizePanel("MainMenuPanel");

		MyGUI::WidgetPtr dCreate = mGui.findWidgetT("dCreate");
		dCreate->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::createDLight);

		MyGUI::WidgetPtr pCreate = mGui.findWidgetT("pCreate");
		pCreate->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::createPLight);

		MyGUI::WidgetPtr sCreate = mGui.findWidgetT("sCreate");
		sCreate->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::createSLight);

		// Ambient
		Ogre::SceneManager* sceneMgr =
			Ogre::Root::getSingletonPtr()->getSceneManager(BFG_SCENEMANAGER);

		MyGUI::WidgetPtr aSet = mGui.findWidgetT("aSet");
		aSet->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::setAmbientLight);

		cv4 ambient = sceneMgr->getAmbientLight();

		cv4ToGui("aDiff", ambient);
	}

	v3 guiEditToV3(const std::string& x, const std::string& y, const std::string& z)
	{
		MyGUI::Edit* wX = mGui.findWidget<MyGUI::Edit>(x);
		MyGUI::Edit* wY = mGui.findWidget<MyGUI::Edit>(y);
		MyGUI::Edit* wZ = mGui.findWidget<MyGUI::Edit>(z);

		v3 ret
		(
			MyGUI::utility::parseValue<f32>(wX->getCaption()),
			MyGUI::utility::parseValue<f32>(wY->getCaption()),
			MyGUI::utility::parseValue<f32>(wZ->getCaption())
		);

		return ret;
	}

	cv4 guiToCv4(const std::string& r, const std::string& g, const std::string& b)
	{
		v3 rgb = guiEditToV3(r, g, b);

		return cv4(rgb.x, rgb.y, rgb.z, 1.0f);
	}

	void qv4ToGui(const std::string widgetPrefix, const qv4& ori)
	{
		const std::string wX = widgetPrefix + "X";
		const std::string wY = widgetPrefix + "Y";
		const std::string wZ = widgetPrefix + "Z";
		const std::string wW = widgetPrefix + "W";

		f32ToGuiEdit(wX, ori.x);
		f32ToGuiEdit(wY, ori.y);
		f32ToGuiEdit(wZ, ori.z);
		f32ToGuiEdit(wW, ori.w);
	}

	void cv4ToGui(const std::string widgetPrefix, const cv4& color)
	{
		const std::string wR = widgetPrefix + "R";
		const std::string wG = widgetPrefix + "G";
		const std::string wB = widgetPrefix + "B";

		f32ToGuiEdit(wR, color.r);
		f32ToGuiEdit(wG, color.g);
		f32ToGuiEdit(wB, color.b);
	}

	f32 guiEditToF32(const std::string& name)
	{
		MyGUI::Edit* w = mGui.findWidget<MyGUI::Edit>(name);

		return MyGUI::utility::parseValue<f32>(w->getCaption());
	}

	void f32ToGuiEdit(const std::string& name, f32 value)
	{
		MyGUI::Edit* w = mGui.findWidget<MyGUI::Edit>(name);

		w->setCaption(MyGUI::utility::toString(value));
	}

	void createDLight(MyGUI::Widget* w)
	{
		View::DirectionalLightCreation dlc
		(
			generateHandle(),
			guiEditToV3("dDirX", "dDirY", "dDirZ"),
			guiToCv4("dDiffR", "dDiffG", "dDiffB"),
			guiToCv4("dSpecR", "dSpecG", "dSpecB"),
			guiEditToF32("dPower")
		);
		mActiveLight.reset(new View::Light(dlc));
		mLights.push_back(mActiveLight);
		unloadLoadingMenu();
	}

	void createPLight(MyGUI::Widget* w)
	{
		View::PointLightCreation plc
		(
			generateHandle(),
			guiEditToV3("pPosX", "pPosY", "pPosZ"),
			guiEditToF32("pRange"),
			guiEditToF32("pConst"),
			guiEditToF32("pLinear"),
			guiEditToF32("pQuad"),
			guiToCv4("pDiffR", "pDiffG", "pDiffB"),
			guiToCv4("pSpecR", "pSpecG", "pSpecB"),
			guiEditToF32("pPower")
		);
		mActiveLight.reset(new View::Light(plc));
		mLights.push_back(mActiveLight);
		unloadLoadingMenu();
	}

	void createSLight(MyGUI::Widget* w)
	{
		View::SpotLightCreation slc
		(
			generateHandle(),
			guiEditToV3("sPosX", "sPosY", "sPosZ"),
			guiEditToV3("sDirX", "sDirY", "sDirZ"),
			guiEditToF32("sRange"),
			guiEditToF32("sConst"),
			guiEditToF32("sLinear"),
			guiEditToF32("sQuad"),
			guiEditToF32("sFalloff"),
			guiEditToF32("sInner"),
			guiEditToF32("sOuter"),
			guiToCv4("sDiffR", "sDiffG", "sDiffB"),
			guiToCv4("sSpecR", "sSpecG", "sSpecB"),
			guiEditToF32("sPower")
		);
		mActiveLight.reset(new View::Light(slc));
		mLights.push_back(mActiveLight);
		unloadLoadingMenu();
	}

	void setAmbientLight(MyGUI::Widget* w)
	{
		Ogre::SceneManager* sceneMgr = 
			Ogre::Root::getSingletonPtr()->getSceneManager(BFG_SCENEMANAGER);

		sceneMgr->setAmbientLight(guiToCv4("aDiffR", "aDiffG", "aDiffB"));
	}

	// Info stuff
	void showInfoWindow(bool show)
	{
		if (show)
		{
			loadInfo();
		}
		else
		{
			MyGUI::LayoutManager::getInstance().unloadLayout(mInfoWindow);
			mInfoWindow.clear();
		}

		mShowingInfoWindow = show;
	}

	void loadInfo()
	{
		loadLayout("infoPanel.layout", mInfoWindow);
		fullsizePanel("MainInfoPanel");

		updateAllInfo();
	}

	void updateAllInfo()
	{
		if (mInfoWindow.empty())
			return;

		MyGUI::StaticText* wSky = mGui.findWidget<MyGUI::StaticText>("infoSky");
		wSky->setCaption(mSkyName);

		MyGUI::StaticText* wLight = mGui.findWidget<MyGUI::StaticText>("infoLight");
		wLight->setCaption(mLightName);

		MyGUI::StaticText* wMesh = mGui.findWidget<MyGUI::StaticText>("infoMesh");
		wMesh->setCaption(mMeshName);

		MyGUI::StaticText* wSubEntity = mGui.findWidget<MyGUI::StaticText>("infoSubEntity");
		wSubEntity->setCaption(mSubEntityName);

		MyGUI::StaticText* wMat = mGui.findWidget<MyGUI::StaticText>("infoMat");
		wMat->setCaption(mMaterialName);
	}

	// submesh stuff
	void showSubMesh(bool show)
	{
		unloadLoadingMenu();

		if (show)
		{
			loadSubMesh();
			mShowingSubMeshMenu = true;
		}
	}

	void loadSubMesh()
	{
		loadLayout("subMesh.layout", mLoadedMenu);

		Ogre::SceneManager* sceneMgr = 
			Ogre::Root::getSingletonPtr()->getSceneManager(BFG_SCENEMANAGER);

		if (sceneMgr->hasEntity(stringify(mObject)))
		{
			MyGUI::Window* window = mGui.findWidget<MyGUI::Window>("subMeshWindow");
			MyGUI::List* list = mGui.findWidget<MyGUI::List>("subMeshList");

			Ogre::Entity* ent = sceneMgr->getEntity(stringify(mObject));

			Ogre::MeshPtr mesh = ent->getMesh();

			std::string captionText = "SubMeshes of " + mesh->getName();
			window->setCaption(captionText);

			const Ogre::Mesh::SubMeshNameMap subMap = mesh->getSubMeshNameMap();
			
			Ogre::Mesh::SubMeshNameMap::const_iterator subIt = subMap.begin();
			for (; subIt != subMap.end(); ++subIt)
			{
				list->addItem(subIt->first);
			}

			list->eventListSelectAccept = MyGUI::newDelegate(this, &ViewComposerState::onSubMeshSelected);
		}
	}

	void onSubMeshSelected(MyGUI::List* list, size_t index)
	{
		std::string subName = list->getItemNameAt(index);

		Ogre::SceneManager* sceneMgr = 
			Ogre::Root::getSingletonPtr()->getSceneManager(BFG_SCENEMANAGER);

		if (sceneMgr->hasEntity(stringify(mObject)))
		{
			Ogre::Entity* ent = sceneMgr->getEntity(stringify(mObject));

			mSelectedSubEntity = ent->getSubEntity(subName);
			mSubEntityName = subName;
			mMaterialName = mSelectedSubEntity->getMaterialName();
			updateAllInfo();
		}
		unloadLoadingMenu();
	}

	void showTextureUnits(bool show)
	{
		unloadLoadingMenu();

		if (show)
		{
			loadTextureUnitMenu();
			mShowingTextureUnitMenu = true;
		}
	}

	void loadTextureUnitMenu()
	{
		loadLayout("texUnit.layout", mLoadedMenu);

		MyGUI::ItemBox* box = mGui.findWidget<MyGUI::ItemBox>("itemBox");

		if (mObject == NULL_HANDLE)
			return;

		if (mMaterialName == "")
			return;

		Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().getByName(mMaterialName);

		Ogre::Technique* tech = mat->getSupportedTechnique(0);
		
		unsigned int height = 0;

		Ogre::Technique::PassIterator passIt = tech->getPassIterator();
		while (passIt.hasMoreElements())
		{
			Ogre::Pass* pass = passIt.getNext();
			
			std::stringstream passName;
			passName << "Pass: " << pass->getName() << " (" << pass->getIndex() << ")";

			MyGUI::StaticText* staticText = box->createWidget<MyGUI::StaticText>("BFE_StaticText", 2, height, 374, 16, MyGUI::Align::Default);
			staticText->setCaption(passName.str());
			staticText->setTextColour(MyGUI::Colour::White);

			height += 16;

			Ogre::Pass::TextureUnitStateIterator texIt = pass->getTextureUnitStateIterator();
			while (texIt.hasMoreElements())
			{
				Ogre::TextureUnitState* tex = texIt.getNext();
				std::string texName = tex->getTextureName();
				f32 texBias = tex->getTextureMipmapBias();

				TextureUnitChange* tuc = findChanges(mMaterialName, tex->getTextureNameAlias());

				MyGUI::Widget* panel = box->createWidgetT("Widget", "Transparent", 0, height, 504, 70, MyGUI::Align::Default);

				MyGUI::StaticText* staticText = panel->createWidget<MyGUI::StaticText>("BFE_StaticText", 2, 4, 374, 16, MyGUI::Align::Default);
				MyGUI::Button* checkButton = panel->createWidget<MyGUI::Button>("CheckBox", 2, 22, 64, 22, MyGUI::Align::Default, "unitEnabled");
				MyGUI::Edit* edit = panel->createWidget<MyGUI::Edit>("BFE_Edit", 68, 22, 310, 22, MyGUI::Align::Default, "texUnitName");
				MyGUI::StaticText* staticTextBias = panel->createWidget<MyGUI::StaticText>("BFE_StaticText", 2, 46, 64, 22, MyGUI::Align::Default);
				MyGUI::Edit* editBias = panel->createWidget<MyGUI::Edit>("BFE_Edit", 68, 46, 64, 22, MyGUI::Align::Default, "texUnitBias");

				checkButton->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::onMapEnabled);
				edit->eventEditSelectAccept = MyGUI::newDelegate(this, &ViewComposerState::onMaterialChanged);
				editBias->eventEditSelectAccept = MyGUI::newDelegate(this, &ViewComposerState::onBiasChanged);

				checkButton->setEnabled(true);
				checkButton->setCaption("on");

				staticTextBias->setCaption("Bias");
				staticTextBias->setTextColour(MyGUI::Colour::White);

				staticText->setCaption(tex->getTextureNameAlias());
				staticText->setTextColour(MyGUI::Colour::White);

				editBias->setUserString("Material", mMaterialName);
				editBias->setUserString("Alias", tex->getTextureNameAlias());
				editBias->setCaption(MyGUI::utility::toString(texBias));

				edit->setUserString("Material", mMaterialName);
				edit->setUserString("Alias", tex->getTextureNameAlias());
				edit->setCaption(texName);


				if (tuc)
				{
					checkButton->setStateCheck(tuc->mEnabled);
					edit->setUserString("SelectedMat", tuc->mTexture);
					edit->setEnabled(tuc->mEnabled);
				}
				else
				{
					checkButton->setStateCheck(true);
					edit->setUserString("SelectedMat", texName);
				}

				if (tex->getTextureNameAlias() == "TextureMap" || 
					tex->getTextureNameAlias() == "aoMap" ||
					tex->getTextureNameAlias() == "SpecularMap")
				{
					edit->setUserString("DefaultMat", "white.png");
				}
				else if (tex->getTextureNameAlias() == "NormalMap")
				{
					edit->setUserString("DefaultMat", "flat_n.png");
				}
				else if (tex->getTextureNameAlias() == "IlluminationMap")
				{
					edit->setUserString("DefaultMat", "black.png");
				}

				height += 70;
			}
		}
	}

	void onBiasChanged(MyGUI::Edit* sender)
	{
		if (mMaterialName.empty())
			return;

		Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().getByName(mMaterialName);

		Ogre::Technique* tech = mat->getSupportedTechnique(0);

		Ogre::Technique::PassIterator passIt = tech->getPassIterator();
		while (passIt.hasMoreElements())
		{
			Ogre::Pass* pass = passIt.getNext();

			Ogre::TextureUnitState* tex = pass->getTextureUnitState(sender->getUserString("Alias"));
			if (tex)
				tex->setTextureMipmapBias(MyGUI::utility::parseValue<f32>(sender->getCaption()));
		}
	}
	
	void onMaterialChanged(MyGUI::Edit* sender)
	{
		if (mMaterialName.empty())
			return;


		Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().getByName(mMaterialName);

		Ogre::Technique* tech = mat->getSupportedTechnique(0);

		Ogre::Technique::PassIterator passIt = tech->getPassIterator();
		while (passIt.hasMoreElements())
		{
			Ogre::Pass* pass = passIt.getNext();

			Ogre::TextureUnitState* tex = pass->getTextureUnitState(sender->getUserString("Alias"));
			if (tex)
				tex->setTextureName(sender->getCaption());
		}
	}

	void onMapEnabled(MyGUI::Widget* sender)
	{
		MyGUI::Button* button = sender->castType<MyGUI::Button>();
		MyGUI::Widget* parent = button->getParent();
		
		MyGUI::Edit* edit = parent->findWidget("texUnitName")->castType<MyGUI::Edit>();

		if(button->getStateCheck())
		{
			TextureUnitChange* tuc = findChanges(edit->getUserString("Material"), edit->getUserString("Alias"));
			if (!tuc)
			{
				tuc = new TextureUnitChange;
				tuc->mMaterial = edit->getUserString("Material");
				tuc->mAlias = edit->getUserString("Alias");
				mTextureChanges.push_back(tuc);
			}
			tuc->mTexture = edit->getCaption();
			tuc->mEnabled = false;

			button->setStateCheck(false);
			edit->setUserString("SelectedMat", edit->getCaption());
			edit->setCaption(edit->getUserString("DefaultMat"));
			edit->setEnabled(false);
		}
		else
		{
			TextureUnitChange* tuc = findChanges(edit->getUserString("Material"), edit->getUserString("Alias"));

			if (tuc)
			{
				edit->setCaption(tuc->mTexture);
				tuc->mEnabled = true;
			}
			else
			{
				edit->setCaption(edit->getUserString("SelectedMat"));
			}
			button->setStateCheck(true);
			edit->setEnabled(true);
		}
		onMaterialChanged(edit);
	}

	TextureUnitChange* findChanges(const std::string& material, const std::string& alias)
	{
		std::vector<TextureUnitChange*>::iterator tucIt = mTextureChanges.begin();

		for (; tucIt != mTextureChanges.end(); ++tucIt)
		{
			TextureUnitChange* tuc = *(tucIt);
			if (tuc->mMaterial == material &&
				tuc->mAlias == alias )
			{
				return tuc;
			}
		}

		return NULL;
	}


	// animation stuff
	void showAnimation(bool show)
	{
		unloadLoadingMenu();

		if (show)
		{
			loadAnimationWindow();
		}
	}

	void loadAnimationWindow()
	{
		if (!mRenderObject)
			return;

		Ogre::SceneManager* sceneMgr =
			Ogre::Root::getSingleton().getSceneManager(BFG_SCENEMANAGER);

		if (sceneMgr == NULL)
			return;

		Ogre::Entity* ent = sceneMgr->getEntity(stringify(mObject));

		if (ent == NULL)
			return;

		Ogre::AnimationStateSet* animStateSet = ent->getAllAnimationStates();

		if (!animStateSet)
		{
			errlog << "No animation found in mesh " << mMeshName;
			return;
		}

		loadLayout("Animation.layout", mLoadedMenu);

		MyGUI::ComboBox* statesBox = mGui.findWidget<MyGUI::ComboBox>("animStates");

		Ogre::AnimationStateIterator animStateIt = animStateSet->getAnimationStateIterator();
		
		while(animStateIt.hasMoreElements())
		{
			Ogre::AnimationState* state = animStateIt.getNext();
			statesBox->addItem(state->getAnimationName());
		}

		statesBox->eventComboChangePosition = MyGUI::newDelegate(this, &ViewComposerState::onAnimationSelected);

		MyGUI::HScroll* slider = mGui.findWidget<MyGUI::HScroll>("animSlider");
		slider->eventScrollChangePosition = MyGUI::newDelegate(this, &ViewComposerState::onChangeSliderPosition);

		MyGUI::Button* playButton = mGui.findWidget<MyGUI::Button>("bPlay");
		playButton->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::onPlayPressed);

		MyGUI::Button* pauseButton = mGui.findWidget<MyGUI::Button>("bPause");
		pauseButton->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::onPausePressed);

		MyGUI::Button* stopButton = mGui.findWidget<MyGUI::Button>("bStop");
		stopButton->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::onStopPressed);

		mShowingAnimationMenu = true;
	}

	void onAnimationSelected(MyGUI::ComboBox* sender, size_t index)
	{
		if (mAnimationState)
		{
			mAnimationState->setEnabled(false);
			mAnimationState->setTimePosition(0.0f);
		}

		std::string stateName = sender->getItemNameAt(index);

		if (stateName.empty())
			return;

		Ogre::SceneManager* sceneMgr =
			Ogre::Root::getSingleton().getSceneManager(BFG_SCENEMANAGER);

		Ogre::Entity* ent = sceneMgr->getEntity(stringify(mObject));

		mAnimationState = ent->getAnimationState(stateName);
		mAnimationState->setEnabled(true);
	}

	void onChangeSliderPosition(MyGUI::VScroll* sender, size_t newPos)
	{
		if (mAnimationState)
		{
			f32 length = mAnimationState->getLength();
			size_t sliderLength = sender->getScrollRange() - 1;

			f32 timePos = length * ((f32)newPos / (f32)sliderLength);
			mAnimationState->setTimePosition(timePos);
		}
	}

	void onPlayPressed(MyGUI::Widget* sender)
	{
		if (mAnimationState)
		{
			mAnimationLoop = true;
		}
	}

	void onPausePressed(MyGUI::Widget* sender)
	{
		if (mAnimationState)
		{
			mAnimationLoop = false;
		}
	}

	void onStopPressed(MyGUI::Widget* sender)
	{
		if (mAnimationState)
		{
			mAnimationLoop = false;
			mAnimationState->setTimePosition(0.0f);
			MyGUI::HScroll* slider = mGui.findWidget<MyGUI::HScroll>("animSlider");
			slider->setScrollPosition(0);
		}
	}

	void pickPosition()
	{
		MyGUI::InputManager* inputMan = MyGUI::InputManager::getInstancePtr();
		Ogre::SceneManager* sceneMan = Ogre::Root::getSingleton().getSceneManager(BFG_SCENEMANAGER);

		// Get mouse position
		const MyGUI::IntPoint& mousePos = inputMan->getMousePosition();
		MyGUI::IntSize size = mGui.getViewSize();
		f32 x = (f32)mousePos.left / (f32)size.width;
		f32 y = (f32)mousePos.top / (f32)size.height;

		// Get ray from camera
		Ogre::Camera* cam = sceneMan->getCamera(stringify(mCamHandle));
		Ogre::Ray mouseRay = cam->getCameraToViewportRay(x, y);

		BFG::Mesh* mesh = new BFG::Mesh(BFG::View::loadMesh(mMeshName));

		// Test if ray intersects with polygons
		f32 distance = std::numeric_limits<float>::max();
		Ogre::Vector3 normal;
		u32 lastIndex = mesh->mIndexCount - 2;
		for (u32 i = 0; i < lastIndex; ++i)
		{
			u32 index1 = mesh->mIndices[i];
			u32 index2 = mesh->mIndices[i+1];
			u32 index3 = mesh->mIndices[i+2];

			std::pair<bool, Ogre::Real> result = 
				Ogre::Math::intersects
				(
					mouseRay,
					BFG::View::toOgre(mesh->mVertices[index1]),
					BFG::View::toOgre(mesh->mVertices[index2]),
					BFG::View::toOgre(mesh->mVertices[index3]),
					true,
					false
				);

			if (result.first)
			{
				if (result.second < distance)
				{
					distance = result.second;

					Ogre::Plane p
					(
						BFG::View::toOgre(mesh->mVertices[index1]),
						BFG::View::toOgre(mesh->mVertices[index2]),
						BFG::View::toOgre(mesh->mVertices[index3])
					);

					normal = p.normal;
				}
			}
		}

		// only select polygons in front of the camera (no hits = max float value)
		if (distance > EPSILON_F && distance < std::numeric_limits<float>::max())
		{
			Ogre::Vector3 pos(mouseRay.getPoint(distance));

			Ogre::Quaternion rot1 = Ogre::Vector3::UNIT_Y.getRotationTo(Ogre::Vector3(normal.x, normal.y, 0.0f), Ogre::Vector3::UNIT_Z);

			Ogre::Quaternion rot2 = rot1.yAxis().getRotationTo(normal, rot1.xAxis());
			Ogre::Quaternion rot = rot2 * rot1;

			showMarker(true, pos, rot);
		}
		else
		{
			showMarker(false);
		}
	}

	void showMarker(bool show,
	                const Ogre::Vector3& position = Ogre::Vector3::ZERO,
	                const Ogre::Quaternion& orientation = Ogre::Quaternion::IDENTITY)
	{
		mPickingNode->setVisible(show);
		mPickingNode->setPosition(position);
		mPickingNode->setOrientation(orientation);
	}

	void showAdapter(bool show)
	{
		unloadLoadingMenu();

		if (show)
		{
			loadAdapterWindow();
		}
	}

	void loadAdapterWindow()
	{
		if (!mRenderObject)
			return;

		Ogre::SceneManager* sceneMgr =
			Ogre::Root::getSingleton().getSceneManager(BFG_SCENEMANAGER);

		if (sceneMgr == NULL)
			return;

		loadLayout("Adapter.layout", mLoadedMenu);

		MyGUI::ComboBox* nameBox = mGui.findWidget<MyGUI::ComboBox>("name");
		nameBox->eventComboChangePosition = MyGUI::newDelegate(this, &ViewComposerState::onAdapterNameChanged);
		
		AdapterMapT::iterator mapIt = mAdapters.begin();
		for(; mapIt != mAdapters.end(); ++mapIt)
		{
			nameBox->addItem(mapIt->first);
		}
		nameBox->setIndexSelected(nameBox->findItemIndexWith(mSelectedAdapterGroup));
		nameBox->beginToItemSelected();

		MyGUI::Button* newGroupButton = mGui.findWidget<MyGUI::Button>("newGroup");
		newGroupButton->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::onNewGroupClicked);

		// new button
		MyGUI::Button* newButton = mGui.findWidget<MyGUI::Button>("newAdapter");
		newButton->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::onNewAdapter);

		// Adapter selection comboBox
		MyGUI::ComboBox* adapterBox = mGui.findWidget<MyGUI::ComboBox>("adapterSelect");

		std::vector<BFG::Adapter>& adapterVec = mAdapters[mSelectedAdapterGroup];
		for (size_t i = 0; i < adapterVec.size(); ++i)
		{
			adapterBox->addItem(MyGUI::utility::toString(i + 1));
		}
		adapterBox->eventComboChangePosition = MyGUI::newDelegate(this, &ViewComposerState::onAdapterSelected);

		// Adapter Position
		MyGUI::Edit* positionEdit = mGui.findWidget<MyGUI::Edit>("position");
		positionEdit->eventEditSelectAccept = MyGUI::newDelegate(this, &ViewComposerState::onPositionChanged);

		// Select adapter position via mouse click
		MyGUI::Button* mousePos = mGui.findWidget<MyGUI::Button>("mousePos");
		mousePos->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::onMousePos);

		MyGUI::Edit* pitchEdit = mGui.findWidget<MyGUI::Edit>("pitch");
		pitchEdit->eventEditSelectAccept = MyGUI::newDelegate(this, &ViewComposerState::onPitchChange);

		MyGUI::Edit* yawEdit = mGui.findWidget<MyGUI::Edit>("yaw");
		yawEdit->eventEditSelectAccept = MyGUI::newDelegate(this, &ViewComposerState::onYawChange);

		MyGUI::Edit* rollEdit = mGui.findWidget<MyGUI::Edit>("roll");
		rollEdit->eventEditSelectAccept = MyGUI::newDelegate(this, &ViewComposerState::onRollChange);

		MyGUI::Button* saveAsButton = mGui.findWidget<MyGUI::Button>("saveAs");
		saveAsButton->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::onSaveAsClicked);

		MyGUI::Button* loadButton = mGui.findWidget<MyGUI::Button>("load");
		loadButton->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::onLoadClicked);

		MyGUI::Button* appendButton = mGui.findWidget<MyGUI::Button>("append");
		appendButton->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::onAppendClicked);

		MyGUI::Button* clearButton = mGui.findWidget<MyGUI::Button>("clear");
		clearButton->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::onClearClicked);

		mShowingAdapterMenu = true;

		if (mSelectedAdapterGroup == "")
		{
			enableFields(false);
		}

		if (mPositionAdapter)
		{
			size_t itemIndex = nameBox->findItemIndexWith(mSelectedAdapterGroup);
			if (itemIndex == MyGUI::ITEM_NONE)
			{
				mSelectedAdapterGroup = "";
				itemIndex = 0;
			}
			nameBox->setIndexSelected(itemIndex);
			nameBox->beginToItemSelected();

			std::vector<BFG::Adapter>& adapterVec = mAdapters[mSelectedAdapterGroup];

			BFG::Adapter& adapter = adapterVec.at(mAdapterSelected - 1);
			adapter.mParentPosition = BFG::View::toBFG(mPickingNode->getPosition());
			adapter.mParentOrientation = BFG::View::toBFG(mPickingNode->getOrientation());

			adapterBox->setIndexSelected(mAdapterSelected - 1);
			onAdapterSelected(adapterBox, mAdapterSelected - 1);

			mPositionAdapter = false;
		}

	}

	void enableFields(bool enable)
	{
		std::vector<std::string> fieldNames;
		fieldNames.push_back("newAdapter");
		fieldNames.push_back("adapterSelect");
		fieldNames.push_back("position");
		fieldNames.push_back("qX");
		fieldNames.push_back("qY");
		fieldNames.push_back("qZ");
		fieldNames.push_back("qW");
		fieldNames.push_back("mousePos");
		fieldNames.push_back("pitch");
		fieldNames.push_back("yaw");
		fieldNames.push_back("roll");

		std::vector<std::string>::iterator it = fieldNames.begin();
		for (; it != fieldNames.end(); ++it)
		{
			MyGUI::Widget* w = mGui.findWidgetT(*it);
			w->setEnabled(enable);
		}
	}

	void onNewGroupClicked(MyGUI::Widget* sender)
	{
		MyGUI::Edit* newGroupEdit = mGui.findWidget<MyGUI::Edit>("newGroupEdit");

		std::string name = newGroupEdit->getCaption();

		if (name == "")
			return;

		MyGUI::ComboBox* groupCombo = mGui.findWidget<MyGUI::ComboBox>("name");

		size_t index = groupCombo->findItemIndexWith(name);
		if (index == MyGUI::ITEM_NONE)
		{
			groupCombo->addItem(name);
			index = groupCombo->findItemIndexWith(name);
			mAdapters[name] = std::vector<BFG::Adapter>();
			mSelectedAdapterGroup = name;
		}
		
		onAdapterNameChanged(groupCombo, index);
	}

	void onPitchChange(MyGUI::Edit* sender)
	{
		f32 pitchVal = MyGUI::utility::parseFloat(sender->getCaption());
		mPickingNode->pitch(Ogre::Degree(pitchVal));

		updateGuiOri();
	}
	
	void onYawChange(MyGUI::Edit* sender)
	{
		f32 yawVal = MyGUI::utility::parseFloat(sender->getCaption());
		mPickingNode->yaw(Ogre::Degree(yawVal));

		updateGuiOri();
	}
	
	void onRollChange(MyGUI::Edit* sender)
	{
		f32 rollVal = MyGUI::utility::parseFloat(sender->getCaption());
		mPickingNode->roll(Ogre::Degree(rollVal));

		updateGuiOri();
	}

	void updateGuiOri()
	{
		std::vector<BFG::Adapter>& adapterVec = mAdapters[mSelectedAdapterGroup];

		BFG::Adapter& adapter = adapterVec.at(mAdapterSelected - 1);
		adapter.mParentOrientation = BFG::View::toBFG(mPickingNode->getOrientation());

		qv4ToGui("q", adapter.mParentOrientation);
	}

	void onSaveAsClicked(MyGUI::Widget* sender)
	{
		fileDialog.setDialogInfo("Save Adapter", "Save", MyGUI::newDelegate(this, &ViewComposerState::saveAdapter));
		fileDialog.setVisible(true);
	}

	void onLoadClicked(MyGUI::Widget* sender)
	{
		fileDialog.setDialogInfo("Load Adapter", "Load", MyGUI::newDelegate(this, &ViewComposerState::loadAdapterClicked));
		fileDialog.setVisible(true);
	}

	void onAppendClicked(MyGUI::Widget* sender)
	{
		fileDialog.setDialogInfo("Append Adapter to", "Append", MyGUI::newDelegate(this, &ViewComposerState::appendAdapter));
		fileDialog.setVisible(true);
	}



	// Adapter stuff
	void saveAdapter(MyGUI::Widget* sender)
	{
		TiXmlDocument document;
		TiXmlDeclaration* declaration = new TiXmlDeclaration("1.0", "utf-8", "" );
		document.LinkEndChild(declaration);

		TiXmlElement* adapterConfigs = new TiXmlElement("AdapterConfigs");
		document.LinkEndChild(adapterConfigs);

		AdapterMapT::iterator mapIt = mAdapters.begin();
		for (; mapIt != mAdapters.end(); ++mapIt)
		{
			TiXmlElement* adapters = new TiXmlElement("AdapterConfig");
			adapters->SetAttribute("name", mapIt->first);
			adapterConfigs->LinkEndChild(adapters);

			std::vector<BFG::Adapter>& adapterVec = mapIt->second;
			std::vector<BFG::Adapter>::iterator it = adapterVec.begin();
			for (; it != adapterVec.end(); ++it)
			{
				v3 pos = (*it).mParentPosition;
				TiXmlElement* adapter = new TiXmlElement("Adapter");
				adapter->SetAttribute
				(
					"id",
					(*it).mIdentifier
				);

				adapter->SetAttribute
				(
					"position",
					MyGUI::utility::toString
					(
						pos.x, ", ",
						pos.y, ", ",
						pos.z
					)
				);

				qv4 ori = (*it).mParentOrientation;
				adapter->SetAttribute
				(
					"orientation",
					MyGUI::utility::toString
					(
						ori.w, ", ",
						ori.x, ", ",
						ori.y, ", ",
						ori.z
					)
				);

				adapters->LinkEndChild(adapter);
			}

		}

		document.SaveFile(fileDialog.getFileName());
		fileDialog.setVisible(false);
	}

	void loadAdapterClicked(MyGUI::Widget* sender)
	{
		std::string filename = fileDialog.getFileName();

		mAdapters.clear();
		mAdapterSelected = 0;

		loadAdapter(filename, mAdapters);

		fileDialog.setVisible(false);
		showAdapter(false);
		showAdapter(true);
	}

	void loadAdapter(const std::string& filename, AdapterMapT& adapterMap)
	{
		TiXmlDocument doc(filename);

		if (!doc.LoadFile())
		{
			throw std::runtime_error("Could not load " + filename);
		}

		TiXmlElement* adapterConfigs = doc.FirstChildElement("AdapterConfigs");
		TiXmlElement* adapterConfig = adapterConfigs->FirstChildElement("AdapterConfig");
		for (; adapterConfig != NULL; adapterConfig = adapterConfig->NextSiblingElement("AdapterConfig"))
		{
			std::string name(adapterConfig->Attribute("name"));
			std::vector<BFG::Adapter> adapterVector;

			TiXmlElement* adapter = adapterConfig->FirstChildElement("Adapter");
			for (; adapter != NULL; adapter = adapter->NextSiblingElement("Adapter"))
			{
				BFG::Adapter a;
				a.mIdentifier = MyGUI::utility::parseUInt(adapter->Attribute("id"));
				BFG::stringToVector3(adapter->Attribute("position"), a.mParentPosition);
				BFG::stringToQuaternion4(adapter->Attribute("orientation"), a.mParentOrientation);
				adapterVector.push_back(a);
			}
			adapterMap[name] = adapterVector;
		}
	}

	void appendAdapter(MyGUI::Widget* sender)
	{
		std::string filename = fileDialog.getFileName();
		TiXmlDocument doc(filename);

		if (!doc.LoadFile())
		{
			throw std::runtime_error("Could not open " + filename);
		}

		TiXmlElement* adapterConfigs = doc.FirstChildElement("AdapterConfigs");

		if (!adapterConfigs)
		{
			throw std::runtime_error("This is not an adapter file!");
		}

		AdapterMapT::iterator mapIt = mAdapters.begin();
		for (; mapIt != mAdapters.end(); ++mapIt)
		{
			bool nameFound = false;
			int lastId = 0;

			TiXmlElement* adapterConfig = adapterConfigs->FirstChildElement("AdapterConfig");
			for (; adapterConfig != NULL; adapterConfig = adapterConfig->NextSiblingElement("AdapterConfig"))
			{
				std::string name(adapterConfig->Attribute("name"));
				if (name == mapIt->first)
				{
					nameFound = true;
					TiXmlElement* adapter = adapterConfig->FirstChildElement("Adapter");
					do 
					{
						adapter->Attribute("id", &lastId);
						adapter = adapter->NextSiblingElement("Adapter");
					} while (adapter);
					break;
				}
			}

			if (!nameFound)
			{
				adapterConfig = new TiXmlElement("AdapterConfig");
				adapterConfig->SetAttribute("name", mapIt->first);
				adapterConfigs->LinkEndChild(adapterConfig);
			}

			std::vector<BFG::Adapter>& adapterVec = mapIt->second;
			std::vector<BFG::Adapter>::iterator it = adapterVec.begin();
			for (; it != adapterVec.end(); ++it)
			{
				v3 pos = (*it).mParentPosition;
				TiXmlElement* adapter = new TiXmlElement("Adapter");
				adapter->SetAttribute
				(
					"id",
					(*it).mIdentifier + lastId
				);
				
				adapter->SetAttribute
				(
					"position",
					MyGUI::utility::toString
					(
						pos.x, ", ",
						pos.y, ", ",
						pos.z
					)
				);
				
				qv4 ori = (*it).mParentOrientation;
				adapter->SetAttribute
				(
					"orientation",
					MyGUI::utility::toString
					(
						ori.w, ", ",
						ori.x, ", ",
						ori.y, ", ",
						ori.z
					)
				);

				adapterConfig->LinkEndChild(adapter);
			}
		}
		doc.SaveFile(filename);
		fileDialog.setVisible(false);
	}

	void onClearClicked(MyGUI::Widget* sender)
	{
		mAdapters.clear();
		mAdapterSelected = 0;
		mSelectedAdapterGroup = "";
		showAdapter(false);
		showAdapter(true);
	}

	void onAdapterNameChanged(MyGUI::ComboBox* list, size_t index)
	{
		if (index == MyGUI::ITEM_NONE)
		{
			enableFields(false);
			return;
		}
		mSelectedAdapterGroup = list->getItemNameAt(index);
		list->setIndexSelected(index);
		list->beginToItemSelected();
		
		enableFields(true);

		refreshAdapter();
	}

	void onNewAdapter(MyGUI::Widget* sender)
	{
		std::vector<BFG::Adapter>& adapterVec = mAdapters[mSelectedAdapterGroup];

		size_t index = adapterVec.size();
		BFG::Adapter adapter;
		adapter.mIdentifier = index + 1;
		adapterVec.push_back(adapter);

		refreshAdapter();

		MyGUI::ComboBox* adapterBox = mGui.findWidget<MyGUI::ComboBox>("adapterSelect");

		adapterBox->setIndexSelected(index);
		onAdapterSelected(adapterBox, index);
	}

	void refreshAdapter()
	{
		std::vector<BFG::Adapter>& adapterVec = mAdapters[mSelectedAdapterGroup];

		MyGUI::ComboBox* adapterBox = mGui.findWidget<MyGUI::ComboBox>("adapterSelect");
		adapterBox->removeAllItems();
		adapterBox->setCaption("");

		std::vector<BFG::Adapter>::iterator it = adapterVec.begin();
		for (; it != adapterVec.end(); ++it)
		{
			Adapter& adapter = *it;
			adapterBox->addItem(MyGUI::utility::toString(adapter.mIdentifier));
		}
		refreshAdapterFields();
	}

	void onAdapterSelected(MyGUI::ComboBox* sender, size_t index)
	{
		std::vector<BFG::Adapter>& adapterVec = mAdapters[mSelectedAdapterGroup];

		BFG::Adapter& adapter = adapterVec.at(index);

		MyGUI::Edit* positionEdit = mGui.findWidget<MyGUI::Edit>("position");
		positionEdit->setCaption(MyGUI::utility::toString(adapter.mParentPosition.x, " ",
		                                                  adapter.mParentPosition.y, " ",
														  adapter.mParentPosition.z));

		qv4ToGui("q", adapter.mParentOrientation);

		mAdapterSelected = index + 1;

		showMarker(true, BFG::View::toOgre(adapter.mParentPosition), BFG::View::toOgre(adapter.mParentOrientation));
	}

	void refreshAdapterFields()
	{
		clearEdit("position");
		clearEdit("qX");
		clearEdit("qY");
		clearEdit("qZ");
		clearEdit("qW");
	}

	void clearEdit(const std::string name)
	{
		MyGUI::Edit* w = mGui.findWidget<MyGUI::Edit>(name);
		w->setCaption("");
	}

	void onPositionChanged(MyGUI::Edit* sender)
	{
		if (!mAdapterSelected)
			return;

		std::vector<BFG::Adapter>& adapterVec = mAdapters[mSelectedAdapterGroup];

		BFG::Adapter& adapter = adapterVec.at(mAdapterSelected-1);
		adapter.mParentPosition = MyGUI::utility::parseValueEx3<v3, f32>(sender->getCaption());

		showMarker(true, BFG::View::toOgre(adapter.mParentPosition), BFG::View::toOgre(adapter.mParentOrientation));
	}

	void onMousePos(MyGUI::Widget* sender)
	{
		if (!mAdapterSelected)
			return;

		mPositionAdapter = true;
		unloadLoadingMenu();
	}

	void clearAdapters()
	{
		showMarker(false);
		mAdapters.clear();
		mAdapterSelected = 0;
		mPositionAdapter = false;
	}

	void showPreview(bool show)
	{
		unloadLoadingMenu();

		if (show)
		{
			loadPreviewMenu();
			mShowingPreviewMenu = true;
		}
	}

	void loadPreviewMenu()
	{
		Ogre::SceneManager* sceneMgr =
			Ogre::Root::getSingleton().getSceneManager(BFG_SCENEMANAGER);

		if (sceneMgr == NULL)
			return;

		loadLayout("Preview.layout", mLoadedMenu);

		MyGUI::Button* loadMeshButton = mGui.findWidget<MyGUI::Button>("loadMesh");
		loadMeshButton->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::onLoadPreviewMesh);

		MyGUI::Button* loadAdapterButton = mGui.findWidget<MyGUI::Button>("loadAdapter");
		loadAdapterButton->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::onLoadPreviewAdapter);

		mGroupName = mGui.findWidget<MyGUI::ComboBox>("groupName");
		mGroupName->eventComboChangePosition = MyGUI::newDelegate(this, &ViewComposerState::onGroupNameSelected);

		mParentAdapter = mGui.findWidget<MyGUI::ComboBox>("parent");
		mParentAdapter->eventComboChangePosition = MyGUI::newDelegate(this, &ViewComposerState::onParentSelected);

		std::vector<BFG::Adapter>& adapterVec = mAdapters[mSelectedAdapterGroup];
		std::vector<BFG::Adapter>::iterator it = adapterVec.begin();
		for (; it != adapterVec.end(); ++it)
		{
			BFG::Adapter& adapter = *it;
			mParentAdapter->addItem(MyGUI::utility::toString(adapter.mIdentifier));
		}

		mChildAdapter = mGui.findWidget<MyGUI::ComboBox>("child");
		mChildAdapter->eventComboChangePosition = MyGUI::newDelegate(this, &ViewComposerState::onChildSelected);

		MyGUI::Button* setPreviewButton = mGui.findWidget<MyGUI::Button>("setPreview");
		setPreviewButton->eventMouseButtonClick = MyGUI::newDelegate(this, &ViewComposerState::onSetPreview);

		mShowingPreviewMenu = true;
	}

	void onLoadPreviewMesh(MyGUI::Widget* sender)
	{
		showPredefinedFiles
		(
			mPreviewMeshMenu,
			ID::P_GRAPHICS_MESHES,
			MyGUI::newDelegate(this, &ViewComposerState::onLoadPreviewMeshOk),
			MyGUI::newDelegate(this, &ViewComposerState::onLoadPreviewMeshSelected)
		);
	}

	void onLoadPreviewMeshOk(MyGUI::Widget* sender)
	{
		MyGUI::List* list = mGui.findWidget<MyGUI::List>("ItemList");
		size_t index = list->getIndexSelected();
		if (index != MyGUI::ITEM_NONE)
		{
			onLoadPreviewMeshSelected(list, index);
		}
	}

	void onLoadPreviewMeshSelected(MyGUI::List* list, size_t index)
	{
		mPreviewMeshName = list->getItemNameAt(index);

		if (!mPreviewMeshMenu.empty())
		{
			MyGUI::LayoutManager::getInstance().unloadLayout(mPreviewMeshMenu);
			mPreviewMeshMenu.clear();
		}
	}

	void onLoadPreviewAdapter(MyGUI::Widget* sender)
	{
		fileDialog.setDialogInfo
		(
			"Load Preview Adapter",
			"Load",
			MyGUI::newDelegate(this, &ViewComposerState::onLoadPreviewAdapterSelected)
		);
		fileDialog.setVisible(true);
	}

	void onLoadPreviewAdapterSelected(MyGUI::Widget* sender)
	{
		std::string fileName = fileDialog.getFileName();

		mPreviewAdapter.clear();
		loadAdapter(fileName, mPreviewAdapter);

		mGroupName->removeAllItems();

		AdapterMapT::iterator groupIt = mPreviewAdapter.begin();
		for (; groupIt != mPreviewAdapter.end(); ++groupIt)
		{
			mGroupName->addItem(groupIt->first);
		}

		fileDialog.setVisible(false);
	}

	void onSetPreview(MyGUI::Widget* sender)
	{
		destroyGameObject();
		createGameObject();

		mPreviewRenderObject.reset();
		mPreviewRenderObject.reset(new View::RenderObject
		(
			mObject,
			mPreviewObject,
			mPreviewMeshName,
			v3::ZERO,
			qv4::IDENTITY
		));

		boost::shared_ptr<BFG::Module> module;
		module.reset(new BFG::Module(mPreviewObject));

		std::vector<BFG::Adapter>& adapterVec = mPreviewAdapter[mSelectedPreviewAdapterGroup];

		mMainObject->attachModule(module, adapterVec, mSelectedPreviewAdapter, mObject, mAdapterSelected);
	}

	void onGroupNameSelected(MyGUI::ComboBox* sender, size_t index)
	{
		mSelectedPreviewAdapterGroup = sender->getItemNameAt(index);

		std::vector<BFG::Adapter>& adapterVec = mPreviewAdapter[mSelectedPreviewAdapterGroup];

		MyGUI::ComboBox* childAdapter = mGui.findWidget<MyGUI::ComboBox>("child");

		std::vector<BFG::Adapter>::iterator it = adapterVec.begin();
		for (; it != adapterVec.end(); ++it)
		{
			BFG::Adapter& adapter = *it;
			childAdapter->addItem(MyGUI::utility::toString(adapter.mIdentifier));
		}
	}

	void onParentSelected(MyGUI::ComboBox* sender, size_t index)
	{
	}
	
	void onChildSelected(MyGUI::ComboBox* sender, size_t index)
	{
		mSelectedPreviewAdapter = MyGUI::utility::parseUInt(sender->getItemNameAt(index));
	}












	void createGameObject()
	{
			// Create Property Plugin (Space)
		BFG::PluginId spId = BFG::ValueId::ENGINE_PLUGIN_ID;
		boost::shared_ptr<BFG::SpacePlugin> sp(new BFG::SpacePlugin(spId));

		BFG::Property::PluginMapT plugMap;
		plugMap.insert(sp);
		mMainObject = new BFG::GameObject(mLoop, mObject, "Test", plugMap, mEnvironment);
		boost::shared_ptr<BFG::Module> module;
		module.reset(new BFG::Module(mObject));

		std::vector<BFG::Adapter>& adapterVec = mAdapters[mSelectedAdapterGroup];

		mMainObject->attachModule(module, adapterVec, 0, NULL_HANDLE, 0);
	}

	void destroyGameObject()
	{
		if (mMainObject)
			delete mMainObject;
	}

	void addPreviewModule(const GameHandle moduleHandle,
	                      const std::vector<BFG::Adapter>& adapterList,
						  const u32 childAdapterID,
						  const u32 parentAdapterID)
	{
		boost::shared_ptr<BFG::Module> module;
		module.reset(new BFG::Module(moduleHandle));

		mMainObject->attachModule(module, adapterList, childAdapterID, mObject, parentAdapterID);

		saveConnection(moduleHandle, childAdapterID, parentAdapterID);
	}

	void saveConnection(const GameHandle moduleHandle, const u32 adapterID, const u32 parentAdapterID)
	{
		std::map<GameHandle, ConnectionData*>::iterator conIt = mPreviewConnections.find(moduleHandle);
		if (conIt != mPreviewConnections.end())
		{
			ConnectionData* connection = conIt->second;
			connection->mAdapterID = adapterID;
			connection->mParentAdapterID = parentAdapterID;
		}
	}

	void removePreviewModule(const GameHandle moduleHandle)
	{
		mMainObject->detachModule(moduleHandle);
	}

	void updatePreview()
	{
		//std::vector<GameHandle>::iterator previewIt = mPreviewObjects.begin();
		//for(; previewIt != mPreviewObjects.end(); ++previewIt)
		//{
		//	removePreviewModule(*previewIt);
		//	// todo attachModule(parameter...)
		//}
	}









	void onReset()
	{
		mCameraRotation->setOrientation(Ogre::Quaternion::IDENTITY);
	}

	void viewEventHandler(View::Event* VE)
	{
		dbglog << VE->getId();
	}

	bool frameStarted(const Ogre::FrameEvent& evt)
	{
		return true;
	}

	bool frameRenderingQueued(const Ogre::FrameEvent& evt)
	{
		if (mCamOrbit)
		{
			mCameraRotation->rotate
			(
				Ogre::Vector3::UNIT_Y,
				Ogre::Radian(mDeltaRot.y) * evt.timeSinceLastFrame,
				Ogre::Node::TS_WORLD
			);

			mCameraRotation->rotate
			(
				Ogre::Vector3::UNIT_X,
				Ogre::Radian(mDeltaRot.x) * evt.timeSinceLastFrame,
				Ogre::Node::TS_WORLD
			);
		}
		else
		{
			mCameraRotation->yaw(Ogre::Radian(mDeltaRot.y) * evt.timeSinceLastFrame);
			mCameraRotation->pitch(Ogre::Radian(mDeltaRot.x) * evt.timeSinceLastFrame);
			mCameraRotation->roll(Ogre::Radian(mDeltaRot.z) * evt.timeSinceLastFrame);
		}

		mCamDistance += mDeltaDis * evt.timeSinceLastFrame;
		if (mCamDistance <= 1.0f)
			mCamDistance = 1.0f;

		mCameraDistance->setPosition(0.0f, 0.0f, -mCamDistance);

		mDeltaRot = v3::ZERO;

		if (!mIsZooming)
		{
			mDeltaDis = 0.0f;
		}
		
		// animation
		if (mAnimationState)
		{
			if (mAnimationLoop)
			{
				MyGUI::HScroll* animSlider = mGui.findWidget<MyGUI::HScroll>("animSlider");

				mAnimationState->addTime(evt.timeSinceLastEvent);

				f32 pos = mAnimationState->getTimePosition();
				f32 length = mAnimationState->getLength();
				size_t sliderLength = animSlider->getScrollRange();

				size_t sliderPos = (sliderLength - 1) * (pos / length);

				animSlider->setScrollPosition(sliderPos);
			}
		}
		return true;
	}

	bool frameEnded(const Ogre::FrameEvent& evt)
	{
		return true;
	}

private:
	void registerEventHandler()
	{
	}
	void unregisterEventHandler()
	{
	}
	
	virtual void pause(){ registerEventHandler(); }
	virtual void resume(){ unregisterEventHandler(); }

	BFG::GameObject* mMainObject;

	MyGUI::Gui& mGui;
	MyGUI::VectorWidgetPtr mLoadedMenu;
	MyGUI::VectorWidgetPtr mInfoWindow;
	MyGUI::VectorWidgetPtr mGuiButtons;

	std::vector<std::string> mFiles;
	GameHandle mObject;
	GameHandle mCamHandle;
	boost::shared_ptr<View::RenderObject> mRenderObject;
	BFG::View::ControllerMyGuiAdapter mControllerAdapter;
	std::vector<boost::shared_ptr<View::Light> > mLights;
	boost::shared_ptr<View::Light> mActiveLight;

	Ogre::SceneNode* mCameraPosition;
	Ogre::SceneNode* mCameraRotation;
	Ogre::SceneNode* mCameraDistance;
	Ogre::SceneNode* mPickingNode;

	Ogre::SubEntity* mSelectedSubEntity;

	v3 mDeltaRot;
	f32 mDeltaDis;
	f32 mCamDistance;
	bool mCamOrbit;
	bool mInfoChanged;
	bool mMouseCamPitchYaw;
	bool mMouseCamRoll;
	bool mIsZooming;

	bool mShowingMeshLoadingMenu;
	bool mShowingMaterialLoadingMenu;
	bool mShowingSkyLoadingMenu;
	bool mShowingLightCreatingMenu;
	bool mShowingInfoWindow;
	bool mShowingSubMeshMenu;
	bool mShowingTextureUnitMenu;
	bool mShowingAnimationMenu;
	bool mShowingAdapterMenu;
	bool mShowingPreviewMenu;
	bool mShowingLoadPreviewMeshMenu;
	bool mShowingLoadPreviewAdapterMenu;

	std::string mSkyName;
	std::string mLightName;
	std::string mMeshName;
	std::string mSubEntityName;
	std::string mMaterialName;
	std::string mAdapterGroupName;

	Settings mCurrentSettings;

	std::vector<TextureUnitChange*> mTextureChanges;

	bool mAnimationLoop;
	Ogre::AnimationState* mAnimationState;


	bool mPositionAdapter;
	AdapterMapT mAdapters;
	size_t mAdapterSelected;
	std::string mSelectedAdapterGroup;
	bool mMenuFocus;
	OpenSaveDialog fileDialog;

	// Preview
	boost::shared_ptr<BFG::Environment> mEnvironment;
	EventLoop* mLoop;

	std::map<GameHandle, ConnectionData*> mPreviewConnections;
	std::map<GameHandle, boost::shared_ptr<BFG::View::RenderObject> > mRenderObjects;

	MyGUI::ComboBox* mGroupName;
	MyGUI::ComboBox* mParentAdapter;
	MyGUI::ComboBox* mChildAdapter;

	MyGUI::VectorWidgetPtr mPreviewMeshMenu;
	std::string mPreviewMeshName;

	AdapterMapT mPreviewAdapter;
	size_t mSelectedPreviewAdapter;
	std::string mSelectedPreviewAdapterGroup;
	GameHandle mPreviewObject;
	boost::shared_ptr<View::RenderObject> mPreviewRenderObject;


};

// This is the Ex-'GameStateManager::SingleThreadEntryPoint(void*)' function
void* SingleThreadEntryPoint(void *iPointer)
{
	EventLoop* loop = static_cast<EventLoop*>(iPointer);
	
	GameHandle siHandle = BFG::generateHandle();
	
	// Hack: Using leaking pointers, because vars would go out of scope
	ComposerState* ps = new ComposerState(siHandle, loop);
	ViewComposerState* vps = new ViewComposerState(siHandle, loop);

	// Init Controller
	GameHandle handle = generateHandle();

	{
		BFG::Controller_::ActionMapT actions;
		actions[A_QUIT] = "A_QUIT";
		actions[A_LOADING_MESH] = "A_LOADING_MESH";
		actions[A_LOADING_MATERIAL] = "A_LOADING_MATERIAL";
		actions[A_CAMERA_AXIS_X] = "A_CAMERA_AXIS_X";
		actions[A_CAMERA_AXIS_Y] = "A_CAMERA_AXIS_Y";
		actions[A_CAMERA_AXIS_Z] = "A_CAMERA_AXIS_Z";
		actions[A_CAMERA_MOVE] = "A_CAMERA_MOVE";
		actions[A_CAMERA_RESET] = "A_CAMERA_RESET";
		actions[A_CAMERA_ORBIT] = "A_CAMERA_ORBIT";
		actions[A_SCREENSHOT] = "A_SCREENSHOT";
		actions[A_LOADING_SKY] = "A_LOADING_SKY";

		actions[A_CREATE_LIGHT] = "A_CREATE_LIGHT";
		actions[A_DESTROY_LIGHT] = "A_DESTROY_LIGHT";
		actions[A_PREV_LIGHT] = "A_PREV_LIGHT";
		actions[A_NEXT_LIGHT] = "A_NEXT_LIGHT";
		actions[A_FIRST_LIGHT] = "A_FIRST_LIGHT";
		actions[A_LAST_LIGHT] = "A_LAST_LIGHT";

		actions[A_INFO_WINDOW] = "A_INFO_WINDOW";

		actions[A_CAMERA_MOUSE_X] = "A_CAMERA_MOUSE_X";
		actions[A_CAMERA_MOUSE_Y] = "A_CAMERA_MOUSE_Y";
		actions[A_CAMERA_MOUSE_Z] = "A_CAMERA_MOUSE_Z";
		actions[A_CAMERA_MOUSE_MOVE] = "A_CAMERA_MOUSE_MOVE";

		actions[A_SUB_MESH] = "A_SUB_MESH";
		actions[A_TEX_UNIT] = "A_TEX_UNIT";
		actions[A_ANIMATION] = "A_ANIMATION";
		actions[A_ADAPTER] = "A_ADAPTER";

		actions[A_MOUSE_MIDDLE_PRESSED] = "A_MOUSE_MIDDLE_PRESSED";
		BFG::Controller_::fillWithDefaultActions(actions);	
		BFG::Controller_::sendActionsToController(loop, actions);

		Path path;
		const std::string config_path = path.Expand("Composer.xml");
		const std::string state_name = "Composer";
		
		Controller_::StateInsertion si(config_path, state_name, handle, true);

		EventFactory::Create<Controller_::ControlEvent>
		(
			loop,
			ID::CE_LOAD_STATE,
			si
		);

		loop->connect(A_QUIT, ps, &ComposerState::ControllerEventHandler);
		loop->connect(A_SCREENSHOT, ps, &ComposerState::ControllerEventHandler);

		loop->connect(A_LOADING_MESH, vps, &ViewComposerState::controllerEventHandler);
		loop->connect(A_LOADING_MATERIAL, vps, &ViewComposerState::controllerEventHandler);
		loop->connect(A_CAMERA_AXIS_X, vps, &ViewComposerState::controllerEventHandler);
		loop->connect(A_CAMERA_AXIS_Y, vps, &ViewComposerState::controllerEventHandler);
		loop->connect(A_CAMERA_AXIS_Z, vps, &ViewComposerState::controllerEventHandler);
		loop->connect(A_CAMERA_MOVE, vps, &ViewComposerState::controllerEventHandler);
		loop->connect(A_CAMERA_RESET, vps, &ViewComposerState::controllerEventHandler);
		loop->connect(A_CAMERA_ORBIT, vps, &ViewComposerState::controllerEventHandler);
		loop->connect(A_LOADING_SKY, vps, &ViewComposerState::controllerEventHandler);

		loop->connect(A_CREATE_LIGHT, vps, &ViewComposerState::controllerEventHandler);
		loop->connect(A_DESTROY_LIGHT, vps, &ViewComposerState::controllerEventHandler);
		loop->connect(A_PREV_LIGHT, vps, &ViewComposerState::controllerEventHandler);
		loop->connect(A_NEXT_LIGHT, vps, &ViewComposerState::controllerEventHandler);
		loop->connect(A_FIRST_LIGHT, vps, &ViewComposerState::controllerEventHandler);
		loop->connect(A_LAST_LIGHT, vps, &ViewComposerState::controllerEventHandler);

		loop->connect(A_INFO_WINDOW, vps, &ViewComposerState::controllerEventHandler);
		loop->connect(A_SUB_MESH, vps, &ViewComposerState::controllerEventHandler);
		loop->connect(A_TEX_UNIT, vps, &ViewComposerState::controllerEventHandler);
		loop->connect(A_ANIMATION, vps, &ViewComposerState::controllerEventHandler);
		loop->connect(A_ADAPTER, vps, &ViewComposerState::controllerEventHandler);

		loop->connect(A_CAMERA_MOUSE_X, vps, &ViewComposerState::controllerEventHandler);
		loop->connect(A_CAMERA_MOUSE_Y, vps, &ViewComposerState::controllerEventHandler);
		loop->connect(A_CAMERA_MOUSE_Z, vps, &ViewComposerState::controllerEventHandler);
		loop->connect(A_CAMERA_MOUSE_MOVE, vps, &ViewComposerState::controllerEventHandler);

		loop->connect(A_MOUSE_MIDDLE_PRESSED, vps, &ViewComposerState::controllerEventHandler);
		loop->connect(BFG::ID::A_MOUSE_LEFT_PRESSED, vps, &ViewComposerState::controllerEventHandler);
		loop->connect(BFG::ID::A_MOUSE_RIGHT_PRESSED, vps, &ViewComposerState::controllerEventHandler);
	}

	assert(loop);
	loop->registerLoopEventListener(ps, &ComposerState::LoopEventHandler);
	return 0;
}

int main( int argc, const char* argv[] ) try
{
	Base::Logger::Init(Base::Logger::SL_DEBUG, "Logs/Composer.log");

	EventLoop iLoop(true);

	size_t controllerFrequency = 1000;

	const std::string caption = "Composer: He composes everything!";

	boost::scoped_ptr<Base::IEntryPoint> epView(View::Interface::getEntryPoint(caption));

	iLoop.addEntryPoint(epView.get());
	iLoop.addEntryPoint(ControllerInterface::getEntryPoint(controllerFrequency));
	iLoop.addEntryPoint(new Base::CEntryPoint(SingleThreadEntryPoint));

	iLoop.run();
	
}
catch (Ogre::Exception& e)
{
	showException(e.getFullDescription().c_str());
}
catch (std::exception& ex)
{
	showException(ex.what());
}
catch (...)
{
	showException("Unknown exception");
}
