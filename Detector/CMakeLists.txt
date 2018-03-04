cmake_minimum_required (VERSION 3.9)
project (Detector)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_INSTALL_PREFIX  ${CMAKE_CURRENT_SOURCE_DIR}/../)
if (WIN32 OR WIN64)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)  
  add_definitions(-D_UNICODE)
endif()

# Don't forget to assign through command line values to boost system variable
# for instance a value for BOOST_LIBRARYDIR

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.60.0 COMPONENTS program_options system log REQUIRED)
if (Boost_FOUND)
  include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIR})
  
  file(GLOB CPPS "*.cpp")
  add_definitions(-Wall)
  add_executable(${PROJECT_NAME} ${CPPS})
  target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} ${CMAKE_DL_LIBS})
endif(Boost_FOUND)
