cmake_minimum_required(VERSION 3.25)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v6.1.9
)

FetchContent_MakeAvailable(ftxui)

project(solitairecpp)

add_executable(solitairecpp
    ./src/main.cpp
    ./src/solitairecpp/solitaire.cpp
    ./src/solitairecpp/cards.cpp
    ./src/solitairecpp/board.cpp
    ./src/solitairecpp/move_manager.cpp
)

target_include_directories(solitairecpp
    PRIVATE ./include
)

target_compile_options(solitairecpp
    PRIVATE -std=c++23 -g
)

if (LINUX)
    target_link_libraries(solitairecpp PRIVATE
        ftxui::component
        ftxui::dom
        ftxui::screen
        atomic
    )
else()
    target_link_libraries(solitairecpp PRIVATE
        ftxui::component
        ftxui::dom
        ftxui::screen
        atomic
    )
endif()
