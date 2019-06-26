cmake_minimum_required(VERSION 3.4)
project(pqpp)

set(CMAKE_CXX_STANDARD 17)

include(GNUInstallDirs)

set(SOURCE_FILES
        CommonIncludes.hpp
        Exception.hpp
        Client.cpp Client.hpp
        Result.cpp Result.hpp
        pqpp.hpp)

add_library(pqpp SHARED ${SOURCE_FILES})
add_library(pqpp_static SHARED ${SOURCE_FILES})
set_target_properties(pqpp_static PROPERTIES OUTPUT_NAME pqpp)
target_link_libraries(pqpp boost_system boost_coroutine pthread pq ssl crypto)
target_link_libraries(pqpp_static boost_system boost_coroutine pthread pq ssl crypto)

add_executable(pqpp_demo demo.cpp)
target_link_libraries(pqpp_demo pqpp)

install(FILES CommonIncludes.hpp Exception.hpp Client.hpp Result.hpp DESTINATION "include/Marisa")