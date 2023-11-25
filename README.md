

## Dependencies

- cmake
- expresscpp
- nlohmann-json
- fmt
- curl
- nodejs

Also see the Node module dependencies in the js/ folder.


## Building

Install the dependencies needed for building:

```
sudo apt install build-essential gdb cmake nodejs npm rollup libcurl4-openssl-dev
```

Also, expresscpp is needed. To build and install it from source (taken from
[here](https://github.com/expresscpp/expresscpp#installing-and-using-find_package)):

```
sudo apt install libboost-all-dev nlohmann-json3-dev libfmt-dev libgtest-dev
git clone https://gitlab.com/expresscpp/expresscpp.git
cd expresscpp
mkdir build
cd build
cmake ..
make -j
sudo make install
```

## Running

Install the dependencies needed for running:

```
sudo apt install libfmt9
```
