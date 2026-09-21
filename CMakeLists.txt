cmake_minimum_required(VERSION 3.25)
project(MyGame CXX)

set(CMAKE_CXX_STANDARD 20)
set(MY_FRAMEWORK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../sdl_framework")

add_executable(MyGame
    main.cpp
    Game.cpp
    Animation.h
    Timer.h
    Game.h
    GameObject.h
    GameState.h
    Resources.h
    SDLState.h
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
	GIT_TAG 	1.0.3
)
FetchContent_MakeAvailable(glm)
target_link_libraries(MyGame PRIVATE glm::glm)
