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

#ifndef __EVENT_SERVER_CHANNEL_H_
#define __EVENT_SERVER_CHANNEL_H_

#include <EventSystem/Core/network/NetworkChannel.h>

#include <boost/enable_shared_from_this.hpp>


namespace EventSystem
{

class ServerChannel : public NetworkChannel,
                      public boost::enable_shared_from_this<ServerChannel>
{
public:
	typedef boost::shared_ptr<ServerChannel> Pointer;
	static Pointer create(boost::asio::io_service& ioService)
	{
		return Pointer(new ServerChannel(ioService));
	}

	virtual ~ServerChannel();

	virtual void startHandShake();
private:
	ServerChannel(boost::asio::io_service& ioService);

	virtual void handleHandShakeResponse(const boost::system::error_code& error);
	void handleClientHandShake(const boost::system::error_code& error);

	virtual void handleReadHeader(const boost::system::error_code& error);
	virtual void handleReadData(const boost::system::error_code& error);

};

}

#endif //__EVENT_SERVER_CHANNEL_H_
