cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0042 NEW)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Name of the project (will be the name of the plugin)
project(tkrzw-node)

add_definitions(-DNAPI_VERSION=9)
add_definitions(-DNODE_ADDON_API_ENABLE_TYPE_CHECK_ON_AS)
add_definitions(-DNODE_ADDON_API_CPP_EXCEPTIONS_ALL)

# Get all files under specified paths:
file(GLOB_RECURSE SOURCE_FILES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/*.cpp"
)

if( NOT (EXISTS "${CMAKE_SOURCE_DIR}/lib/libtkrzw.a") )
    message(STATUS "Couldn't find Tkrzw library under ${CMAKE_SOURCE_DIR}/lib/! Let's build the library...")

    execute_process(COMMAND "chmod" "+x" "dependency-builder.sh"
			WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/"
			COMMAND_ECHO STDOUT)
			
	execute_process(COMMAND "./dependency-builder.sh"
			WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/"
			COMMAND_ECHO STDOUT)
else()
    message(STATUS "Found Tkrzw library under '${CMAKE_SOURCE_DIR}/lib/'! proceeding...")
endif()


# Essential include files to build a node addon,
# You should add this line in every CMake.js based project
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
# Gives our library file a .node extension without any "lib" prefix
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
include_directories(${CMAKE_JS_INC} ${CMAKE_SOURCE_DIR}/lib/include ${CMAKE_SOURCE_DIR}/include)
target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/lib)

# Essential library files to link to a node addon
# You should add this line in every CMake.js based project
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB} libtkrzw.a atomic pthread lz4)
