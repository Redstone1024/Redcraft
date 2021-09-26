cmake_minimum_required (VERSION 3.8)

# Set module name.
set (MODULE_NAME "Redcraft.Core")

# Add target.
file (GLOB_RECURSE MODULE_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/Source/*")
add_library (${MODULE_NAME} ${MODULE_SOURCE_FILES})
target_compile_definitions (${MODULE_NAME} PRIVATE "MODULE_NAME=${MODULE_NAME}")

# Add include directories.
target_include_directories (${MODULE_NAME} PUBLIC "Source/Public")
target_include_directories (${MODULE_NAME} PRIVATE "Source/Private")

# Define API macro.
string (TOUPPER ${MODULE_NAME} MODULE_API)
string (REGEX REPLACE "[^A-Z ^0-1]" "" MODULE_API ${MODULE_API})
set (MODULE_API "${MODULE_API}_API")
target_compile_definitions (${MODULE_NAME} PRIVATE "${MODULE_API}=DLLEXPORT")
target_compile_definitions (${MODULE_NAME} INTERFACE "${MODULE_API}=DLLIMPORT")

# Add project dependencies
#target_link_libraries (${MODULE_NAME} PRIVATE Redcraft.Core)
