cmake_minimum_required(VERSION 3.5)
project(pwrseqd)

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
	# Update submodules as needed
	option(GIT_SUBMODULE "Check submodules during build" ON)
		if(GIT_SUBMODULE)
			message(STATUS "Submodule update")
			execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
					WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
					RESULT_VARIABLE GIT_SUBMOD_RESULT)
		if(NOT GIT_SUBMOD_RESULT EQUAL "0")
			message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
		endif()
	endif()
endif()

find_package(yaml-cpp)
FIND_PACKAGE( Boost 1.55 COMPONENTS  REQUIRED )

find_library(GPIOD_LIBRARY NAMES libgpiod.so)
if(NOT GPIOD_LIBRARY)
	message(FATAL_ERROR "gpiod library not found.")
endif()
find_library(GPIODCXX_LIBRARY NAMES libgpiodcxx.so)
if(NOT GPIODCXX_LIBRARY)
	message(FATAL_ERROR "gpiodcxx library not found.")
endif()
find_path(GPIOD_INCLUDE_DIR gpiod.hpp)
if(NOT GPIOD_INCLUDE_DIR)
	message(FATAL_ERROR "gpiod headers not found.")
endif()
find_package(PkgConfig REQUIRED)
pkg_check_modules(PHOSPHOR_LOGGING phosphor-logging)
if(PHOSPHOR_LOGGING)
	include_directories(${LOGGING_INCLUDE_DIRS})
	link_directories(${LOGGING_LIBRARY_DIRS})
	add_definitions(-DWITH_PHOSPHOR_LOGGING=1)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

option(BUILD_TESTS "Build the tests" ON)
if(BUILD_TESTS)
	enable_testing()
	include(GoogleTest)
	add_subdirectory(tests)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_TEST OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(USE_BOOST_FILESYSTEM OFF CACHE BOOL "" FORCE)
add_subdirectory(extern/inotify-cpp)

set(BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(extern/popl)

include_directories(src/ ${YAML_CPP_INCLUDE_DIRS} ${Boost_INCLUDE_DIR} ${GPIOD_INCLUDE_DIR} extern/inotify-cpp/src/include extern/popl/include)

add_executable(psd src/Main.cpp src/Config.cpp src/StateMachine.cpp src/GpioInput.cpp src/GpioOutput.cpp src/Signal.cpp
	src/SignalProvider.cpp src/Logic.cpp src/LogicInput.cpp src/NullInput.cpp src/NullOutput.cpp src/VoltageRegulator.cpp
	src/ACPIStates.cpp)
target_link_libraries(psd ${YAML_CPP_LIBRARIES} ${GPIODCXX_LIBRARY} Threads::Threads inotify-cpp::inotify-cpp)

install(TARGETS psd DESTINATION bin)

