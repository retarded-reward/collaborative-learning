# collaborative-learning

## How to build

0) Assume `$PROJECT_PATH` contains full path to collaborative-learning folder and `$OMNETPP_PATH` to the Omnet++ one;

1) Create the environment for the python agent by invoking the following command in the `$PROJECT_PATH/agent` dir:
```
conda env create -f $PROJECT_PATH/agent/environment.yml -p $PROJECT_PATH/agent/.conda/collaborative-learning-dev
```
2) Save in the environment variable `AGENT_PATH` the absolute path to the `agent` dir:
```
export AGENT_PATH=$PROJECT_PATH/agent
```

- if needed, checkout the cmake submodule first

```
git submodule update --init --recursive
```
3) Configure the cmake project and generate the Makefile:
```
/usr/bin/cmake --no-warn-unused-cli -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_C_COMPILER:FILEPATH=/usr/bin/gcc -DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/g++ -S$PROJECT_PATH -B$PROJECT_PATH/bin -G "Unix Makefiles"
```
- if it complains about not finding the Omnet++ package, it could be that you forgot to set its env:
```
source $OMNETPP_PATH/setenv
```
4) Launch the simulation by building the Makefile:
```
/usr/bin/cmake --build $PROJECT_PATH/bin --config Debug --target run_collaborative-learning-sim -j 14
```

## Analyzing results

Statistics are recorderd in the `simulations/results` folder.

To analyze statistics, open the `simulations/results/General.anf` in the Omnet++ IDE and use its Analysys Tool.


## Agent coniguration
The agent configuration is done through a JSON file named "agent_conf.json". This file contains the basic configuration, which can be overridden in various ways:

    by modifying the file directly,
    by creating another file and specifying its path in the agent bean (in this case, all fields must be filled),
    by passing a new JSON string directly into the agent bean that specifies all or some of the parameters, with any missing parameters being taken from the base configuration file.

Documentation on how to configure the agent can be found at: [Agent Configuration](https://github.com/retarded-reward/collaborative-learning/wiki/Agent-Configuration).
