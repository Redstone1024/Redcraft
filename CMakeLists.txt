cmake_minimum_required (VERSION 3.8)

# Main project
project ("Redcraft")

# Reset the binary file directory
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Build")
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Build")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Binaries")

# Configure compile options
set (BUILD_SHARED_LIBS true)
set (CMAKE_CXX_STANDARD 20)

# Define platform macros
add_compile_definitions ("PLATFORM_NAME=${CMAKE_SYSTEM_NAME}")
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_compile_definitions ("PLATFORM_WINDOWS=1")
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_compile_definitions ("PLATFORM_LINUX=1")
else ()
	add_compile_definitions ("PLATFORM_UNKNOWN=1")
endif ()

# Define configuration type macros
add_compile_definitions ("BUILD_TYPE=${CMAKE_BUILD_TYPE}")
if (CMAKE_BUILD_TYPE MATCHES "Debug")
	add_compile_definitions ("BUILD_DEBUG=1")
elseif (CMAKE_BUILD_TYPE MATCHES "Release")
	add_compile_definitions ("BUILD_RELEASE=1")
else ()
	add_compile_definitions ("BUILD_UNKNOWN=1")
endif ()

# Add subproject  
file (GLOB PROJECT_FOLDERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/*")
foreach (PROJECT_SUBDIRECTORY ${PROJECT_FOLDERS}) 
	if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_SUBDIRECTORY}")
		file (GLOB PROJECT_CMAKELISTS "${PROJECT_SUBDIRECTORY}/CMakeLists.txt")
		if (NOT "${PROJECT_CMAKELISTS}" STREQUAL "")
			message (STATUS "Add subdirectory: " ${PROJECT_SUBDIRECTORY})
			add_subdirectory (${PROJECT_SUBDIRECTORY})
		endif ()
	endif ()
endforeach ()
