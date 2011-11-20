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

//
// The Event-Channel secures the Communication between EventManagers
//
// Incoming means : Communication from the IP-RealWorld to this EventManager
// Outgoing means : Communication from this EventManager to the IP-Realworld
// To sum this up, proper communication needs the correct usage of :
//
// GetPublishEvents + ReceiveEvents  ( EventManager )
//
// Remember! This will only be used in a threaded environment and you never work directly with this class.
//
// This class is similar to the original EventChannel, perhaps we the Interfaces will be joined someday
//

#ifndef __EVENT_NETWORK_CHANNEL_H_
#define __EVENT_NETWORK_CHANNEL_H_

#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include <Base/Cpp.h>


namespace EventSystem
{

const std::string NetworkHandshake = "EventSystem Network Channel";
const std::string NetworkHandshakeResponse = "EventSystem Network Channel Response";
const std::string NetworkHandshakeACK = "EventSystem Network Channel ACK";

class NetworkChannel
{
public:
	typedef boost::shared_ptr<NetworkChannel> Pointer;

	boost::asio::ip::tcp::socket& socket()
	{
		return mSocket;
	}

	virtual ~NetworkChannel();

	virtual void startHandShake() = 0;

	virtual void setValid(bool valid);

	void sendPacket(const std::string& header, const std::string& data);

	template <typename T>
	void readMessage(void (T::*handleMethod)(const boost::system::error_code&),
	                 boost::shared_ptr<T> otherThis,
                     size_t size)
	{
		boost::asio::async_read
		(
			mSocket,
			mIncomingBuffer,
			boost::asio::transfer_exactly(size),
			boost::bind
			(
				handleMethod,
				otherThis,
				boost::asio::placeholders::error
			)
		);
	}

protected:
	NetworkChannel(boost::asio::io_service& io_service, bool masterFlag = true);

	virtual void registerChannel();

	void writeStaticMessage(boost::asio::const_buffer buffer);

	void handleWrite(const boost::system::error_code& error,
	                 size_t bytes_transferred);

	void handlePacketSendCompletion(boost::shared_ptr<std::string> headerPtr,
	                                boost::shared_ptr<std::string> dataPtr,
	                                const boost::system::error_code& error,
	                                size_t bytes_transferred) const;


	//! Flag indicating which sides has started the connection
	bool mMaster;

	//! Flag indicating if proper Handshaking has occured
	bool mValidConnection; 

	//! The socket of this Channel
	boost::asio::ip::tcp::socket mSocket;

	//std::string message_;
	//! Hopefully this buffer is big enough
	boost::asio::streambuf mIncomingBuffer;

};

}


#endif //__EVENT_NETWORK_CHANNEL_H_
