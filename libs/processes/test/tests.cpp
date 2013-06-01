#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>

#include <boost/processes/processes.hpp>

static void create_terminate_sub(boost::processes::command_line cmdline)
{
    boost::processes::scheduler s;

	boost::processes::information info;
	boost::system::error_code ec = s.spawn(cmdline, info);

	BOOST_REQUIRE_MESSAGE(!ec, "error: " << ec.message());

	BOOST_CHECK(info.id.pid);
	BOOST_CHECK(info.id.handle);
	BOOST_CHECK(info.cmdline.argc() > 0);
	BOOST_CHECK(!info.io.input);
	BOOST_CHECK(!info.io.output);
	BOOST_CHECK(!info.io.error);

	BOOST_CHECK(s.running(info.id));

	ec = s.terminate(info.id);
	BOOST_REQUIRE_MESSAGE(!ec, "error: " << ec.message());

	BOOST_CHECK(!s.running(info.id));
	BOOST_CHECK(!s.any_running());
}

BOOST_AUTO_TEST_CASE(create_terminate)
{
	create_terminate_sub("notepad.exe");
	create_terminate_sub("c:/windows/notepad.exe");
	create_terminate_sub(boost::processes::command_line("notepad.exe") << "c:/windows/win.ini");
	create_terminate_sub(boost::processes::command_line("c:/windows/notepad.exe") << "c:/windows/win.ini");
}

BOOST_AUTO_TEST_CASE(pipe)
{

	boost::processes::scheduler s;
	boost::processes::pipe p(true);
	boost::processes::null n;
	boost::processes::stdio stdio;

	boost::processes::information info;

	boost::processes::input_output io;

	io.input = p.io().output;
	//io.output = stdio.io().input;

	boost::system::error_code ec = s.spawn("sort", io, info);
	BOOST_REQUIRE_MESSAGE(!ec, "error: " << ec.message());

	DWORD w = 0;
	::WriteFile(p.io().input, "bl\n", 3, &w, 0);

	//io.input = 0;
	//io.output = p.io().input;

	//ec = s.spawn("ping", io, info);
	//BOOST_REQUIRE_MESSAGE(!ec, "error: " << ec.message());

	s.wait();

}

BOOST_AUTO_TEST_CASE(hub)
{

	boost::processes::scheduler s;
	boost::processes::hub h;

	boost::processes::information info;
	boost::system::error_code ec = s.spawn(boost::processes::command_line("netstat") << "-n", boost::processes::output_to(h.io()), info);
	BOOST_REQUIRE_MESSAGE(!ec, "error: " << ec.message());

	ec = s.spawn("ping", info);
	BOOST_REQUIRE_MESSAGE(!ec, "error: " << ec.message());

    s.wait();
	h.flush();

	std::string str;

	while(h.getline(str))
	{
		std::cout << str.c_str() << std::endl;
	}

    BOOST_CHECK(!s.any_running());

}
