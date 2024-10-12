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

function(escape_backslashes input_path output_path)
    string(REPLACE "\\" "\\\\" escaped_path "${input_path}")
    set(${output_path} "${escaped_path}" PARENT_SCOPE)
endfunction()

# Determine the platform.
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set(OS_MAC 1)
    set(OS_MACOSX 1)  # For backwards compatibility.
    set(OS_POSIX 1)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    set(OS_LINUX 1)
    set(OS_POSIX 1)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    set(OS_WINDOWS 1)
else()
    message(FATAL_ERROR "Unsupported target platform")
endif()

# Set application data directory
# Obviously this won't work in production
if(OS_WINDOWS)
    set(APPLICATION_DATA_DIR "$ENV{APPDATA}\\csworld\\")
    escape_backslashes("${APPLICATION_DATA_DIR}" APPLICATION_DATA_DIR)
elseif(OS_MAC)
    set(APPLICATION_DATA_DIR "$ENV{HOME}/Library/Application Support/csworld/")
elseif(OS_LINUX)
    set(APPLICATION_DATA_DIR "$ENV{HOME}/.config/csworld/")
endif()
configure_file(common/config.h.in "${CMAKE_SOURCE_DIR}/common/config.h")
file(MAKE_DIRECTORY ${APPLICATION_DATA_DIR})
 
# Sources for common
file(GLOB PROJECT_SOURCE_FILES_COMMON
    "common/*.cc"
    "ext/*.c"
)
add_library(common STATIC ${PROJECT_SOURCE_FILES_COMMON})
target_include_directories(common PUBLIC
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/ext
)

# CEF
set(CEF_USE_SANDBOX OFF)
include(${CMAKE_SOURCE_DIR}/ext/cef-cmake/cmake/cef_cmake.cmake)
add_subdirectory(${CMAKE_SOURCE_DIR}/ext/cef-cmake)
set(CEF_SUBPROCESS_NAME "cef_subprocess")
if (OS_WINDOWS)
    set(CEF_SUBPROCESS_NAME_WITH_EXT "${CEF_SUBPROCESS_NAME}.exe")
else()
    set(CEF_SUBPROCESS_NAME_WITH_EXT "${CEF_SUBPROCESS_NAME}")
endif()
file(GLOB PROJECT_SOURCE_FILES_CEF_SUBPROCESS
    "cef_subprocess/*.cc"
    "client/src/UI/*.cc"
)
set(projectSourcesCefSubprocess
	${PROJECT_SOURCE_FILES_CEF_SUBPROCESS}
)
add_executable(cef_subprocess ${projectSourcesCefSubprocess})

# Sources for client
file(GLOB PROJECT_SOURCE_FILES_CLIENT
     "client/src/*.cc"
     "client/src/**/*.cc"
     "client/src/**/*.c" 
)
set(projectSources
	${PROJECT_SOURCE_FILES_CLIENT}
)
add_executable(client ${projectSources})

# Sources for server
file(GLOB PROJECT_SOURCE_FILES_SERVER 
    "server/src/*.cc"
)
set(projectSourcesServer
	${PROJECT_SOURCE_FILES_SERVER}
)
add_executable(server ${projectSourcesServer})

# Compile C files as CPP
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
    ${CMAKE_SOURCE_DIR}/client/src
    ${CMAKE_SOURCE_DIR}/ext
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/fbs    
)
target_include_directories(cef_subprocess PRIVATE
    ${CMAKE_SOURCE_DIR}/client/src/UI
    ${CMAKE_SOURCE_DIR}/ext
)
target_include_directories(server PRIVATE
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/fbs
)

target_link_libraries(client PRIVATE
    common
    glfw            
    ${GLEW_LIBRARIES}
    ${OPENGL_LIBRARIES}
    SQLite::SQLite3
    cefdll_wrapper
)
target_link_libraries(cef_subprocess PRIVATE
    cefdll_wrapper
)
target_link_libraries(server PRIVATE
    common
    CURL::libcurl
)

target_compile_definitions(server PRIVATE
    ASIO_HAS_BOOST_BIND
)
target_compile_definitions(cef_subprocess PRIVATE
    UNICODE
)
target_compile_definitions(client PRIVATE
    GLM_FORCE_LEFT_HANDED
    GLM_ENABLE_EXPERIMENTAL
    ASIO_HAS_BOOST_BIND
    UNICODE
    CEF_SUBPROCESS_NAME="${CEF_SUBPROCESS_NAME}"
    CEF_SUBPROCESS_NAME_WITH_EXT="${CEF_SUBPROCESS_NAME_WITH_EXT}"
)

if(OS_WINDOWS AND MSVC)
    set_target_properties(client PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
    set_target_properties(server PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
    set_target_properties(cef_subprocess PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
endif()

