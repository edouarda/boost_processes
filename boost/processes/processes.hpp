
#pragma once

#include <boost/config.hpp>

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
typedef detail::slaver<windows_platform> slaver;
}
}
#else

#endif

