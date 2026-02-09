cmake_minimum_required(VERSION 3.20)
project(cm-advanced-search LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Put outputs in build/ (handy for VS Code)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# Library (example)
add_library(repository 
    src/club_repository.cpp 
    src/staff_repository.cpp 
    src/first_name_repository.cpp
    src/index_repository.cpp)

# Executable
add_executable(cm-advanced-search src/main.cpp)
target_link_libraries(cm-advanced-search PRIVATE repository)
# add_executable(dat-probe src/dat_probe.cpp)
# add_executable(club-dat src/read_club_dat.cpp)
# add_executable(staff-dat src/read_staff_dat.cpp)
# add_executable(club-staff-dat src/read_club_staff.cpp)


# Warnings in Debug (optional)
if(CMAKE_BUILD_TYPE MATCHES "Debug")
  target_compile_options(cm-advanced-search PRIVATE -Wall -Wextra -Wpedantic)
  # target_compile_options(staff-dat PRIVATE -Wall -Wextra -Wpedantic)
  # target_compile_options(dat-probe PRIVATE -Wall -Wextra -Wpedantic)
  # target_compile_options(club-dat PRIVATE -Wall -Wextra -Wpedantic)
  # target_compile_options(club-staff-dat PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(repository PRIVATE -Wall -Wextra -Wpedantic)
endif()
