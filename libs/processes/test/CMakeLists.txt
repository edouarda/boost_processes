include_directories(${Boost_INCLUDE_DIRS}
    ${Tbb_INCLUDE_DIR}
    ${QDB_INCLUDE})

add_executable(boost_processes_test 
    tests.cpp)

target_link_libraries (boost_processes_test


    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_DATE_TIME_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    ${Boost_CHRONO_LIBRARY}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${Boost_PRG_EXEC_MONITOR_LIBRARY}

    ${Realtime_LIBRARY}
    ${Pthread_LIBRARY}

    ${Additional_LIBRARIES})

add_dependencies(boost_processes_test boost_processes)

add_test(NAME unit COMMAND boost_processes_test --log_level=test_suite --build-info=yes)