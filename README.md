

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

Generate the build files and build the binary from the `build` directory:

```
cd build/
cmake ..
cmake --build .
``

## Testing

After the build is completed (still in the `build` directory), run the tests:

```
ctest
```

## Running

Install the dependencies needed for running:

```
sudo apt install libfmt9 libcurl4
```

### Running the Bot Locally

After the build is completed (still in the `build` directory), run the bot
as a local HTTP server (add `--debug` to get more info logs printed):

```
./BuildingHabits
```

Play against the bot by going to http://localhost:8080/index.html

### Running the Bot on Lichess

Get a login token for a new account on Lichess:

* create an account at https://lichess.org/signup
* create a personal OAuth2 token with the "Play games with the bot API"
  (bot:play) scope at
  https://lichess.org/account/oauth/token/create?scopes%5B%5D=bot:play&description=lichess-bot
* store the displayed token in a file (`~/.lichess-token` by default)

Use the token to upgrade the account to a BOT account
([irreversible](https://lichess.org/api#operation/botAccountUpgrade)):

```
curl -d '' https://lichess.org/api/bot/account/upgrade -H "Authorization: Bearer <yourTokenHere>"
```

On the account's profile page, the username should now be prefixed with BOT.

Run the bot in Lichess Bot mode (after the build is completed, and still in
the `build` directory):

```
./BuildingHabits --lichess
```

## Releasing

Before releasing, consider updating the version at the top of
[CMakeLists.txt](CMakeLists.txt).

Create the release package file from the build directory:

```
cd build/
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
cpack
``

The generated file `BuildingHabits-<version>-Linux-binary.tar.gz` can be
transferred to another computer and run (the dependencies are still needed):

```
sudo apt install libfmt9 libcurl4
mkdir building-habits
cd building-habits/
tar -xzvf ../BuildingHabits-<version>-Linux-binary.tar.gz
cd bin
./BuildingHabits
```
