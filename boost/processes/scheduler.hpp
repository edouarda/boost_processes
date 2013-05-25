
#pragma once

#include <boost/bind.hpp>

#include <boost/noncopyable.hpp>

#include <boost/container/flat_set.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/thread/mutex.hpp>

namespace boost
{
namespace processes
{
namespace detail
{

    template <typename Platform>
	class scheduler : public boost::noncopyable
	{
	private:
		typedef boost::container::flat_set<information> processes_set;
		typedef scheduler<Platform> this_type;

	public:
		boost::system::error_code spawn(command_line cmdline, information & info)
		{
			return spawn(cmdline, input_output(), info);
		}

		boost::system::error_code spawn(command_line cmdline, input_output io, information & info)
		{
			info = information(cmdline, io);
			boost::system::error_code ec = _platform.run(info);
			if (!ec)
			{
				boost::unique_lock<boost::mutex> lock(_mutex);

				// if we have a previous entry with the same id, close it
				// nota bene, this is a bug and should never happen
				processes_set::iterator it = _processes.find(info);
				if (it != _processes.end())
				{
					_platform.close(it->id);
					_processes.erase(it);
				}

				assert(_processes.find(info) == _processes.end());

				_processes.insert(info);
			}

			return ec;
		}

	public:
		boost::system::error_code terminate(identifier id)
		{
			boost::system::error_code ec = _platform.terminate(id);
			if (!ec)
			{
				close_process(id);
			}
			return ec;
		}

        void terminate_all(void)
        {
            processes_set copy;

            {
                // work on a copy to make sure we don't have any deadlock
                boost::unique_lock<boost::mutex> lock(_mutex);
                copy = _processes;
            }

            for(processes_set::const_iterator it = copy.begin(); it != copy.end(); ++it)
            {
                // terminate will remove the entry from _processes
                terminate(it->id);
            }
        }

	public:
		bool process_info(identifier id, information & info) const
		{
			bool found = false;
			boost::unique_lock<boost::mutex> lock(_mutex);
			processes_set::const_iterator it = _processes.find(id.pid);
			if (it != _processes.end())
			{
				info = *it;
				found = true;
			}
			return found;
		}

	private:
		void close_process(const identifier & id)
		{
			_platform.close(id);

			boost::unique_lock<boost::mutex> lock(_mutex);
			_processes.erase(id.pid);	
		}

	private:
		bool gc_process(const identifier & id)
		{
			if (_platform.running(id))
			{
				// process is alive, nothing to do
				return false;
			}

			close_process(id);
	
			return true;
		}

	public:
		bool running(identifier id)
		{
			return running(id.pid);
		}

		bool running(pid_type p) 
		{
			boost::unique_lock<boost::mutex> lock(_mutex);
			processes_set::const_iterator it = _processes.find(p);
			if (it == _processes.end())
			{
				return false;
			}
			identifier id = it->id;
			lock.unlock();
			
			// attempt to garbage collect the pid, if we can it means it exited 
			return !gc_process(id);
		}

		bool any_running(void) 
		{
			processes_set to_gc;

			{
				boost::unique_lock<boost::mutex> lock(_mutex);				

				// in C++ 11 this would be a one-liner :'(			
				for(processes_set::const_iterator it = _processes.begin(); it != _processes.end(); ++it)
				{
					if (_platform.running(it->id))
					{
						return true;
					}
					to_gc.insert(*it);
				}
			}

			// let's gc what we found to be no longer running
			for(processes_set::const_iterator it = to_gc.begin(); it != to_gc.end(); ++it)
			{
				gc_process(it->id);
			}

			return false;
		}

	public:
		void wait(void)
		{
			while (any_running())
			{
				boost::this_thread::yield();
			}
		}

	public:
		Platform _platform;

		mutable boost::mutex _mutex;
		processes_set _processes;
	};
}
}
}
