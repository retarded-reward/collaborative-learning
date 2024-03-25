cmake_minimum_required(VERSION 3.20)
cmake_policy(VERSION 3.20)
project(collaborative-learning LANGUAGES CXX)

# Change "cmake" if you put the submodule in a different directory
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package(OmnetPP 6.0 REQUIRED)

# Load the CMake commands for OMNeT++
include(OmnetppHelpers)

add_subdirectory(simulations simulations/bin)