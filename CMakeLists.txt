cmake_minimum_required(VERSION 3.5)
  project(yaml_test)
  find_package(yaml-cpp)
  FIND_PACKAGE( Boost 1.40 COMPONENTS system REQUIRED )

  include_directories(${YAML_CPP_INCLUDE_DIRS}  ${Boost_INCLUDE_DIR})
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)

  add_executable(yaml_test src/Main.cpp src/Config.cpp)
  target_link_libraries(yaml_test ${YAML_CPP_LIBRARIES} ${Boost_LIBRARIES} )
