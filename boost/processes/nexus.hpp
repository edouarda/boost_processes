
#pragma once

#include <boost/processes/structures.hpp>
#include <boost/processes/scheduler.hpp>

namespace boost
{
namespace processes
{
namespace detail
{

    template <typename Platform>
	class slaver : public boost::noncopyable
	{

	public:
		slaver(void)
		{
			boost::system::error_code ec = Platform::make_pipe(_master);
			if (ec != 0)
			{
				throw ec;
			}
		}

		~slaver(void)
		{
			Platform::close(_master);
		}
	
	public:		
		boost::system::error_code spawn(command_line cmdline, boost::processes::information & info)
		{
			return _scheduler.spawn(cmdline, output_to(_master), info); 
		}

	public:
		boost::system::error_code getline(std::string & line)
		{	
			line.clear();

			char c = '\0';

			boost::system::error_code ec;

			while(!ec)
			{
				ec = Platform::read_char(_master, c);
				if (ec != 0)
				{
					break;
				}

				if (c == '\n')
				{
					break;
				}

				if (c != '\r')
				{
					line += c;
				}
			}

			return ec;			
		}

	public:
		bool any_running(void)
		{
			return _scheduler.any_running();
		}

		void wait(void)
		{
			_scheduler.wait();
		}

	private:
		input_output _master;

		detail::scheduler<Platform> _scheduler;
	};

}
}
}
