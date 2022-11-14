# Softeq's C++ implementation of object–relational mapping

## Requirements 
The library supports C++11 standard. More modern standards are not supported by some compilers the project is supposed to be compiled with.

# Environment setup and build under docker 

## Prepare build environment

1. Install [cmake_docker](https://stash.softeq.com/projects/EMBLAB/repos/cmake-docker.io/browse) and execute 
   steps from 0 to 2

2. Build docker image by executing `cmake_build docker` command from the project root directory

## Building library under docker

### Building target

Configure environment variable using ``export CMAKE_VOL=`pwd` `` if CMAKE_VOL has not been set to another location and use `cmake_build dev` to build library.

### Building and running tests 

Configure environment variable using ``export CMAKE_VOL=`pwd` `` if CMAKE_VOL has not been set to another location and use `cmake_build test` to run tests

## Building and testing migration and mysql extensions

### Migration extension

Migration requires [softeq common library](https://stash.softeq.com/projects/EMBLAB/repos/linux-common-library/browse) to be present on the system.

Use `cmake_build dev -DEXTENSION_MIGRATION=on` to build with migration extension.

Use `cmake_build test -DEXTENSION_MIGRATION=on` to run migration tests

### MySQL extension

TBA 

## Troubleshooting

if build fails with error 'The dir (/.../dbfacade) does not contain prepare_env.sh' please rebuild docker image via `cmake_build docker` command
