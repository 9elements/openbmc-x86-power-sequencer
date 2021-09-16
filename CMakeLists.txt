cmake_minimum_required(VERSION 3.5)
  project(yaml_test)
  find_package(yaml-cpp)
  include_directories(${YAML_CPP_INCLUDE_DIRS})
   
  add_executable(yaml_test src/Main.cpp)
  target_link_libraries(yaml_test ${YAML_CPP_LIBRARIES})
