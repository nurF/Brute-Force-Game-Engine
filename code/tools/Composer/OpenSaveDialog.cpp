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

#include <OpenSaveDialog.h>

#include <boost/filesystem.hpp>
#include <Base/CLogger.h>
#include <Core/Path.h>

OpenSaveDialog::OpenSaveDialog()
{
	using namespace MyGUI;

	BFG::Path path;
	std::string layout = path.Expand("OpenSaveDialog.layout");
	mContainer = LayoutManager::getInstance().load(layout);

	Gui* gui = Gui::getInstancePtr();

	mWindow = gui->findWidget<Window>("OpenSaveDialog");
	mFilesList = gui->findWidget<List>("FilesList");
	mFileNameEdit = gui->findWidget<Edit>("FileNameEdit");
	mCurrentFolderEdit = gui->findWidget<Edit>("CurrentFolderEdit");
	mOpenSaveButton = gui->findWidget<Button>("OpenSaveButton");

	mWindow->eventWindowButtonPressed = 
		newDelegate(this, &OpenSaveDialog::notifyWindowButtonPressed);
	mFilesList->eventListChangePosition = 
		newDelegate(this, &OpenSaveDialog::notifyListChangePosition);
	mFilesList->eventListSelectAccept = 
		newDelegate(this, &OpenSaveDialog::notifyListSelectAccept);
	mFileNameEdit->eventEditSelectAccept = 
		newDelegate(this, &OpenSaveDialog::notifyEditSelectAccept);
	mFileNameEdit->eventEditTextChange =
		newDelegate(this, &OpenSaveDialog::notifyEditTextChanged);

	mCurrentFolder = boost::filesystem::current_path().string();

	update();
	setVisible(false);
}

void OpenSaveDialog::notifyWindowButtonPressed(MyGUI::Window* sender,
                                               const std::string& name)
{
	if (name == "close")
	{
		setVisible(false);
		mFileName = "";
		mFileNameEdit->setCaption("");
	}
}

void OpenSaveDialog::notifyEditSelectAccept(MyGUI::Edit* sender)
{
	accept();
}

void OpenSaveDialog::setDialogInfo(const std::string& caption,
                                   const std::string& button,
                                   MyGUI::delegates::IDelegate1<MyGUI::Widget*>* clickHandler)
{
	mWindow->setCaption(caption);
	mOpenSaveButton->setCaption(button);
	mOpenSaveButton->eventMouseButtonClick = clickHandler;
}

void OpenSaveDialog::notifyEditTextChanged(MyGUI::Edit* sender)
{
	accept();
}

void OpenSaveDialog::notifyListChangePosition(MyGUI::List* sender,
                                              size_t index)
{
	if (index == MyGUI::ITEM_NONE)
	{
		mFileNameEdit->setCaption("");
		return;
	}

	FileInfo info = *sender->getItemDataAt<FileInfo>(index);
	if (!info.mFileName.empty())
	{
		mFileNameEdit->setCaption(info.mFileName);
		accept();
	}
}

void OpenSaveDialog::notifyListSelectAccept(MyGUI::List* sender, size_t index)
{
	if (index == MyGUI::ITEM_NONE) return;

	FileInfo info = *sender->getItemDataAt<FileInfo>(index);
	if (info.mFileName.empty())
	{
		mCurrentFolder = info.mFolder;
		update();
	}
	else
	{
		mFileNameEdit->setCaption(info.mFileName);
		accept();
	}
}

void OpenSaveDialog::accept()
{
	mFileName = mCurrentFolder + "/" + mFileNameEdit->getCaption();
}

void OpenSaveDialog::fillInfoVector(FileInfoVectorT& folders,
                                    FileInfoVectorT& files)
{
	using namespace boost::filesystem;

	path p(mCurrentFolder);

	if(!exists(p))
		throw std::runtime_error("Current folder doesn't exist: " + mCurrentFolder);

	if (!is_directory(p))
		throw std::logic_error("Current folder is NOT a folder: " + mCurrentFolder);

	directory_iterator it(p), end;

	for (; it != end; ++it)
	{
		path pathEntry((*it).path());

		if(is_regular_file(pathEntry))
		{
			std::string folder = pathEntry.parent_path().string();
			std::string filename = pathEntry.filename().string();

			files.push_back(FileInfo(filename, folder));
		}
		else if(is_directory(pathEntry))
		{
			std::string folder = pathEntry.string();
			std::string filename("");

			folders.push_back(FileInfo(filename, folder));
		}
	}
}

void OpenSaveDialog::update()
{
	using namespace boost::filesystem;

	mCurrentFolderEdit->setCaption(mCurrentFolder);

	mFilesList->removeAllItems();

	// add all folders first
	FileInfoVectorT folders;
	FileInfoVectorT files;

	fillInfoVector(folders, files);

	path p(mCurrentFolder);

	if(p != p.root_path())
	{
		FileInfo parent("", p.parent_path().string());
		mFilesList->addItem("[..]", parent);
	}

	for(FileInfoVectorT::iterator it = folders.begin(); it != folders.end(); ++it)
	{
		path folderPath((*it).mFolder);

 		mFilesList->addItem("[" + folderPath.filename().string() + "]", *it);
	}

	for(FileInfoVectorT::iterator it = files.begin(); it != files.end(); ++it)
	{
		mFilesList->addItem((*it).mFileName, *it);
	}
}
