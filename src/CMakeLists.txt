# include_directories(${CMAKE_SOURCE_DIR}/third_party/entt)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/sdl2)

# SDL2
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_gfx REQUIRED)

set(SOURCE_FILES main.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} SDL2::Main SDL2::Image SDL2::GFX EnTT::EnTT)