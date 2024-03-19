cmake_minimum_required(VERSION 3.20)
project(helloworld LANGUAGES CXX)

# TODO: make path to agent conda env configurable
set(agent_env_path "./agent/.conda/collaborative-learning-dev")

# python + pybind11 configuration
#
# Must use the same python environment used by the agent
# Pybind11 must be unstalled in the agent environment
set(Python_ROOT_DIR ${agent_env_path})
set(pybind11_DIR "${agent_env_path}/lib/python3.11/site-packages/pybind11/share/cmake/pybind11/")
find_package (Python COMPONENTS Interpreter Development)
find_package(pybind11 REQUIRED)

set(sources 
    main.cc
)

add_executable(node ${sources})
target_link_libraries(node PRIVATE pybind11::embed)
