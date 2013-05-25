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

BOOST_AUTO_TEST_CASE(slaver)
{

	boost::processes::slaver s;

	boost::processes::information info;
	boost::system::error_code ec = s.spawn(boost::processes::command_line("netstat") << "-n", info);
	BOOST_REQUIRE_MESSAGE(!ec, "error: " << ec.message());

	ec = s.spawn("ping", info);
	BOOST_REQUIRE_MESSAGE(!ec, "error: " << ec.message());

	std::string str;

	while(!ec)
	{
		ec = s.getline(str);
		std::cout << str.c_str() << std::endl;
	}

    s.wait();
    BOOST_CHECK(!s.any_running());

}
