cmake_minimum_required( VERSION 3.7 )

if( NOT WIN32 )
 message( FATAL_ERROR "This program can be compiled only for Windows platform." )
endif()

project( loader )

add_executable( loader loader.cpp loader.rc )

target_link_options( loader PRIVATE -s )
set_target_properties( loader PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR} )
