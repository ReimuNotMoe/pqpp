cmake_minimum_required(VERSION 3.4)
project(pqpp)

set(CMAKE_CXX_STANDARD 17)

include(GNUInstallDirs)

set(SOURCE_FILES
        CommonIncludes.hpp
        Exception.hpp
        Client.cpp Client.hpp
        Result.cpp Result.hpp
        pqpp.hpp Pool.cpp Pool.hpp)

add_library(pqpp SHARED ${SOURCE_FILES})
add_library(pqpp_static STATIC ${SOURCE_FILES})
set_target_properties(pqpp_static PROPERTIES OUTPUT_NAME pqpp)
target_link_libraries(pqpp boost_system boost_coroutine pthread pq ssl crypto)
target_link_libraries(pqpp_static boost_system boost_coroutine pthread pq ssl crypto)

add_executable(pqpp_demo demo.cpp)
target_link_libraries(pqpp_demo pqpp)

add_executable(pqpp_demo_pool demo_pool.cpp)
target_link_libraries(pqpp_demo_pool pqpp)

install(TARGETS pqpp
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(TARGETS pqpp_static
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES pqpp.hpp CommonIncludes.hpp Exception.hpp Client.hpp Result.hpp DESTINATION "include/pqpp")