# python + pybind11 configuration
#
# Must use the same python environment used by the agent
# Pybind11 must be unstalled in the agent environment
message(CMAKE_BINARY_DIR="${CMAKE_BINARY_DIR}")
set(agent_env_path ${PROJECT_SOURCE_DIR}/agent/.conda/collaborative-learning-dev)

set(Python_ROOT_DIR ${agent_env_path})
set(Python_EXECUTABLE ${agent_env_path}/bin/python)
set(pybind11_DIR ${agent_env_path}/lib/python3.11/site-packages/pybind11/share/cmake/pybind11/)
find_package (Python COMPONENTS Interpreter Development)
find_package(pybind11 REQUIRED)

# Omnet++ configuration

# Uncomment the following if you have external dependencies like INET
#find_path(INET_DIR NAMES src/inet/package.ned DOC "INET root directory")
#import_opp_target(inet ${INET_DIR}/src/Makefile)

# Define your library/simulation sources
set(SOURCES
    src/node/controller.cc
    src/node/agentc/agent_client.cc
    src/node/agentc/agent_client_pybind.cc
    src/node/agentc/python_interpreter.cc
)

add_library(project_library SHARED ${SOURCES})

# Define your messages as well
set(MESSAGE_SOURCES
    messages/ActionRequest.msg
    messages/NodeStateMsg.msg
    messages/SinkRewardMsg.msg
)

foreach(MESSAGE ${MESSAGE_SOURCES})

    # uncomment the following if you have messages
    generate_opp_message(
        ${MESSAGE}
        TARGET project_library
        OUTPUT_ROOT ${CMAKE_BINARY_DIR}/simulations
        DIRECTORY messages 
        GEN_SOURCES gen_sources
        # NOTE: this parameter does not work, thus include directory must be added manually.
        # see https://github.com/omnetpp/cmake?tab=readme-ov-file#caveats for details
        GEN_INCLUDE_DIR ${CMAKE_BINARY_DIR}/messages
        )
endforeach()

target_include_directories(project_library
 PUBLIC ${CMAKE_BINARY_DIR}/simulations/messages)

# You will need to tweak and add the additional properties for your project
set_target_properties(project_library
    PROPERTIES OUTPUT_NAME bin
    NED_FOLDERS src)

# Link the libraries you need for your project; add "inet" if necessary
target_link_libraries(project_library OmnetPP::header)
#target_link_libraries(project_library OmnetPP::cmdenv)
#target_link_libraries(project_library OmnetPP::common)
#target_link_libraries(project_library OmnetPP::envir)
#target_link_libraries(project_library OmnetPP::eventlog)
#target_link_libraries(project_library OmnetPP::layout)
#target_link_libraries(project_library OmnetPP::main)
#target_link_libraries(project_library OmnetPP::nedxml)
#target_link_libraries(project_library OmnetPP::qtenv)
#target_link_libraries(project_library OmnetPP::qtenv-osg)
#target_link_libraries(project_library OmnetPP::scave)
#target_link_libraries(project_library OmnetPP::sim)
#target_link_libraries(project_library OmnetPP::tkenv)
target_link_libraries(project_library pybind11::embed)

# This creates an OMNet++ CMake run for you
add_opp_run(collaborative-learning-sim 
    CONFIG res/omnetpp.ini 
    DEPENDENCY project_library
)



