cmake_minimum_required(VERSION 3.10)

project(CCraft C)

add_executable(
    CCraft
    src/glmath.c
    src/util.c
    src/mesh.c
    src/block.c
    src/chunk.c
    src/world.c
    src/game.c
    src/main.c
    deps/glad/src/glad.c
    deps/lodepng/lodepng.c)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "Build example apps" FORCE)

add_subdirectory(deps/glfw)
include_directories(deps/glad/include)
include_directories(deps/glfw/include)
include_directories(deps/lodepng)

if(WIN32)
    target_link_libraries(CCraft opengl32)
elseif(APPLE)
    target_link_libraries(CCraft
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework OpenGL"
    )
elseif(UNIX)
    target_link_libraries(CCraft GL)
endif()

target_link_libraries(CCraft glfw ${GLFW_LIBRARIES})

add_custom_command(TARGET CCraft POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:CCraft>/res)