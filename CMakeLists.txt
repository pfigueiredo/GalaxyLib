cmake_minimum_required(VERSION 3.16)
project(GalaxyLib VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(GALAXYLIB_ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets" CACHE PATH "Directory containing Galaxy.bin")

add_library(galaxylib
    src/galaxy_api.cpp
    src/galaxy_map.cpp
    src/star_date.cpp
    src/sector.cpp
    src/coords.cpp
    src/known_space.cpp
    src/known_space_catalog.cpp
    src/base36.cpp
    src/star_system.cpp
    src/star_system_tables.cpp
    src/space_port_tables.cpp
    src/space_port.cpp
    src/system_body.cpp
    src/resource.cpp
    src/stc.cpp
)
target_include_directories(galaxylib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
if(MSVC)
    target_compile_options(galaxylib PRIVATE /utf-8)
endif()

add_executable(galaxy_test apps/galaxy_test/main.cpp)
target_link_libraries(galaxy_test PRIVATE galaxylib)
target_compile_definitions(galaxy_test PRIVATE
    GALAXYLIB_ASSETS_DIR="${GALAXYLIB_ASSETS_DIR}"
)

add_executable(stc_test apps/stc_test/main.cpp)
target_link_libraries(stc_test PRIVATE galaxylib)
target_compile_definitions(stc_test PRIVATE
    GALAXYLIB_ASSETS_DIR="${GALAXYLIB_ASSETS_DIR}"
)

if(MSVC)
    target_compile_options(galaxy_test PRIVATE /utf-8)
    target_compile_options(stc_test PRIVATE /utf-8)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/Galaxy.bin")
    add_custom_command(TARGET galaxy_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/assets/Galaxy.bin"
            "$<TARGET_FILE_DIR:galaxy_test>/Galaxy.bin"
        COMMENT "Copy Galaxy.bin next to galaxy_test for IDE/local runs")
    
    add_custom_command(TARGET stc_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/assets/Galaxy.bin"
            "$<TARGET_FILE_DIR:stc_test>/Galaxy.bin"
        COMMENT "Copy Galaxy.bin next to stc_test for IDE/local runs")
endif()
