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

#include <network/ServerChannel.h>

#include <boost/archive/text_iarchive.hpp>

#include <Base/CLogger.h>
#include <EventSystem/Core/EventPool.h>
#include <EventSystem/Core/EventManager.h>



namespace EventSystem
{

ServerChannel::ServerChannel(boost::asio::io_service& io_service):
NetworkChannel(io_service, true)
{
}

ServerChannel::~ServerChannel()
{
}

void ServerChannel::startHandShake()
{
	readMessage
	(
		&ServerChannel::handleClientHandShake,
		shared_from_this(),
		NetworkHandshake.size()
	);
}
 
void ServerChannel::handleClientHandShake(const boost::system::error_code& error)
{
	infolog << "ServerChannel::handleServerHandshake";
	if (!error)
	{
		std::ostringstream ss;
		ss << &mIncomingBuffer;

		if (ss.str() == NetworkHandshake)
		{
			// Send ACK to the other side, last step in authentication process
			writeStaticMessage(boost::asio::buffer(NetworkHandshakeResponse));

			readMessage
			(
				&ServerChannel::handleHandShakeResponse,
				shared_from_this(),
				NetworkHandshakeACK.size()
			);
		}
		else
		{
			errlog << "ServerChannel::handleClientHandShake: received wrong package >> "
			       << ss.str();
		}
	}
}
 
void ServerChannel::handleHandShakeResponse(const boost::system::error_code& error)
{
	infolog << "ServerChannel::handleHandshakeResponse";
	if (!error)
	{
		// Write all of the data that has been read so far.
		std::ostringstream ss;
		ss << &mIncomingBuffer;

		if (ss.str() == NetworkHandshakeACK)
		{
			// Got a valid response from the other side
			setValid(true);

			// Continue reading remaining data until EOF.
			readMessage
			(
				&ServerChannel::handleReadHeader,
				shared_from_this(),
				8
			);
		}
		else
		{
			errlog << "ServerChannel::handleHandShakeResponse << received wrong package >> "
			       << ss.str();
		}
	}
	else if (error != boost::asio::error::eof)
	{
		errlog << "ServerChannel::handleHandshakeResponse: " << error << "\n";
	}  
}

void ServerChannel::handleReadHeader(const boost::system::error_code& error)
{
	if (!error)
	{
		size_t dataSize;

		std::istream(&mIncomingBuffer) >> std::hex >> dataSize;  

		readMessage
		(
			&ServerChannel::handleReadData,
			shared_from_this(),
			dataSize
		);
	}
	else
	{
		errlog << "ServerChannel::handleReader: " << error.value();
		errlog << "ServerChannel::handleReader: " << error.category().name();
	}
}

void ServerChannel::handleReadData(const boost::system::error_code& error)
{
	try
	{
		std::istream dataStream(&mIncomingBuffer);
		boost::archive::text_iarchive archive(dataStream);

		// request a new or unused eventpool
		BaseEventPool* pool = EventManager::getInstance()->requestPool();

		archive & (*pool);

		// copy this pool to all channels
		EventManager::getInstance()->publishNetworkPool(pool);

		dbglog << "ServerChannel has received:" << std::endl
		       << (*pool);

		// mark eventpool as unused
		EventManager::getInstance()->freePool(pool);

		// prepare read for next message
		readMessage
		(
			&ServerChannel::handleReadHeader,
			shared_from_this(),
			8
		);
	}
	catch(boost::archive::archive_exception& e)
	{
		errlog << "ServerChannel::handleReadData: " << e.what();
		return;
	}
	catch (std::exception& e)
	{
		errlog << "ServerChannel::handleReadData: " << e.what();
		return;
	}
}

} //namespace EventSystem
