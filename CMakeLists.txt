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

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(APPLICATION_DATA_DIR "$ENV{APPDATA}\\csworld\\")
    escape_backslashes("${APPLICATION_DATA_DIR}" APPLICATION_DATA_DIR)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(APPLICATION_DATA_DIR "$ENV{HOME}/Library/Application Support/csworld/")
else()
    set(APPLICATION_DATA_DIR "$ENV{HOME}/.config/csworld/")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
    endif()
endif()

# Sources
file(GLOB PROJECT_SOURCE_FILES_CLIENT
	 "client/*.cc"
	 "client/*.c"
     "common/*.cc"
     "common/*.cc"
     "client/GameObjects/*.cc"
)
set(projectSources
	${PROJECT_SOURCE_FILES_CLIENT}
)
add_executable(client ${projectSources})

file(GLOB PROJECT_SOURCE_FILES_SERVER 
    "server/*.cc"
    "common/*.cc"
)
set(projectSourcesServer
	${PROJECT_SOURCE_FILES_SERVER}
)
add_executable(server ${projectSourcesServer})

file(GLOB_RECURSE CFILES "${CMAKE_SOURCE_DIR}/*.c")
SET_SOURCE_FILES_PROPERTIES(${CFILES} PROPERTIES LANGUAGE CXX )

# Generate flatbuffer headers 
file(GLOB FBS_FILES ${PROJECT_SOURCE_DIR}/fbs/*.fbs)
set(GENERATED_FILES ${PROJECT_SOURCE_DIR}/fbs/generated_fbs.stamp)
add_custom_command(
    OUTPUT ${GENERATED_FILES}
    COMMAND flatc -o ${PROJECT_SOURCE_DIR}/fbs --cpp ${FBS_FILES}
    COMMAND ${CMAKE_COMMAND} -E touch ${GENERATED_FILES}
    DEPENDS ${FBS_FILES}
    COMMENT "Generating FlatBuffers files"
)
file(GLOB GENERATED_HEADERS ${PROJECT_SOURCE_DIR}/fbs/*_generated.h)
add_custom_target(
    generate_fbs
    DEPENDS ${GENERATED_HEADERS} ${GENERATED_FILES}
)

add_dependencies(client generate_fbs)
add_dependencies(server generate_fbs)

target_include_directories(client PRIVATE
    ${CMAKE_SOURCE_DIR}/ext/cy
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/fbs
)
target_include_directories(server PRIVATE
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/fbs
)

target_link_libraries(
    client PRIVATE
    glfw            
    ${GLEW_LIBRARIES}
    ${OPENGL_LIBRARIES}
    SQLite::SQLite3
)
target_link_libraries(
    server PRIVATE
    CURL::libcurl
)

target_compile_definitions(server PRIVATE
    ASIO_HAS_BOOST_BIND
)
target_compile_definitions(client PRIVATE
    GLM_FORCE_LEFT_HANDED
    GLM_ENABLE_EXPERIMENTAL
    ASIO_HAS_BOOST_BIND
)

# Obviously this won't work in production
configure_file(common/config.h.in "${CMAKE_SOURCE_DIR}/common/config.h")
file(MAKE_DIRECTORY ${APPLICATION_DATA_DIR})