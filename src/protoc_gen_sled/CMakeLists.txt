cmake_minimum_required(VERSION 3.10)

project(protoc-gen-sled LANGUAGES C CXX)
add_executable(${PROJECT_NAME} main.cc sled_generator.cc)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE libprotoc sled)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
