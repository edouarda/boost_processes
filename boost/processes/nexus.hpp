
#pragma once

#include <boost/scoped_ptr.hpp>
#include <boost/bind.hpp>

#include <boost/lockfree/queue.hpp>
#include <boost/thread/thread.hpp>

#include <boost/algorithm/string/split.hpp>

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
        typedef slaver<Platform> this_type;

	public:
		slaver(void) : _read_event(0)
		{
			boost::system::error_code ec = Platform::make_pipe(_master);
			if (ec != 0)
			{
				throw ec;
			}

            _read_event = ::CreateEvent(0, TRUE, TRUE, 0);
            if (!_read_event)
                throw boost::system::error_code(::GetLastError(), boost::system::system_category());

            _consumer_thread.reset(new boost::thread(boost::bind(&this_type::consumer, this)));
		}

		~slaver(void)
		{
            stop_consumer();

			Platform::close(_master);
            _master = input_output();
            ::CloseHandle(_read_event);
		}

    private:
        void stop_consumer(void)
        {
            if (_consumer_thread)
            {
                OVERLAPPED copy = _ov;
                ::CancelIoEx(_master.output, &copy);
                _consumer_thread->interrupt();
                _consumer_thread->join();
                _consumer_thread.reset();
            }
        }

    private:
        void consumer(void)
        {
            try
            {
                assert(_read_event);
                assert(_master.output);

                ::ZeroMemory(&_ov, sizeof(_ov));

                _ov.hEvent = _read_event;

                std::string remainder;

                while(true)
                {
                    boost::this_thread::interruption_point();

                    static const size_t buf_size = 2048;
                    char buf[buf_size];

                    DWORD read = 0;
                    if (!::ReadFile(_master.output, buf, buf_size, &read, &_ov))
                        throw boost::system::error_code(::GetLastError(), boost::system::system_category());

                    DWORD transferred = 0;
                    if (!::GetOverlappedResult(_master.output, &_ov, &transferred, TRUE))
                        throw boost::system::error_code(::GetLastError(), boost::system::system_category());

                    if (!transferred)
                        continue;

                    // remove carriage return (Windows)                    
                    char sanitized[buf_size];
                    const char * beg_sanitized = sanitized;

                    static_assert(sizeof(sanitized) == sizeof(buf), "incompatible buffer sizes");

                    const char * end_sanitized = std::copy_if(buf, buf + transferred, sanitized, [](char c) -> bool
                    {
                        return c != '\r';
                    });

                    // now split according to newlines
                    std::deque<std::string> lines;

                    boost::algorithm::split(lines, boost::iterator_range<const char *>(beg_sanitized, end_sanitized), boost::is_any_of("\n"));

                    if (lines.empty())
                        continue;

                    std::string temp_remainder;

                    // was the last char a new line? if not the last line should be the remainder because it's not a full line yet
                    if (end_sanitized > sanitized)
                    {
                        if (*end_sanitized != '\n')
                        {
                            temp_remainder = lines.back();
                            lines.pop_back();
                        }
                    }

                    if (lines.empty())
                        continue;

                    // add remainder to the first entry
                    lines.front() = remainder + lines.front();
                    remainder = temp_remainder;
                    
                    // and push all lines, including the empty lines
                    for(std::deque<std::string>::const_iterator it = lines.begin(); it != lines.end(); ++it)
                    {
                        // in case the push fails, we block until it works
                        const std::string * copy(new std::string(*it));

                        while(!_available_lines.push(copy))
                        {
                            boost::this_thread::yield();
                        }
                    }

                }

            }

            catch(boost::thread_interrupted &) {}
            
        }
	
	public:		
		boost::system::error_code spawn(command_line cmdline, boost::processes::information & info)
		{
			return _scheduler.spawn(cmdline, output_to(_master), info); 
		}

	public:
		bool getline(std::string & line)
		{	
            const std::string * in;
            bool res = _available_lines.pop(in);		
            if (res)
            {
                line = *in;
                delete in;
            }
            return res;
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

        boost::scoped_ptr<boost::thread> _consumer_thread;
        boost::lockfree::queue<const std::string *> _available_lines;
        HANDLE _read_event;
        OVERLAPPED _ov;

		detail::scheduler<Platform> _scheduler;
	};

}
}
}
