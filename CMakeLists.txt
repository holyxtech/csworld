cmake_minimum_required(VERSION 3.20.5)

project(CsWorld CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set (BUILD_SHARED_LIBS OFF)
#set(CMAKE_DEBUG_POSTFIX "")

# Dependencies
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(flatbuffers REQUIRED)
find_package(tsl-robin-map REQUIRED)
find_package(CURL REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread system)

# Sources
file(GLOB PROJECT_SOURCE_FILES "*.cc")
set(projectSources
	${PROJECT_SOURCE_FILES}
)
file(GLOB PROJECT_SOURCE_FILES_SERVER "server/*.cc")
set(projectSourcesServer
	#${projectSources}
	${PROJECT_SOURCE_FILES_SERVER}
)

# Include dirs.
set(projectIncludeDirs
	${THIRD_PARTY_DIR}/tsl
	${PROJECT_SOURCE_DIR}/fb
)
include_directories(${projectIncludeDirs})

add_executable(CsWorldClient ${projectSources})
target_link_libraries(
	CsWorldClient PRIVATE
	glfw
	${GLEW_LIBRARIES}
	${OPENGL_LIBRARIES}
)


add_executable(CsWorldServer ${projectSourcesServer})
target_link_libraries(
	CsWorldServer PRIVATE
	${CURL_LIBRARIES}
	#glfw
	#${GLEW_LIBRARIES}
	#${OPENGL_LIBRARIES}
)
