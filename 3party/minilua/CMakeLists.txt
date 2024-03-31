cmake_minimum_required(VERSION 3.10)
project(
  minilua
  VERSION 5.4.6
  LANGUAGES C CXX)
add_library(minilua STATIC src/minilua.c)
target_include_directories(minilua PUBLIC src)
