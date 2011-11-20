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

#include "CLogger.h"

#pragma warning( disable: 4512 )

#include <iostream>
#include <fstream>

#include <boost/log/filters.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/formatters/stream.hpp>
#include <boost/log/formatters/message.hpp>
#include <boost/log/attributes/clock.hpp>
#include <boost/log/utility/init/to_file.hpp>
#include <boost/log/utility/init/common_attributes.hpp>
#include <boost/log/utility/empty_deleter.hpp>
#include <boost/log/formatters/date_time.hpp>


namespace BFG {
namespace Base {
namespace Logger {

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace fmt = boost::log::formatters;
namespace flt = boost::log::filters;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

void setupFileSink(const std::string& Filename);
void setupCoutSink();
void setupCore(SeverityLevel Min_Log_Level);

std::ostream& operator<< (std::ostream& strm, const SeverityLevel& svl)
{
	std::string val = "";

	switch (svl)
	{
		case SL_DEBUG:
			val = "debug>\t";
		break;
		case SL_INFORMATION:
			val = "information>";
		break;
		case SL_WARNING:
			val = "warning>\t";
		break;
		case SL_ERROR:
			val = "error>\t";
		break;
	}

	return strm << "< " << val;
}

void Init(SeverityLevel Min_Log_Level, const std::string& Filename)
{
	static bool already_initialized = false;
	
	assert("It seems that you called Base::Logger::Init() twice."
	       " This is not recommended." && ! already_initialized);

	already_initialized = true;

	setupCore(Min_Log_Level);
	setupFileSink(Filename);
	setupCoutSink();
}

void setupFileSink(const std::string& Filename)
{
	if (Filename.empty())
		return;

	typedef sinks::synchronous_sink<sinks::text_ostream_backend> text_sink;
	boost::shared_ptr<text_sink> sink = boost::make_shared<text_sink>();
	
	sink->locked_backend()->add_stream
	(
		boost::make_shared<std::ofstream>(Filename.c_str())
	);

	logging::add_common_attributes();

	sink->locked_backend()->set_formatter
	(
	fmt::stream 
	<< fmt::attr<unsigned int>("LineID")
	<< ". " << fmt::time("TimeStamp") 
	<< " " <<fmt::attr<SeverityLevel>("Severity") 
	<< ":\t " << fmt::message()
	);
	
	sink->locked_backend()->auto_flush(true);
		
	logging::core::get()->add_sink(sink);
}

void setupCoutSink()
{
	typedef sinks::synchronous_sink<sinks::text_ostream_backend> TextSinkT;
	boost::shared_ptr<TextSinkT> pSink = boost::make_shared<TextSinkT>();

	boost::shared_ptr<std::ostream> pOutStream
	(
		&std::cout,
		logging::empty_deleter()
	);

	pSink->locked_backend()->add_stream(pOutStream);
	logging::core::get()->add_sink(pSink);
}

void setupCore(SeverityLevel Min_Log_Level)
{
	boost::shared_ptr< logging::core > pCore = logging::core::get();

	pCore->add_global_attribute
	(
		"TimeStamp",
		attrs::local_clock()
	);
	
	pCore->set_filter
	(
		flt::attr
		<
			SeverityLevel
		>("Severity") >= Min_Log_Level
	);
}

} // Logger
} // Base

//BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(usp_log, BFG::Base::Logger::SourceT);
BOOST_LOG_GLOBAL_LOGGER_DEFAULT(usp_log, BFG::Base::Logger::SourceT);

} // namespace BFG
