set(CMAKE_CXX_STANDARD 17)

# for asio weird dependence 
find_package(Threads REQUIRED)
find_package(Boost 1.71.0 REQUIRED COMPONENTS system )

add_executable(tftp
    main.cpp)

target_link_libraries(tftp
    PRIVATE
        Threads::Threads
        Boost::system)