cmake_minimum_required(VERSION 3.20.5)

project(csworld CXX)

set(CMAKE_CXX_STANDARD 23)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()


# Dependencies
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
#find_package(glm REQUIRED)
find_package(flatbuffers REQUIRED)
find_package(CURL REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread system)

# Define constants for directory paths
if(WIN32)
    set(APPLICATION_DATA_DIR "$ENV{APPDATA}/csworld")
elseif(APPLE)
    set(APPLICATION_DATA_DIR "$ENV{HOME}/Library/Application Support/csworld")
else()
    set(APPLICATION_DATA_DIR "$ENV{HOME}/.config/csworld")
endif()

# Sources
file(GLOB PROJECT_SOURCE_FILES
	 "*.cc"
	 "*.c"
)
set(projectSources
	${PROJECT_SOURCE_FILES}
)
file(GLOB PROJECT_SOURCE_FILES_SERVER "server/*.cc")
set(projectSourcesServer
	${PROJECT_SOURCE_FILES_SERVER}
)

file(GLOB_RECURSE CFILES "${CMAKE_SOURCE_DIR}/*.c")
SET_SOURCE_FILES_PROPERTIES(${CFILES} PROPERTIES LANGUAGE CXX )

# Generate flatbuffer headers 
add_custom_target(
	delete_generated_fbs 
	ALL
	COMMAND rm -f ${PROJECT_SOURCE_DIR}/fbs/*.h
)
add_custom_target(
	generate_fbs 
	ALL
	COMMAND flatc -o ${PROJECT_SOURCE_DIR}/fbs --cpp ${PROJECT_SOURCE_DIR}/fbs/*.fbs
	DEPENDS delete_generated_fbs
)

# Include dirs.
set(projectIncludeDirs
	${PROJECT_SOURCE_DIR}/fb
)
include_directories(${projectIncludeDirs})

# Configure a header file to pass some of the CMake settings
configure_file(config.h.in "${CMAKE_SOURCE_DIR}/config.h")

add_executable(client ${projectSources})
add_dependencies(client generate_fbs)
target_link_libraries(
	client PRIVATE
	glfw
	${GLEW_LIBRARIES}
	${OPENGL_LIBRARIES}
)

add_executable(server ${projectSourcesServer})
add_dependencies(server generate_fbs)
target_link_libraries(
	server PRIVATE
	${CURL_LIBRARIES}
	#glfw
	#${GLEW_LIBRARIES}
	#${OPENGL_LIBRARIES}
)

file(MAKE_DIRECTORY ${APPLICATION_DATA_DIR})

