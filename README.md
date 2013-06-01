Boost.Processes
===============

*This is not an official Boost library*

Launch processes! Kill them! Launch them again! Get the output!

What currently works
--------------------

 * Basic process management (launch, kill)
 * Hub (running several processes and gathering output)

Usage
-----

If you just want to run a single process and wait for completion:

```cpp
#include <boost/processes/processes.hpp>


int main(int argc, char ** argv)
{
    boost::processes::scheduler s;

    boost::processes::information info;
    boost::system::error_code ec = s.spawn(boost::processes::command_line("notepad.exe") << "c:/windows/win.ini", info);

    s.wait();

    return 0;
}
```

If you want to run one or several processes, and gather all outputs through an unified interface:

```cpp

#include <boost/processes/processes.hpp>


int main(int argc, char ** argv)
{
    boost::processes::scheduler s;
    boost::processes::hub h;

    boost::processes::information info;
    boost::system::error_code ec = s.spawn(boost::processes::command_line("netstat") << "-n", boost::processes::output_to(h.io()), info);
    // error checking

    ec = s.spawn("ping", boost::processes::output_to(h.io()), info);
    // error checking

    s.wait();
    h.flush();

    std::string str;

    while(h.getline(str))
    {
        std::cout << str.c_str() << std::endl;
    }

    return 0;
}


```