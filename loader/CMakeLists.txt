cmake_minimum_required( VERSION 3.7 )

if( NOT WIN32 )
 message( FATAL_ERROR "This program can be compiled only for Windows platform." )
endif()

# Find toolchain path to 'windows.h'...
find_path( TC_INCLUDE_PATH "windows.h" )

project( loader CXX )

add_executable( loader loader.cpp SCM.hpp device.hpp loader.rc )

# With exceptions enabled, GCC will force to compile with pthreads and use GetTickCount64 which is not available in Windows XP.
target_compile_options    ( loader PRIVATE -Wall -Wextra -fno-exceptions -fno-rtti )
target_link_options       ( loader PRIVATE -s -static )
set_target_properties     ( loader PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR} )
target_include_directories( loader PRIVATE ${TC_INCLUDE_PATH}/ddk )
