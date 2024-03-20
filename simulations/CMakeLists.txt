# python + pybind11 configuration
#
# Must use the same python environment used by the agent
# Pybind11 must be unstalled in the agent environment

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
    src/node.cc
)

add_library(project_library SHARED ${SOURCES})

# Define your messages as well
set(MESSAGE_SOURCES
    #messages/a.msg
    #messages/b.msg
)

# uncomment the following if you have messages
#generate_opp_message(${MESSAGE_SOURCES} TARGET project_library)

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
target_link_libraries(project_library pybind11::embed) # pybind11

# This creates an OMNet++ CMake run for you
add_opp_run(collaborative-learning-sim 
    CONFIG src/omnetpp.ini 
    DEPENDENCY project_library
    )



