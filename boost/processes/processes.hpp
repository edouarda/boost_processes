
#pragma once

#include <boost/config.hpp>

#include <boost/asio.hpp>

#include <boost/processes/command_line.hpp>
#include <boost/processes/structures.hpp>
#include <boost/processes/scheduler.hpp>
#include <boost/processes/nexus.hpp>

#ifdef BOOST_WINDOWS
#include <boost/processes/windows.hpp>

namespace boost
{
namespace processes
{
typedef detail::scheduler<windows_platform> scheduler;
typedef detail::pipe<windows_platform> pipe;
typedef detail::hub<windows_platform> hub;
typedef detail::null<windows_platform> null;
typedef detail::stdio<windows_platform> stdio;
}
}
#else

#endif

