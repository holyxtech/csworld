cmake_minimum_required(VERSION 3.20.5)

project(CsWorld CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
	${PROJECT_SOURCE_FILES_SERVER}
)

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

add_executable(CsWorldClient ${projectSources})
add_dependencies(CsWorldClient generate_fbs)
target_link_libraries(
	CsWorldClient PRIVATE
	glfw
	${GLEW_LIBRARIES}
	${OPENGL_LIBRARIES}
)


add_executable(CsWorldServer ${projectSourcesServer})
add_dependencies(CsWorldServer generate_fbs)
target_link_libraries(
	CsWorldServer PRIVATE
	${CURL_LIBRARIES}
	#glfw
	#${GLEW_LIBRARIES}
	#${OPENGL_LIBRARIES}
)
