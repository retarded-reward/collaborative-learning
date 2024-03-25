#include "python_interpreter.h"
#include <pybind11/embed.h>
#include <stdlib.h>

namespace py = pybind11;

PythonInterpreter* PythonInterpreter::instance = nullptr;

PythonInterpreter::PythonInterpreter(){
    this->python_ref_count = 0;
}

PythonInterpreter* PythonInterpreter::getInstance(){

    if (PythonInterpreter::instance == nullptr){
        PythonInterpreter::instance = new PythonInterpreter();
    }
    return PythonInterpreter::instance;
}

void PythonInterpreter::setup()
{
    py::initialize_interpreter();
    
    // Finds the agent module by reading the path from the environment
    // variable AGENT_MODULE and prepending it to the python path
    char *agent_path = getenv("AGENT_PATH");
    py::module_ sys = py::module_::import("sys");
    py::object path = sys.attr("path");
    path.attr("append")(agent_path);

}

void PythonInterpreter::use()
{
    if (python_ref_count == 0){
        this->setup();
    }
    this->python_ref_count++;
}

void PythonInterpreter::teardown()
{
    py::finalize_interpreter();
}

void PythonInterpreter::put()
{
    if (python_ref_count > 0){
        python_ref_count--;
        if (python_ref_count == 0){
            this ->teardown();
        }
    }
}

PythonInterpreter::~PythonInterpreter()
{
    // cannot destroy until all users have called put()
    if (python_ref_count == 0){
        delete PythonInterpreter::instance;
    }
}


