cmake_minimum_required(VERSION 3.10)
project(
  elf
  VERSION 0.3.0
  LANGUAGES C CXX)

add_library(
  dwarf STATIC
  dwarf/abbrev.cc
  dwarf/attrs.cc
  dwarf/cursor.cc
  dwarf/die.cc
  dwarf/dwarf.cc
  dwarf/elf.cc
  dwarf/expr.cc
  dwarf/line.cc
  dwarf/rangelist.cc
  dwarf/value.cc
  dwarf/to_string.cc)
target_include_directories(dwarf PUBLIC dwarf/)

add_library(elf STATIC elf/elf.cc elf/to_string.cc elf/mmap_loader.cc)
target_include_directories(elf PUBLIC elf/)
target_link_libraries(elf PUBLIC dwarf)
