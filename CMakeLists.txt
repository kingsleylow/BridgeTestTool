cmake_minimum_required(VERSION 3.0.0)

project(BridgeTest)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4267")

aux_source_directory(${CMAKE_SOURCE_DIR} SRC_LIST)

file(GLOB headers ${CMAKE_SOURCE_DIR}/*.h)
message(STATUS ${CMAKE_SOURCE_DIR}})
message(STATUS ${SRC_LIST})

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME} ${SRC_LIST} ${headers})
target_link_libraries(${PROJECT_NAME} ws2_32)

if(${CMAKE_BUILD_TYPE} MATCHES "Release")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(OUT "Release")
else()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(OUT "Debug")
endif()

file(GLOB libfile ${CMAKE_SOURCE_DIR}/config/*)
foreach(lib ${libfile})
  message(STATUS "copy file..." ${lib})
  file(COPY ${lib} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${OUT}/config)
endforeach()

file(GLOB libfile ${CMAKE_SOURCE_DIR}/mt4lib/*)
foreach(lib ${libfile})
  message(STATUS "copy file..." ${lib})
  file(COPY ${lib} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${OUT}/mt4lib)
endforeach()

set(DIR_NAME "logs")
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${OUT}/${DIR_NAME})