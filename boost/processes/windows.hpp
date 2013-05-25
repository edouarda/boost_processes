
#pragma once

#include <windows.h>

namespace boost
{
namespace processes
{

    struct windows_platform 
	{

		static boost::system::error_code make_pipe(input_output & io)
		{
			SECURITY_ATTRIBUTES sa_attr;

			sa_attr.nLength = sizeof(SECURITY_ATTRIBUTES);
			sa_attr.bInheritHandle = TRUE;
			sa_attr.lpSecurityDescriptor = 0;

			if (!::CreatePipe(&io.output, &io.input, &sa_attr, 0))
			{
				return boost::system::error_code(::GetLastError(), boost::system::system_category());
			}

			// we don't want to inherit this handle
			if (!::SetHandleInformation(io.output, HANDLE_FLAG_INHERIT, 0))
			{
				::CloseHandle(io.input);
				::CloseHandle(io.output);
				return boost::system::error_code(::GetLastError(), boost::system::system_category());
			}

			return boost::system::error_code();
		}

		static boost::system::error_code read_char(const input_output & io, char & c)
		{
			DWORD read = 0;	

			if (!::PeekNamedPipe(io.output, 0, 0, 0, &read, 0) || !read)
			{
				return make_error_code(boost::system::errc::no_message_available);
			}

			BOOL res = ::ReadFile(io.output, &c, 1, &read, 0);
			if (!res || (read != 1))
			{
				return boost::system::error_code(::GetLastError(), boost::system::system_category());
			}

			return boost::system::error_code();			
		}

		static boost::system::error_code run(information & p)
		{
			assert(p.cmdline.argc() > 0);

			STARTUPINFO si;
			::ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);

			BOOL inherit_handles = FALSE;

			if (p.io.error || p.io.input || p.io.output)
			{
				si.hStdError = p.io.error;
				si.hStdInput = p.io.input;
				si.hStdOutput = p.io.output;
				si.dwFlags |= STARTF_USESTDHANDLES;
				inherit_handles = TRUE;
			}

			PROCESS_INFORMATION pi;
			::ZeroMemory(&pi, sizeof(pi));

			// CreateProcess may modify the command... so we need to provide access to a read/write buffer			
			static const int buf_size = MAX_PATH;
			TCHAR buf[buf_size];

			const std::string cmdline = p.cmdline;

			#ifdef UNICODE
			int conv_res = ::MultiByteToWideChar(CP_UTF8,
				MB_ERR_INVALID_CHARS,
				cmdline.c_str(),
				-1,
				buf,
				buf_size);
			if (!conv_res)
			{
				return boost::system::error_code(::GetLastError(), boost::system::system_category());
			}
			#else			
			// strncpy_s will always add a zero terminator or return an error if there's isn't enough room
			errno_t err = ::strncpy_s(buf, sizeof(buf), cmdline.c_str(), cmdline.size());
			if (err != 0)
			{
				return boost::system::error_code(err, boost::system::system_category());
			}
			#endif

			BOOL res = ::CreateProcess(0, // lpApplicationName 
				buf, // lpCommandLine 
				0, // lpProcessAttributes 
				0, // lpThreadAttributes 
				inherit_handles, // bInheritHandles 
				0, // dwCreationFlags
				0, // lpEnvironment
				0, // current directory
				&si,
				&pi);

			if (!res)
			{
				return boost::system::error_code(::GetLastError(), boost::system::system_category());					
			}
					
			assert(pi.hThread);
			// we don't need the thread handle, close it now to prevent leaking
			::CloseHandle(pi.hThread);

			// a handle is actually a void *, surprised?
			p.id.handle = pi.hProcess;
			assert(p.id.handle);
			p.id.pid = pi.dwProcessId;
			assert(p.id.pid);

			return boost::system::error_code();
		}

		// when exiting this function, the process is terminated or we failed at it
		static boost::system::error_code terminate(const identifier & id)
		{
			if (!id.handle)
			{
				return make_error_code(boost::system::errc::invalid_argument);
			}

			BOOL res = ::TerminateProcess(id.handle, 666);
			if (!res)
			{
				return boost::system::error_code(::GetLastError(), boost::system::system_category());		
			}

			// the process takes more than 10s to exit? -> timeout
			// keep in mind that there is no way for a Windows program to prevent termination
			if (::WaitForSingleObject(id.handle, 10000) == WAIT_TIMEOUT)
			{
				return make_error_code(boost::system::errc::timed_out);
			}

			// do NOT close the handle, we will probably need it later to request the exit code and check if the process is 
			// still running
			// when we no longer need the handle the scheduler will call close()
			return boost::system::error_code();
		}
		
		static void close(const identifier & id)
		{
			::CloseHandle(id.handle);
		}

		static void close(input_output & io)
		{
			::CloseHandle(io.input);
			io.input = 0;
			::CloseHandle(io.output);
			io.output = 0;
			::CloseHandle(io.error);
			io.error = 0;
		}

		// in Windows making this function is very easy and quite reliable, unless some nasty processe likes to exit
		// with STILL_ACTIVE, in which case we just need to find the author, take him to the forest and make him dig a hole
		// I'll let you imagine the conclusion
		static bool running(const identifier & id)
		{
			if (!id.handle)
				return false;

			DWORD exit_code;
			if (!::GetExitCodeProcess(id.handle, &exit_code))
			{
				// something is wrong
				return false;
			}

			return exit_code == STILL_ACTIVE;
		}

	};

}
}