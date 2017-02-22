
project (liberation-circuit)
cmake_minimum_required (VERSION 3.4)

# Set binary output directory
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

# Get source files
file (GLOB_RECURSE SOURCE_FILES "src/*.c"
                                "src/*.h")

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")
add_executable (liberation-circuit ${SOURCE_FILES})

# Base library
find_package (Allegro REQUIRED)

if (ALLEGRO_FOUND)

    include_directories (${ALLEGRO_INCLUDE_DIR})
    target_link_libraries (liberation-circuit ${ALLEGRO_LIBRARY})

else ()
    message (FATAL_ERROR "Could not find Allegro")
endif ()
