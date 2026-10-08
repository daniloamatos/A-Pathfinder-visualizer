cmake_minimum_required(VERSION 3.30)

project(PathfinderVisualization)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED sdl3)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/imgui)

add_executable(main
    main.cpp
    pathfinding.cpp
    
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp

    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp
)

target_include_directories(main PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${SDL3_INCLUDE_DIRS}
)

target_link_libraries(main PRIVATE
    ${SDL3_LIBRARIES}
)

include(CTest)
if(BUILD_TESTING)
    add_executable(pathfinding_tests tests/pathfinding_tests.cpp pathfinding.cpp)
    target_include_directories(pathfinding_tests PRIVATE ${CMAKE_SOURCE_DIR} ${SDL3_INCLUDE_DIRS})
    target_link_libraries(pathfinding_tests PRIVATE ${SDL3_LIBRARIES})
    add_test(NAME pathfinding_lifecycle COMMAND pathfinding_tests)
endif()
