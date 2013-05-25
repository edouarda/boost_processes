Boost.Processes
===============

Launch processes! Kill them! Launch them again! Get the output!

What currently works
--------------------

    * Basic process management (launch, kill)

Usage
-----

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