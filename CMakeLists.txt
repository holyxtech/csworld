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
find_package(SQLite3 REQUIRED)
find_package(unofficial-nuklear CONFIG REQUIRED)

function(escape_backslashes input_path output_path)
    string(REPLACE "\\" "\\\\" escaped_path "${input_path}")
    set(${output_path} "${escaped_path}" PARENT_SCOPE)
endfunction()

if(WIN32)
    set(APPLICATION_DATA_DIR "$ENV{APPDATA}\\csworld\\")
    escape_backslashes("${APPLICATION_DATA_DIR}" APPLICATION_DATA_DIR)

    if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG")    
    endif()

elseif(APPLE)
    set(APPLICATION_DATA_DIR "$ENV{HOME}/Library/Application Support/csworld/")
else()
    set(APPLICATION_DATA_DIR "$ENV{HOME}/.config/csworld/")
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
    endif()
endif()

# Sources
file(GLOB PROJECT_SOURCE_FILES_CLIENT
	 "client/*.cc"
	 "client/*.c"
     "common/common.cc"
)
set(projectSources
	${PROJECT_SOURCE_FILES_CLIENT}
)
file(GLOB PROJECT_SOURCE_FILES_SERVER 
    "server/*.cc"
    "common/common.cc"
)
set(projectSourcesServer
	${PROJECT_SOURCE_FILES_SERVER}
)

file(GLOB_RECURSE CFILES "${CMAKE_SOURCE_DIR}/*.c")
SET_SOURCE_FILES_PROPERTIES(${CFILES} PROPERTIES LANGUAGE CXX )

# Generate flatbuffer headers 
file(GLOB FBS_FILES ${PROJECT_SOURCE_DIR}/fbs/*.fbs)
set(GENERATED_FILES ${PROJECT_SOURCE_DIR}/fbs/generated_fbs.stamp)

add_custom_command(
    OUTPUT ${GENERATED_FILES}
    COMMAND flatc -o ${PROJECT_SOURCE_DIR}/fbs --cpp ${FBS_FILES}
    COMMAND ${CMAKE_COMMAND} -E touch ${GENERATED_FILES} # touch a file to mark as up-to-date
    DEPENDS ${FBS_FILES}
    COMMENT "Generating FlatBuffers files"
)

add_custom_target(
    generate_fbs
    ALL
    DEPENDS ${GENERATED_FILES}
)

include_directories(${CMAKE_SOURCE_DIR}/ext/cy)
include_directories(${CMAKE_SOURCE_DIR}/common)
include_directories(${CMAKE_SOURCE_DIR}/fbs)

# Obviously this won't work in production
configure_file(common/config.h.in "${CMAKE_SOURCE_DIR}/common/config.h")

add_executable(client ${projectSources})
add_dependencies(client generate_fbs)
target_link_libraries(
    client PRIVATE
    glfw            
    ${GLEW_LIBRARIES}
    ${OPENGL_LIBRARIES}
    SQLite::SQLite3
)

add_executable(server ${projectSourcesServer})
add_dependencies(server generate_fbs)
target_link_libraries(
    server PRIVATE
    CURL::libcurl
)

file(MAKE_DIRECTORY ${APPLICATION_DATA_DIR})

add_definitions(-D ASIO_HAS_BOOST_BIND)
target_compile_definitions(client PRIVATE GLM_FORCE_LEFT_HANDED)
target_compile_definitions(client PRIVATE GLM_ENABLE_EXPERIMENTAL)