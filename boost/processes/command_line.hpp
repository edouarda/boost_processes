
#pragma once

#include <deque>
#include <numeric>
#include <string>

#include <boost/algorithm/string/trim.hpp>
#include <boost/system/error_code.hpp>

namespace boost
{
namespace processes
{

    class command_line
	{
				
	private:
		struct argv_merger
		{
			std::string operator()(std::string acc, std::string current) const
			{
				return acc.empty() ? current : (acc + " " + current);
			}
		};

	public:
		command_line(void) {}

		command_line(const char * argv0) 
		{
			(*this) << std::string(argv0);
		}

		command_line(std::string argv0)
		{
			(*this) << argv0;
		}

		command_line & operator << (std::string parameter)
		{
			boost::algorithm::trim(parameter);
			if (!parameter.empty())
			{
				_argv.push_back(parameter);
			}

			return *this;
		}

		size_t argc(void) const
		{
			return _argv.size();
		}

		const std::deque<std::string> & argv(void) const
		{
			return _argv;
		}

		const std::string & operator[](size_t index) const
		{
			return _argv[index];
		}

		operator std::string() const
		{
			return std::accumulate(_argv.begin(), _argv.end(), std::string(), argv_merger());
		}
	
	private:
		std::deque<std::string> _argv;

	};


}
}
