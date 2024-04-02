#ifndef PYTHON_INTERPRETER_H
#define PYTHON_INTERPRETER_H

/*
    Singleton holder for references to the python interpreter used to 
    run the agent.
    Each object that wants to interact with the python interpreter must declare it
    by invoking the use() method.
    When the object knows that it will no longer use the python interpreter, it must
    call the put() method.
*/
class PythonInterpreter{

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
         * NOTE! Users must destroy all their references to python objects they own
         * before calling this method.
        */
        void put();

        ~PythonInterpreter();
};

#endif // PYTHON_INTERPRETER_H