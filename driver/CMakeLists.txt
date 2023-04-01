cmake_minimum_required( VERSION 3.7 )

if( NOT WIN32 )
 message( FATAL_ERROR "This program can be compiled only for Windows platform." )
endif()


set( PATH_SDK "C:/Program Files (x86)/Windows Kits/10/bin/10.0.22621.0/x64" )
set( PATH_DDK "C:/msys2/ucrt64/include/ddk" )

message( STATUS "PATH_SDK: ${PATH_SDK}" )
#file( TO_CMAKE_PATH ${PATH_SDK} ${PATH_SDK} )


find_program( SIGNTOOL signtool PATHS ${PATH_SDK} NO_DEFAULT_PATH NO_PACKAGE_ROOT_PATH NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH )
#find_program( SIGNTOOL signtool PATHS ${PATH_SDK} NO_SYSTEM_ENVIRONMENT_PATH )
if( NOT SIGNTOOL )
 message( FATAL_ERROR "SignTool not found!" )
endif()

message( STATUS "SignTool: ${SIGNTOOL}" )





project( simple_driver )

message( STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}" )

#[[
if ( CMAKE_CXX_COMPILER_ID NOT STREQUAL "Clang" AND CMAKE_CXX_COMPILER_ID NOT STREQUAL "GNU" )
#if ( CMAKE_CXX_COMPILER_ID NOT MATCHES "Clang" AND CMAKE_CXX_COMPILER_ID NOT MATCHES "GNU" )
 message( FATAL_ERROR "This program can be compiled only by GNU or Clang toolchain." )
endif()
]]

add_executable( simple_driver simple_driver.cpp )

set( CMAKE_CXX_FLAGS "" )
set( CMAKE_CXX_STANDARD_LIBRARIES "" )

target_compile_options    ( simple_driver PRIVATE -Wall -Wextra -Wno-sign-compare -Wno-strict-aliasing -fPIC -fvisibility=hidden -ffunction-sections -fdata-sections -fno-builtin -ffreestanding -fno-stack-protector -mno-stack-arg-probe )
target_link_options       ( simple_driver PRIVATE -Wl,--subsystem,native,--image-base,0x140000000,--dynamicbase,--nxcompat,--file-alignment,0x200,--section-alignment,0x1000,--stack,0x100000,--gc-sections,--exclude-all-symbols,--entry,DriverEntry -nostartfiles -nodefaultlibs -nostdlib -s )
set_target_properties     ( simple_driver PROPERTIES SUFFIX ".sys" )
set_target_properties     ( simple_driver PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR} )
target_compile_definitions( simple_driver PRIVATE __INTRINSIC_DEFINED_InterlockedBitTestAndSet __INTRINSIC_DEFINED_InterlockedBitTestAndReset )
target_include_directories( simple_driver PRIVATE ${PATH_DDK} )
target_link_libraries     ( simple_driver PRIVATE ntoskrnl hal )

# Sign driver
add_custom_command( TARGET simple_driver POST_BUILD
	#COMMAND echo $<TARGET_FILE:simple_driver>
	COMMAND ${SIGNTOOL} sign /ph /fd "sha256" /sha1 "670A7019D58F3BC17BB1FDEFA39D31988D1E3EE2" $<TARGET_FILE:simple_driver>
)



#[[
message( STATUS "IMPORTED_LINK_DEPENDENT_LIBRARIES: ${IMPORTED_LINK_DEPENDENT_LIBRARIES}" )
message( STATUS "IMPORTED_LINK_INTERFACE_LIBRARIES: ${IMPORTED_LINK_INTERFACE_LIBRARIES}" )
message( STATUS "         INTERFACE_LINK_LIBRARIES: ${INTERFACE_LINK_LIBRARIES}" )
message( STATUS "  INTERFACE_LINK_LIBRARIES_DIRECT: ${INTERFACE_LINK_LIBRARIES_DIRECT}" )
message( STATUS "         LINK_INTERFACE_LIBRARIES: ${LINK_INTERFACE_LIBRARIES}" )
message( STATUS "                   LINK_LIBRARIES: ${LINK_LIBRARIES}" )
message( STATUS "      LINK_LIBRARIES_ONLY_TARGETS: ${LINK_LIBRARIES_ONLY_TARGETS}" )
]]




#add_custom_command( OUTPUT ${CMAKE_PROJECT_NAME}
#	COMMAND "gcc -Wall -Wextra -Wno-sign-compare -Wno-strict-aliasing -m64 -fPIC -fvisibility=hidden -ffunction-sections -fdata-sections -fno-builtin -ffreestanding -fno-stack-protector -mno-stack-arg-probe -I/d/Dev_Source/mingw-w64-dpp-main/CRT -IC:/msys2/ucrt64/include/ddk -D__INTRINSIC_DEFINED_InterlockedBitTestAndSet -D__INTRINSIC_DEFINED_InterlockedBitTestAndReset -c simple_driver.c -o simple_driver.o"
#	)

#[[
add_custom_target(
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/res
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/assets
	COMMAND aapt package --no-crunch -f -0 apk -M ${MANIFEST} -I ${JAR} -S ${CMAKE_SOURCE_DIR}/res -A ${CMAKE_SOURCE_DIR}/assets -F ${APK}.apk --generate-dependencies
	COMMAND ${EXE_READELF} --dynamic lib/${CMAKE_ANDROID_ARCH_ABI}/$<TARGET_FILE_NAME:${CMAKE_PROJECT_NAME}> | grep NEEDED
	COMMAND aapt a ${APK}.apk lib/${CMAKE_ANDROID_ARCH_ABI}/$<TARGET_FILE_NAME:${CMAKE_PROJECT_NAME}>
	COMMAND zipalign -f -v -p 4 ${APK}.apk ${APK}-aligned.apk
	COMMAND apksigner.bat sign --ks ${CMAKE_SOURCE_DIR}/../../my-release-key.jks --ks-pass pass:111111 ${APK}-aligned.apk
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
]]


#[[

#set( COMMON_FLAGS "-fomit-frame-pointer -fvisibility=hidden -fno-rtti" ) # -flto
#set( CMAKE_C_FLAGS "${COMMON_FLAGS}" )
#set( CMAKE_CXX_FLAGS "${COMMON_FLAGS} -std=c++17 -fno-exceptions" ) #

#set( CMAKE_C_FLAGS "-Wall -Wextra -Wno-sign-compare -Wno-strict-aliasing -m64 -fPIC -fvisibility=hidden -ffunction-sections -fdata-sections -fno-builtin -ffreestanding -fno-stack-protector -mno-stack-arg-probe -nostartfiles -nostdlib" )

# -fvisibility=hidden
set( CMAKE_C_FLAGS "-Wall -Wextra -ffunction-sections -fdata-sections -fno-builtin -ffreestanding -fno-stack-protector -mno-stack-arg-probe -nostartfiles -nostdlib -Wl,--entry,DriverEntry@8" )

target_link_options       ( ${CMAKE_PROJECT_NAME} PRIVATE -s -e DriverEntry ) # -s -static
set_target_properties     ( ${CMAKE_PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR} )
set_target_properties     ( ${CMAKE_PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR} )
target_link_libraries     ( ${CMAKE_PROJECT_NAME} PRIVATE ntoskrnl hal )
]]
