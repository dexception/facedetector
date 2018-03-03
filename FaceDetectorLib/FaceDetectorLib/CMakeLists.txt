cmake_minimum_required (VERSION 3.9)
set (PROJECT_NAME "FaceDetectorLib" CXX)
project (${PROJECT_NAME})
set (PROJECT_SOURCE_DECLARATION_DIRECTORY ${PROJECT_SOURCE_DIR}/include)
SET(PROJECT_SOURCE_DEFINITION_DIRECTORY ${PROJECT_SOURCE_DIR})


# Set options used by "CMake".
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS:STRING=-fPIC)
SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
SET(BUILD_SHARED_LIBS TRUE)
SET(CMAKE_INSTALL_PREFIX  ${CMAKE_CURRENT_SOURCE_DIR}/../)
SET(BOOST_ROOT "/usr/local")
SET(BOOST_INCLUDE_DIR "${BOOST_ROOT}/include/boost")
SET(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib")
#SET(Boost_DEBUG ON)
set(Boost_NO_SYSTEM_PATHS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
#set(Boost_THREADAPI "win32")
set(Boost_ALL_DYN_LINK ON)
add_definitions(-DBOOST_LOG_DYN_LINK=1)
#find_package(Boost 1.60.0 REQUIRED COMPONENTS system filesystem log thread asio)
find_package(Boost 1.60.0 REQUIRED COMPONENTS system filesystem log_setup log thread regex)

set(OpenCV_MAIN_PATH "/home/ruslan/opencv/build")
set(OpenCV_DIR "${OpenCV_MAIN_PATH}")
SET(OpenCV_LIB_DIR "${OpenCV_MAIN_PATH}/build/lib")
#SET(OpenCV_INCLUDE_DIRS "${OpenCV_DIR}/include")
SET(USED_LIBS ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_REGEX_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${Boost_LOG_LIBRARY} ${Boost_ASIO_LIBRARY})

find_package( OpenCV REQUIRED )

LINK_DIRECTORIES(${OpenCV_LIB_DIR})

if (Boost_FOUND)
  include_directories (SYSTEM ${Boost_INCLUDE_DIR})  
  add_definitions(-Wall -O2)
  #add_executable(${PROJECT_NAME} ${CPPS})
endif(Boost_FOUND)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${OpenCV_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${OpenCV_MAIN_PATH} ./include)  

file(GLOB CPPS "*.cpp")
# "Add an executable to the project using the specified source files."
#add_executable (${PROJECT_NAME} ${MAIN_FILE})
add_library(${PROJECT_NAME} SHARED ${CPPS})

#target_link_libraries (${PROJECT_NAME} ${Boost_LIBRARIES} ${OpenCV_LIBRARIES})
target_link_libraries (${PROJECT_NAME} ${USED_LIBS} ${OpenCV_LIBRARIES})
#install(TARGETS ${PROJECT_NAME} DESTINATION bin)
