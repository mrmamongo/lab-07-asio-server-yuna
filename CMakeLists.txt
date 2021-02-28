cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests" ON)
option(BUILD_DOCS "Build documentation" OFF)
option(BUILD_COVERAGE "Build code coverage" OFF)

set(
  HUNTER_CACHE_SERVERS
  "https://github.com/bmstu-iu8-cpp-sem-3/hunter-cache"
  CACHE STRING "Default cache server"
)

include("cmake/HunterGate.cmake")

HunterGate(
  URL "https://github.com/cpp-pm/hunter/archive/v0.23.278.tar.gz"
  SHA1 "35fa55bc1dfcf9f4d7264f8bb26cd1759ce1ae07"
)

# TODO: rename project and delete this comment
project(template VERSION 0.1.0)
string(APPEND CMAKE_CXX_FLAGS " -pedantic -Werror -Wall -Wextra")
string(APPEND CMAKE_CXX_FLAGS " -Wno-unused-command-line-argument")
string(APPEND CMAKE_CXX_FLAGS " -Wshadow -Wnon-virtual-dtor")

set(Boost_USE_STATIC_LIBS ON)

hunter_add_package(Boost COMPONENTS log thread system chrono filesystem regex date_time)
find_package(Boost COMPONENTS log log_setup thread system chrono filesystem regex date_time)

hunter_add_package(nlohmann_json)
find_package(nlohmann_json)

set(Boost_LIB
        ${Boost_LOG_SETUP_LIBRARY}
        ${Boost_LOG_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_THREAD_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        )

#==============================================LOGGER==============================================
add_library(logger INTERFACE)

target_include_directories(logger INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/logger>"
        "$<BUILD_INTERFACE:${Boost_INCLUDE_DIR}>"
        "$<INSTALL_INTERFACE:include/logger>"
        )

target_link_libraries(logger INTERFACE ${Boost_LIBRARIES} Boost::boost pthread)

add_executable(logger_demo
        sources/log_demo.cpp
        )
target_link_libraries(logger_demo logger Boost::log_setup Boost::log)
#==================================================================================================

#==============================================COMMON==============================================
add_library(common INTERFACE)

target_include_directories(common INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/common>"
        "$<BUILD_INTERFACE:${Boost_INCLUDE_DIR}>"
        "$<INSTALL_INTERFACE:include/common>"
        )

target_link_libraries(common INTERFACE
        ${Boost_LIB} logger pthread Boost::boost
        )
#==================================================================================================

#==============================================SERVER==============================================
add_library(server INTERFACE)

target_include_directories(server INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/server>"
        "$<BUILD_INTERFACE:${Boost_INCLUDE_DIR}>"
        "$<INSTALL_INTERFACE:include/server>"
        )

add_executable(server_ex
        sources/server_main.cpp
        )

target_link_libraries(server INTERFACE
        ${Boost_LIB} common logger pthread Boost::boost
        )

target_link_libraries(server_ex server common logger)
#==================================================================================================
#==============================================CLIENT==============================================
add_library(client INTERFACE)

target_include_directories(client INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/client>"
        "$<BUILD_INTERFACE:${Boost_INCLUDE_DIR}>"
        "$<INSTALL_INTERFACE:include/client>"
        )

add_executable(client_ex
        sources/client_main.cpp
        )

target_link_libraries(client INTERFACE
        ${Boost_LIB} common logger pthread Boost::boost
        )

target_link_libraries(client_ex client common logger)
#==================================================================================================

if(BUILD_DOCS)
  set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/_docs)
  set(DOXYGEN_EXAMPLE_PATH examples)
  set(DOXYGEN_GENERATE_HTML NO)
  set(DOXYGEN_GENERATE_MAN YES)
  find_package(Doxygen)
  doxygen_add_docs(docs include)
  install(DIRECTORY ${DOXYGEN_OUTPUT_DIRECTORY}/man DESTINATION share)
  # add_dependencies(install docs) need to call target docs before install or package
endif()

include(CPackConfig.cmake)

install(TARGETS client_ex server_ex
  EXPORT "${PROJECT_NAME}-targets"
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  INCLUDES DESTINATION include
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)

install(EXPORT "${PROJECT_NAME}-targets"
  NAMESPACE "${PROJECT_NAME}::"
  DESTINATION "lib/cmake"
)
