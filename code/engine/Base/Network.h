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

#ifndef BFG_BASE_NETWORK_H
#define BFG_BASE_NETWORK_H

#include <boost/asio.hpp>

namespace BFG {
namespace Base {

//! FQDN to IP (e.g. boost.org to 129.79.245.252)
void resolveDns(const std::string& fqdn, std::string& ip)
{
	using namespace boost::asio;
	io_service ioService;
	ip::tcp::resolver resolver(ioService);
	ip::tcp::resolver::query query(fqdn, "http");
	ip::tcp::resolver::iterator iter = resolver.resolve(query);
	ip::tcp::resolver::iterator end;
	ip::tcp::endpoint endpoint = *iter;
	ip = endpoint.address().to_string();
}

} // namespace Base
} // namespace BFG

#endif