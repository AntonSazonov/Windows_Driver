cmake_minimum_required( VERSION 3.7 )

if( NOT WIN32 )
 message( FATAL_ERROR "This program can be compiled only for Windows platform." )
endif()

set( PATH_SDK "C:/Program Files (x86)/Windows Kits/10/bin/10.0.22621.0/x64" )
set( PATH_DDK "C:/msys2/ucrt64/include/ddk" )

message( STATUS "PATH_SDK: ${PATH_SDK}" )
#file( TO_CMAKE_PATH ${PATH_SDK} ${PATH_SDK} )

find_program( SIGNTOOL signtool PATHS ${PATH_SDK} NO_DEFAULT_PATH NO_PACKAGE_ROOT_PATH NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_CMAKE_INSTALL_PREFIX NO_CMAKE_FIND_ROOT_PATH )
if( NOT SIGNTOOL )
 message( FATAL_ERROR "SignTool not found. Check your Windows SDK path." )
endif()

message( STATUS "SignTool: ${SIGNTOOL}" )

project( simple_driver )

# CMAKE_CXX_COMPILER_ID is set only after project was defined.
#if( CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )
#elseif( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
#else()
# message( FATAL_ERROR "This program can be compiled only with GNU or Clang toolchain." )
#endif()

add_executable( simple_driver simple_driver.cpp )

set( CMAKE_C_FLAGS "" )					# no default C flags
set( CMAKE_CXX_FLAGS "" )				# no default C++ flags
set( CMAKE_CXX_STANDARD_LIBRARIES "" )	# no default libraries

target_compile_options    ( simple_driver PRIVATE -Wall -Wextra -Wno-sign-compare -Wno-strict-aliasing -fPIC -fvisibility=hidden -ffunction-sections -fdata-sections -fno-builtin -ffreestanding -fno-stack-protector -mno-stack-arg-probe )
target_link_options       ( simple_driver PRIVATE -Wl,--subsystem,native,--image-base,0x140000000,--dynamicbase,--nxcompat,--file-alignment,0x200,--section-alignment,0x1000,--stack,0x100000,--gc-sections,--exclude-all-symbols,--entry,DriverEntry -nostartfiles -nodefaultlibs -nostdlib -s )
set_target_properties     ( simple_driver PROPERTIES SUFFIX ".sys" )
set_target_properties     ( simple_driver PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR} )
target_compile_definitions( simple_driver PRIVATE __INTRINSIC_DEFINED_InterlockedBitTestAndSet __INTRINSIC_DEFINED_InterlockedBitTestAndReset )
target_include_directories( simple_driver PRIVATE ${PATH_DDK} )
target_link_libraries     ( simple_driver PRIVATE ntoskrnl hal )

# Sign driver
add_custom_command( TARGET simple_driver POST_BUILD
	COMMAND ${SIGNTOOL} sign /tr http://timestamp.digicert.com /td SHA256 /fd SHA256 /v /f ${CMAKE_CURRENT_SOURCE_DIR}/OEM_Test_Cert_2017.pfx $<TARGET_FILE:simple_driver>
)
