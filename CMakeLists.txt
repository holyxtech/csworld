cmake_minimum_required(VERSION 3.20.5)

project(csworld CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(flatbuffers CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(Boost CONFIG REQUIRED COMPONENTS bind)
find_package(asio CONFIG REQUIRED)

# Helper function to escape backslashes in Windows paths
function(escape_backslashes input_path output_path)
    string(REPLACE "\\" "\\\\" escaped_path "${input_path}")
    set(${output_path} "${escaped_path}" PARENT_SCOPE)
endfunction()

if(WIN32)
    set(APPLICATION_DATA_DIR "$ENV{APPDATA}\\csworld")
    escape_backslashes("${APPLICATION_DATA_DIR}" APPLICATION_DATA_DIR)

    if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG")    
    endif()

elseif(APPLE)
    set(APPLICATION_DATA_DIR "$ENV{HOME}/Library/Application Support/csworld")
else()
    set(APPLICATION_DATA_DIR "$ENV{HOME}/.config/csworld")
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
    endif()
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
#add_custom_target(
#	delete_generated_fbs 
#	ALL
#	COMMAND rm -f ${PROJECT_SOURCE_DIR}/fbs/*.h
#)
#add_custom_target(
#	generate_fbs 
#	ALL
#	COMMAND flatc -o ${PROJECT_SOURCE_DIR}/fbs --cpp ${PROJECT_SOURCE_DIR}/fbs/*.fbs
#	DEPENDS delete_generated_fbs
#)

set(projectIncludeDirs
	${PROJECT_SOURCE_DIR}/fbs
)
include_directories(${projectIncludeDirs})

configure_file(config.h.in "${CMAKE_SOURCE_DIR}/config.h")

add_definitions(-D GLM_ENABLE_EXPERIMENTAL)

add_executable(client ${projectSources})
target_link_libraries(
    client PRIVATE
    glfw            
    ${GLEW_LIBRARIES}
    glm::glm         
    flatbuffers::flatbuffers      
    ${OPENGL_LIBRARIES}
)

add_executable(server ${projectSourcesServer})
target_link_libraries(
    server PRIVATE
    CURL::libcurl
    flatbuffers::flatbuffers
)

file(MAKE_DIRECTORY ${APPLICATION_DATA_DIR})

