#ifndef PYTHON_INTERPRETER_H
#define PYTHON_INTERPRETER_H

#include <pybind11/embed.h>
#include "cpp_visibility_tools.h"

namespace py = pybind11;

// https://github.com/pybind/pybind11/issues/1622#issuecomment-452718093
class DLL_LOCAL PyStdStreamsRedirect {
    py::object _stdout;
    py::object _stderr;
    py::object _out_buffer;
public:
    PyStdStreamsRedirect() {
        auto sysm = py::module::import("sys");
        _stdout = sysm.attr("stdout");
        _stderr = sysm.attr("stderr");
        auto stringio = py::module::import("io").attr("StringIO");
        _out_buffer = stringio();  // Other filelike object can be used here as well, such as objects created by pybind11
        // stderr and stdout buffers are merged
        sysm.attr("stdout") = _out_buffer;
        sysm.attr("stderr") = _out_buffer;
    }
    std::string outString() {
        py::str out_string;
        
        _out_buffer.attr("seek")(0);
        out_string = py::str(_out_buffer.attr("read")());
        _out_buffer.attr("truncate")(0);
        _out_buffer.attr("seek")(0);
        return out_string;
    }
    ~PyStdStreamsRedirect() {
        auto sysm = py::module::import("sys");
        sysm.attr("stdout") = _stdout;
        sysm.attr("stderr") = _stderr;
    }
};

/*
    Singleton holder for references to the python interpreter used to 
    run the agent.
    Each object that wants to interact with the python interpreter must declare it
    by invoking the use() method.
    When the object knows that it will no longer use the python interpreter, it must
    call the put() method.
*/
class DLL_LOCAL PythonInterpreter{

    private:
        static PythonInterpreter* instance; // do not destroy before all users called the put() method
        /**
         * How many users are currently using the python interpreter.
        */
        int python_ref_count;
        
        PythonInterpreter();
        void setup();
        void teardown();

    public:
        
        PyStdStreamsRedirect *pyStdStreamsRedirect;

        static PythonInterpreter* getInstance();
        /**
         * Increments by one the number of users of the python interpreter.
         * If the counter was zero prior to this call, the python interpreter
         * is initialized and can be used.
        */
        void use();

        /**
         * Decrements by one the number of users of the python interpreter.
         * If the counter reaches zero, the python interpreter is shut down
         * and must be initialized again before being reused.
         * NOTE: Users must destroy all their references to python objects they own
         * before calling this method.
        */
        void put();

        ~PythonInterpreter();
};

#endif // PYTHON_INTERPRETER_H