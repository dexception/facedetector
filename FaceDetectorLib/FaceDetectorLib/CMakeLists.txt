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

if (WIN32 OR WIN64)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-DDETECTOR_DLL_EXPORTS)
endif()

if (UNIX)
    set(BOOST_ROOT "/usr/local")
    set(BOOST_INCLUDE_DIR "${BOOST_ROOT}/include/boost")
    set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib")
    set(Boost_ALL_DYN_LINK ON)
    add_definitions(-DBOOST_LOG_DYN_LINK=1)
endif(UNIX)
# else for windows assign through command line value of BOOST_LIBRARYDIR variable 

set(Boost_USE_MULTITHREADED ON)

if (WIN32 OR WIN64)
  set(Boost_USE_STATIC_LIBS ON)
endif(WIN32 OR WIN64)

set(Boost_USE_STATIC_RUNTIME OFF)

#find_package(Boost 1.60.0 COMPONENTS system filesystem log_setup log thread regex REQUIRED)
find_package(Boost 1.60.0 COMPONENTS system filesystem log_setup log thread REQUIRED)

#set(OpenCV_MAIN_PATH "/home/ruslan/opencv/build")
#set(OpenCV_DIR "${OpenCV_MAIN_PATH}")
#SET(OpenCV_LIB_DIR "${OpenCV_MAIN_PATH}/build/lib")

find_package(OpenCV REQUIRED)

LINK_DIRECTORIES(${OpenCV_LIB_DIR})

if (Boost_FOUND)
  include_directories (SYSTEM ${Boost_INCLUDE_DIR})  
  #add_definitions(-Wall -O2)  
  add_definitions(-Wall)  
  include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${OpenCV_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${OpenCV_MAIN_PATH} ./include)

  file(GLOB CPPS "*.cpp")
  
  add_library(${PROJECT_NAME} SHARED ${CPPS})
  target_link_libraries (${PROJECT_NAME} ${USED_LIBS} ${OpenCV_LIBRARIES} ${Boost_LIBRARIES})
  #target_link_libraries (${PROJECT_NAME}  ${Boost_SYSTEM_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${Boost_LOG_LIBRARY} ${Boost_LOG_SETUP_LIBRARY} ${Boost_THREAD_LIBRARY})

endif(Boost_FOUND)
