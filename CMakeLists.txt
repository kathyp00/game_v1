cmake_minimum_required(VERSION 3.25)
project(MyGame CXX)

set(CMAKE_CXX_STANDARD 20)
set(MY_FRAMEWORK_DIR "$ENV{HOME}/Documents/sdl_framework")

add_executable(MyGame
    main.cpp
    animation.h
    timer.h
    game_object.h
)

target_compile_options(MyGame PRIVATE -Wno-switch)

add_custom_command(TARGET MyGame PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Starting fresh build step..."
)

target_compile_options(MyGame PRIVATE "-F${MY_FRAMEWORK_DIR}")
target_link_options(MyGame PRIVATE "-F${MY_FRAMEWORK_DIR}")
target_link_options(MyGame PRIVATE "-Wl,-rpath,${MY_FRAMEWORK_DIR}")

target_link_libraries(MyGame PRIVATE
    "-framework SDL3"
    "-framework SDL3_image"
    "-framework SDL3_mixer"
    "-framework SDL3_ttf"
)

include(FetchContent)
FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG 	0af55ccecd98d4e5a8d1fad7de25ba429d60e863 #refs/tags/1.0.1
)
FetchContent_MakeAvailable(glm)
target_link_libraries(MyGame PRIVATE glm::glm)