
#pragma once

#include <queue>
#include <sstream>

#include <boost/static_assert.hpp>

#include <boost/scoped_ptr.hpp>
#include <boost/bind.hpp>

#include <boost/thread/thread.hpp>

#include <boost/algorithm/string/split.hpp>

namespace boost
{
namespace processes
{
namespace detail
{

	template <typename Platform>
	class device : public boost::noncopyable
	{

	protected:
		template <typename Initializer>
		device(Initializer init)
		{
			boost::system::error_code ec = init(_device);
			if (ec != 0)
				throw ec;
		}

	public:
		~device(void)
		{
			close();
		}

	public:
		void close(void)
		{
			Platform::close(_device);
		}

	public:
		const input_output & io(void) const
		{
			return _device;
		}

	protected:
		input_output _device;
	};

	template <typename Platform>
	class null : public device<Platform>
	{
	public:
		null(void) : device<Platform>(&Platform::make_null) {}
	};

	template <typename Platform>
	class pipe : public device<Platform>
	{
	public:
		explicit pipe(bool inherit_output = true) : device<Platform>(boost::bind(&Platform::make_pipe, _1, inherit_output)) {}

	public:
		boost::system::error_code peek(size_t & avail) const
		{
			return Platform::peek_pipe(_device, avail);
		}

		boost::system::error_code read(void * p, size_t l, size_t & read)
		{
			return Platform::sync_read_pipe(_device, p, l, read);
		}
	};

	template <typename Platform>
	class stdio : public device<Platform>
	{
	public:
		stdio(void) : device<Platform>(&Platform::make_stdio) {}
	};

    template <typename Platform>
	class hub : public pipe<Platform>
	{

    public:
        typedef hub<Platform> this_type;

	public:
		hub(void) : pipe<Platform>(false), _remaining(0)
		{		
			_consumer_thread.reset(new boost::thread(boost::bind(&this_type::consumer_thread, this)));
		}

		~hub(void)
		{
            stop_consumer();
		}

    private:
        void stop_consumer(void)
        {
			pipe<Platform>::close();
            
            if (_consumer_thread)
            {
                _consumer_thread->join();
                _consumer_thread.reset();
            }
        }

	private:
		void update_remaining(void)
		{
			// no need to hold the lock during the system call
			size_t r = 0;
			boost::system::error_code ec = pipe<Platform>::peek(r);

			boost::unique_lock<boost::mutex> lock(_lines_mutex);
			_last_error = ec;
			_remaining = r;
		}

	private:
		void consumer_thread(void)
		{
			std::string remainder;

			assert(!_last_error);

			static const DWORD buf_size = 2048;
			char buf[buf_size];

			while(!_last_error)
			{
				update_remaining();

				// inform all waiting thread of the current state just before the read
				_lines_cond.notify_all();

				size_t transfered = 0;

				boost::system::error_code ec = pipe<Platform>::read(buf, buf_size, transfered);

				boost::unique_lock<boost::mutex> lock(_lines_mutex);

				_last_error = ec;

				if ((_last_error != 0) || !transfered)
					continue;

				// stringstream is certainly not the fastest way, but it's the most
				// portable way
				std::stringstream ss;
				ss.write(buf, transfered);

				if (ss.bad())
				{
					// got an error :'(
					// not much we can do, propably an out of memory condition let's try again and drop
					// that data
					continue;
				}

				while (!ss.eof())
				{
					std::string new_line;
					std::getline(ss, new_line);
					_lines.push(new_line);
				}
			}

			// loop exited, signal all as well (error is non zero)
			_lines_cond.notify_all();

		}
   
	public:
		bool unsafe_getline(std::string & s)
		{
			bool avail = !_lines.empty();
			if (avail)
			{
				s = _lines.front();
				_lines.pop();
			}
			return avail;
		}

	public:		
		bool getline(std::string & s)
		{
			boost::unique_lock<boost::mutex> lock(_lines_mutex);
			return !_last_error && unsafe_getline(s);
		}

	public:
		void flush(void)
		{
			boost::unique_lock<boost::mutex> lock(_lines_mutex);
			while (!_last_error && (_remaining > 0))
			{
				_lines_cond.wait(lock);
			}
		}

	private:
		mutable boost::mutex _lines_mutex;
		mutable boost::condition_variable _lines_cond;
		std::queue<std::string> _lines;

		boost::system::error_code _last_error;		
		volatile size_t _remaining;

        boost::scoped_ptr<boost::thread> _consumer_thread;
		
	};

}

}
}
