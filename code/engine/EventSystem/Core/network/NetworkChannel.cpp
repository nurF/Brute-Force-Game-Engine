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

#include <EventSystem/Core/network/NetworkChannel.h>

#include <boost/make_shared.hpp>

#include <Base/CLogger.h>
#include <EventSystem/Core/EventManager.h>

namespace EventSystem {

NetworkChannel::NetworkChannel(boost::asio::io_service& io_service, bool masterFlag) : 
mSocket(io_service),
mMaster(masterFlag),
mValidConnection(false)
{
}

NetworkChannel::~NetworkChannel()
{
	infolog << "NetworkChannel::Connection Lost";
	EventManager::getInstance()->unregisterConnection(this);
}

void NetworkChannel::setValid(bool valid)
{
	mValidConnection = valid;
	// do it implicitely
	registerChannel();
}

void NetworkChannel::sendPacket(const std::string& header, const std::string& data)
{
	boost::shared_ptr<std::string> headerPtr = boost::make_shared<std::string>(header);
	boost::shared_ptr<std::string> dataPtr = boost::make_shared<std::string>(data);

	std::vector<boost::asio::const_buffer> buffers;
	buffers.push_back(boost::asio::buffer(*headerPtr));
	buffers.push_back(boost::asio::buffer(*dataPtr));

	async_write
	(
		socket(),
		buffers,
		boost::bind
		(
			&NetworkChannel::handlePacketSendCompletion,
			this,
			headerPtr,
			dataPtr,
			_1,
			_2
		)
	);
}

void NetworkChannel::registerChannel()
{
	EventManager::getInstance()->registerConnection(this);
}

void NetworkChannel::writeStaticMessage(boost::asio::const_buffer buffer)
{
	mSocket.async_send
	(
		boost::asio::buffer(buffer),
		boost::bind
		(
			&NetworkChannel::handleWrite,
			this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred
		)
	);
}

void NetworkChannel::handleWrite(const boost::system::error_code& error,
								 size_t bytesTransferred)
{
	// Debug Message
	dbglog << "NetworkChannel has written " << bytesTransferred;
	if (error)
	{
		errlog << "NetworkChannel has error (" << error << ") while writing to socket ";
	}
}

void NetworkChannel::handlePacketSendCompletion(boost::shared_ptr<std::string> headerPtr,
                                                boost::shared_ptr<std::string> dataPtr,
                                                const boost::system::error_code& error,
                                                size_t bytes_transferred) const
{
	dbglog << "STUB: NetworkChannel::handlePacketSendCompletion";
}
} // namespace EventSystem
