# collaborative-learning

## How to build

0) Assume `$(PROJECT_PATH)` contains full path to collaborative-learning folder;

1) Create the environment for the python agent by invoking the following command in the `$(PROJECT_PATH)/agent` dir:
```
conda env create -f $PROJECT_PATH/agent/environment.yml -p $PROJECT_PATH/agent/.conda/collaborative-learning-dev
```
2) Save in the environment variable `AGENT_PATH` the absolute path to the `agent` dir:
```
export AGENT_PATH=$PROJECT_PATH/agent
```
    2.1) if needed, checkout the cmake submodule
```
git submodule update --init --recursive
```
3) Configure the cmake project and generate the Makefile:
```
/usr/bin/cmake --no-warn-unused-cli -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_C_COMPILER:FILEPATH=/usr/bin/gcc -DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/g++ -S$PROJECT_PATH -B$PROJECT_PATH/bin -G "Unix Makefiles"
```
4) Launch the simulation by building the Makefile:
```
/usr/bin/cmake --build $PROJECT_PATH/bin --config Debug --target run_collaborative-learning-sim -j 14
```
