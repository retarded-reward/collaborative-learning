#include <pybind11/embed.h> // everything needed for embedding
#include <stdlib.h>

namespace py = pybind11;

int main(int argc, char *argv[]) {
    
    // get the path to the agent package in the environment variable AGENT_PATH
    char *agent_path = getenv("AGENT_PATH");
    
    // start the interpreter and keep it alive
    py::scoped_interpreter guard{};
    
    py::print("Hello, World!"); // use the Python API
    
    py::module_ sys = py::module_::import("sys");
    py::object path = sys.attr("path");
    path.attr("append")(agent_path);

    py::print(sys.attr("path"));
    py::module_ agent = py::module_::import("agent");

    agent.attr("hello_world_keras")();
    
    return 0;
}
