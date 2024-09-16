# Build from Source

When you want to use `PeTrack` on a operating system we do not provide an executable or you want to improve `PeTrack`, you need to build `PeTrack` from source. In the following the dependencies and the building process is described.

## Dependencies
In order to compile `PeTrack` it is necessary to first install the required libraries.

### Compilers
Any compiler with support for C++ 17 should work. At the moment the following compilers are tested:
- MinGW 8.1 on Windows
- clang++-14 on Linux (Ubuntu)

### Required tools and libraries
For using `PeTrack` following tools and libraries are needed:
- git
- CMake (>= 3.16)
- Qt (>= 5.14, < 6.0)
- OpenCV (== 4.10)
- Qwt (>= 6.2)

When developing `PeTrack` and using the test framework following additional tools and libraries are used, they are provided as submodules by `PeTrack`:
- Catch2
- trompeloeil
- spdlog
- ezc3d

Some additional tools need to be installed by yourself, as:
- Python (>= 3.6)
- pytest

## Linux/MacOS/Windows

Before starting building `PeTrack` it is necessary to install the required dependencies. If done, you can start to build `PeTrack`. Here only the process to compile with the command line is described. You can use your favorite IDE (which supports CMake projects), e.g., QtCreator, CLion, Visual Studio Code, Visual Studio, XCode.

For windows users, the linux notation of paths `/` instead of `\` is used. It is highly recommended to install [Windows Terminal](https://aka.ms/terminal) for the same behavior.

### Get the code
In this tutorial we will assume that we start at `/home/dev/`. The following notation will be used for the commands (when no <command> is displayed, it shows the result of the previous command, usually a change of directory):
```
[<current_directory>] $ <command>
```

First thing you need to do is clone the repository to get the source code. This can be done in any directory. Be aware that a new directory with the name `petrack` will be created.

```
[/home/dev] $ git clone https://jugit.fz-juelich.de/ped-dyn-emp/petrack.git

```

### How to build 
After getting the source code, you need to switch to the newly created directory:

```
[/home/dev/] $ cd petrack
[/home/dev/petrack] $
```
Next steps is to load the third-party dependencies specified directly as submodules. This can be done with:

```
[/home/dev/petrack] $ git submodule update --init deps/Catch2 deps/spdlog deps/ezc3d deps/trompeloeil
```

Next you need to create a build directory in which CMake will create all the files needed for compiling `PeTrack`. It can also be used to have different executable for different versions of `PeTrack`. Also we need to move into the build directory.

```
[/home/dev/petrack] $ mkdir build
[/home/dev/petrack] $ cd build
[/home/dev/petrack/build] $
```

Now you can start to build `PeTrack`:

```
[/home/dev/petrack/build] $ cmake -DCMAKE_BUILD_TYPE=Release ..
[/home/dev/petrack/build] $ cmake --build .
```

Afterwards `PeTrack` can be executed with:

```
[/home/dev/petrack/build] $ ./petrack
```
If you want to develop on PeTrack you need to download the test data, which is stored in the git large file system.
It can be downloaded with
```
[/home/dev/petrack] $ git lfs install --skip-smudge
[/home/dev/petrack] $ git lfs pull
```

When you add your building directory to your path (here `/home/dev/petrack/build`). You can call `PeTrack` from anywhere with

```
[/home/dev/some/where/else] $ petrack
```

Please check the documentation of your operating system how to do this.


### CMake configuration flags

Following CMake options might be helpful for you when compiling, see [CMake Documentation](https://cmake.org/cmake/help/latest/manual/cmake.1.html) for more details. Some of higher interest are explained here:
- `-G` sets the generator, which is used to compile the source code later. Default on linux/MacOS are `Makefiles` and on Windows `Visual Studio XX YEAR` depending on the installed MSVS version. Also other generators as `Ninja` can be used. Usage:

```
$ cmake -G Ninja <path_to_cmakelist>
```

- `-DCMAKE_BUILD_TYPE=` sets the build type. Typical options are `Release` for faster and better optimized builds. And `Debug` for slower debugging builds. Usage:

```
$ cmake -DCMAKE_BUILD_TYPE=Debug <path_to_cmakelist>
```

- `-DCMAKE_PREFIX_PATH=[paths where additionally to search for libraries etc]` adds directories where CMake will search for libraries, useful if you do not want to install a library to the `PATH`.

- `-DCMAKE_CXX_COMPILER=` sets the C++ compilers used when building the project. Usage:

```
$ cmake -DCMAKE_CXX_COMPILER=clang++ <path_to_cmakelist>
```

We extended the options with our own, which allow better configuration of the projects:

- `-DBUILD_UNIT_TESTS=` can be set to `ON` or `OFF`. Defines with the unit tests are build. Default if `ON`

### Known issues

- CMake fails when searching one of the needed libraries.
    - Possible solution: Check if the libraries are installed to your `PATH` or use `-DCMAKE_PREFIX_PATH` CMake option

- Change of CMake option seem to have no influence
    - Possible solution: It may be necessary to delete the CMakeCache and run CMake again with the new options.

## Using docker for developing

In some cases it may be hard to set up the environment.
If the goal is to create a development environment you can also use a docker container.
For example CLion has a built-in functionality to support working inside a docker container.
You can find an ubuntu dockerfile in the repository at `container/ubuntu`.
This example is for a linux system, but for example docker commands should be similar on other 
operating systems.

### Setup Docker
If not done already you have to install docker on you system. For Linux, this is normally done
via you packet manager and well documented (e.g. for ArchLinux see the [wiki](https://wiki.archlinux.org/title/docker)).
After installing, you can start the service with `# systemctl start docker`.
:::{note}
It is best to install the service in such a way, that you can use it without root.
Then it is very easy to make your IDE use it.
An easy way is to add you user to the `docker` group: `$ usermod -aG docker [username]`
:::

You can check the installation by calling 
- `$ docker info` to get status information
- with `$ docker run -it --rm archlinux bash -c "Hello World"` an archlinux container gets 
  downloaded and a Hello World program is run.

### Installing the Petrack Container
There is a dockerfile inside the repository (`container/ubuntu/`).
After navigating into the folder, you can install it with `$ docker build -t petrack:latest .`.
`petrack` is the name and `latest` the version of the container, and technically you can choose it freely.
The process may take a while. Afterward you should see the container when running `$ docker images`.

### Setting up the IDE
CLion supports setting up a docker toolchain directly.
Under "Settings -> Build, Execution, Deployment -> Toolchains", you can create a new docker-toolchain.
If everything is installed correctly, CLion should detect it automatically.
There you have to select petrack:latest as the image to use.  
At "Settings -> Build, Execution, Deployment -> CMake" you can set up a profile that uses this docker toolchain.
For this, select as "Toolchain" "Docker". 
:::{note}
When making a complete new CMake profile you may want to set some CMake options (e.g. build units tests).
See the default installation manual for more information.
:::

**Now you can select the profile and petrack should compile successfully**.   
However, it does not run *yet*. This is because you have to define the display when running the docker container (see [here](https://wiki.archlinux.org/title/docker#Run_graphical_programs_inside_a_container)).
On you linux machine you have to run `xhost +local:`.
Then you can add this to the "Container Settings" under "Tollchains" in CLion such that it says:  
`--entrypoint -v /tmp/.X11-unix:/tmp/.X11-unix --device /dev/dri:/dev/dri --rm`

Now petrack should compile and run successfully.

