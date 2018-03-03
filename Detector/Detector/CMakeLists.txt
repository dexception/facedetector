cmake_minimum_required (VERSION 3.9)
project (Detector)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_INSTALL_PREFIX  ${CMAKE_CURRENT_SOURCE_DIR}/../)
#set(BOOST_ROOT "c:/Users/Ruslan/libs/build/boost/")
#set(BOOST_INCLUDE_DIR "c:/Users/Ruslan/libs/build/boost/boost/")

find_package(Boost 1.60.0 COMPONENTS program_options system log REQUIRED)
set(Boost_DEBUG on)
if (Boost_FOUND)
  include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIRS})
  file(GLOB CPPS "*.cpp")
  add_definitions(-Wall -O2)
  add_executable(${PROJECT_NAME} ${CPPS})
  target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} ${CMAKE_DL_LIBS})
  install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()
