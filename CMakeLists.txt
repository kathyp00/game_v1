cmake_minimum_required(VERSION 3.25)
project(MyGame CXX)

set(CMAKE_CXX_STANDARD 20)
set(MY_FRAMEWORK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../sdl_framework")

add_executable(MyGame
    main.cpp
    src/Game.cpp
    src/Level.cpp
    src/Game.h
    src/Animation.h
    src/Timer.h
    src/GameObject.h
    src/GameState.h
    src/Resources.h
    src/SDLState.h
    src/Level.h
    src/GameInfo.h
)

target_include_directories(MyGame PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
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

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)
target_link_libraries(MyGame PRIVATE nlohmann_json::nlohmann_json)
