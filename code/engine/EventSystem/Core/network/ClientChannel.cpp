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

#include <EventSystem/Core/network/ClientChannel.h>

#include <boost/archive/text_iarchive.hpp>

#include <Base/CLogger.h>
#include <EventSystem/Core/EventDefs.h>
#include <EventSystem/Core/EventManager.h>
#include <EventSystem/Core/EventPool.h>


namespace EventSystem
{

ClientChannel::ClientChannel(boost::asio::io_service& io_service):
NetworkChannel(io_service, false)
{
}

ClientChannel::~ClientChannel()
{
}

void ClientChannel::connect(boost::asio::ip::tcp::endpoint& ep)
{
	mSocket.connect(ep);
	startHandShake();
}

void ClientChannel::startHandShake()
{
	writeStaticMessage(boost::asio::buffer(NetworkHandshake));
	readMessage
	(
		&ClientChannel::handleHandShakeResponse,
		shared_from_this(),
		NetworkHandshakeResponse.size()
	);
}

void ClientChannel::handleHandShakeResponse(const boost::system::error_code& error)
{
	infolog << "handleClientHandshakeResponse";
	if (!error)
	{
		// Write all of the data that has been read so far.
		std::ostringstream ss;
		ss << &mIncomingBuffer;
		if (ss.str() == NetworkHandshakeResponse)
		{
			// Got a valid response from the other side
			setValid(true);

			// Send ACK to the other side, last step in authentication process
			writeStaticMessage(boost::asio::buffer(NetworkHandshakeACK));

			readMessage
			(
				&ClientChannel::handleReadHeader,
				shared_from_this(),
				8
			);
		}
	}
	else if (error != boost::asio::error::eof)
	{
		errlog << "ClientChannel::handleHandShakeResponse: " << error;
	}  
}

void ClientChannel::handleReadHeader(const boost::system::error_code& error)
{
	if (!error)
	{
		size_t dataSize;

		std::istream(&mIncomingBuffer) >> std::hex >> dataSize;  

		readMessage
		(
			&ClientChannel::handleReadData,
			shared_from_this(),
			dataSize
		);
	}
	else if (error != boost::asio::error::eof)
	{
		errlog << "ClientChannel::handleReadHeader: " << error;
	}
}

void ClientChannel::handleReadData(const boost::system::error_code& error)
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

		dbglog << "ClientChannel has received:" << std::endl
		          << (*pool);

		// mark eventpool as unused
		EventManager::getInstance()->freePool(pool);

		// prepare read for next message
		readMessage
		(
			&ClientChannel::handleReadHeader,
			shared_from_this(),
			8
		);
	}
	catch(boost::archive::archive_exception& e)
	{
		errlog << "ClientHandle::handleReadData: ArchiveError: " << e.what();
		return;
	}
	catch (std::exception& e)
	{
		errlog << "ClientHandle::handleReadData: " << e.what();
		return;
	}
}

} //namespace EventSystem 
