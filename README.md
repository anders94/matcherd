# matcherd

An in-memory order matcher written in C++ and (eventually) replicated through RAFT.

## How it Works

`matcherd` attempts to connect to Redis and `BLPOP` orders from a named list.
Orders are submitted to `matcherd` by pushing them onto the Redis list using
`RPUSH`. They are in the pipe deliniated format [id]|[side]|[price]|[volume]
and only include integers. (decimal precision is added externally) Example:

```4256|B|2800|419```

In this case, order ID 4256 is placing a Buy order at 2800 for 419 units.

Internally, there is a map for buys and a map for sells. The keys are the price
and the values are queues of all the offers for that price in oldest to newest
order. Offers get removed from the front of the queue and new orders are added
to the end.

## Getting Started

These instructions will get you a copy of `matcherd` up and running on your local
machine for development and testing purposes.

### Prerequisites

This project has several requirements.

* **Redis 4.0 and above** - found at [https://redis.io](https://redis.io)

* **C++ Compiler** - needs to support at least the **C++17** standard, i.e. *MSVC*,
*GCC*, *Clang*

> ***Note:*** *You also need to be able to provide ***CMake*** a supported
[generator](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html).*

### Installing

Clone the project from [GitHub](https://github.com/anders94/matcherd) or
[generate a new repository from it](https://github.com/anders94/matcherd/generate)
(also on **GitHub**).

If you wish to clone the repository, rather than generate from it, you simply need
to run:

```bash
git clone https://github.com/anders94/matcherd/
```

## Building the project

To build the project:

```bash
mkdir build/ && cd build/
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build . --target install
```

> ***Note:*** *The custom ``CMAKE_INSTALL_PREFIX`` can be omitted if you wish to
install in [the default install location](https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html).*

## Running the tests

By default, the template uses [Google Test](https://github.com/google/googletest/)
for unit testing. Unit testing can be disabled in the options, by setting the
`ENABLE_UNIT_TESTING` (from
[cmake/StandardSettings.cmake](cmake/StandardSettings.cmake)) to be false. To run
the tests, simply use CTest, from the build directory, passing the desire
configuration for which to run tests for. An example of this procedure is:

```bash
cd build          # if not in the build directory already
ctest -C Release  # or `ctest -C Debug` or any other configuration you wish to test

# you can also run tests with the `-VV` flag for a more verbose output (i.e.
#GoogleTest output as well)
```

### End to end tests

If applicable, should be presented here.

### Coding style tests

If applicable, should be presented here.

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on how you can
become a contributor and the process for submitting pull requests to us.

## Authors

* **Anders Brownworth** - [@anders94](https://github.com/anders94)

## License

This project is licensed under the [MIT License](https://mit-license.org) - see the
[LICENSE](LICENSE) file for details.
