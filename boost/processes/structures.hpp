
#pragma once

#include <boost/fusion/adapted/struct/define_struct.hpp>
#include <boost/fusion/include/define_struct.hpp>

#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <boost/operators.hpp>

#ifdef BOOST_WINDOWS 
namespace boost
{
namespace processes
{
    // typedef pid_t pid_type;
	typedef DWORD pid_type;
}
}

BOOST_FUSION_DEFINE_STRUCT((boost)(processes), identifier, (boost::processes::pid_type, pid)(void *, handle));
BOOST_FUSION_DEFINE_STRUCT((boost)(processes), input_output, (void *, input)(void *, output)(void *, error));
#else

#include <sys/types.h>

namespace boost
{
namespace processes
{
	// typedef pid_t pid_type;
	typedef pid_t pid_type;
}
}

BOOST_FUSION_DEFINE_STRUCT((boost)(processes), identifier, (boost::processes::pid_type, pid));
BOOST_FUSION_DEFINE_STRUCT((boost)(processes), input_output, (int, input)(int, output)(int, error));
#endif

namespace boost
{
namespace processes
{

	inline input_output input_from(const boost::processes::input_output & io)
	{
		return input_output(io.output, 0, 0);
	}

	inline input_output output_to(const boost::processes::input_output & io)
	{
		return input_output(0, io.input, io.input);
	}

	inline input_output from_to(const boost::processes::input_output & from, const boost::processes::input_output & to)
	{
		return input_output(from.output, to.input, to.input);
	}

	inline bool operator < (const identifier & left, const identifier & right)
	{
		return left.pid < right.pid;
	}

	inline bool operator == (const identifier & left, const identifier & right)
	{
		return left.pid == right.pid;
	}

	inline bool operator != (const identifier & left, const identifier & right)
	{
		return left.pid != right.pid;
	}

	struct information : boost::partially_ordered<information>, boost::equality_comparable<information>
	{

		information(void) {}
		information(command_line c, input_output i) : cmdline(c), io(i) {}
		// used to look for a process via its pid in a set
		information(pid_type pid) : id(pid, 0) {}

		bool operator < (const information & other) const
		{
			return this->id < other.id;
		}

		bool operator == (const information & other) const
		{
			// what if the id has been recycled? Well, you shouldn't compare process object directly
			// this comparison is useful for the managing classes that know what's actually going on
			return (this->id == other.id);		
		}

		identifier id;
		command_line cmdline;
		input_output io;

	};

}
}

BOOST_FUSION_ADAPT_STRUCT(boost::processes::information, 
	(boost::processes::identifier, id)
	(boost::processes::command_line, cmdline)
	(boost::processes::input_output, io));
