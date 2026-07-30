cmake_minimum_required(VERSION 3.11)
project(RoadFighterGame C)

set(CMAKE_C_STANDARD 99)

add_executable(game 
    main.c 
    game.c 
    ui.c
    audiomanager.c 
)

target_include_directories(game PRIVATE .)

target_link_libraries(game PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/libraylib.a
    opengl32 
    gdi32 
    winmm
)