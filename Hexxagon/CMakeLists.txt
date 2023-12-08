cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 23)
set(BUILD_SHARED_LIBS FALSE)#!!
include(FetchContent)

add_executable (Hexxagon "Hexxagon.cpp" "Hexxagon.h" "HexxagonAI.h" "GameBoard.h" "GameBoard.cpp" "HexxagonAI.cpp" "ExtendedAssets.h" "ExtendedAssets.cpp")

FETCHCONTENT_DECLARE(
        SFML
        GIT_REPOSITORY
        https://github.com/SFML/SFML.git
)
FETCHCONTENT_MAKEAVAILABLE(SFML)

target_link_libraries(Hexxagon
        sfml-system
        sfml-window
        sfml-graphics)
