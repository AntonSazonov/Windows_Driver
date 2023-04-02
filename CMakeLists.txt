cmake_minimum_required( VERSION 3.7 )

if( NOT WIN32 )
 message( FATAL_ERROR "This program can be compiled only for Windows platform." )
endif()

project( Driver_Example )

add_subdirectory( driver )
add_subdirectory( loader )
