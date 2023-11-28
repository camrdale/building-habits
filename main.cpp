#include <algorithm>
#include <iostream>
#include <string>

#include "habits/http.hpp"

int main(int argc, char* argv[]) {
  if (std::find(argv, argv + argc, std::string("--help")) != argv + argc) {
    std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options" << std::endl;
    std::cout << "  --debug     = Print debugging messages." << std::endl;
    std::cout << "  --help      = Print usage information and exit." << std::endl;
    std::cout << std::endl;
    return 0;
  }

  bool debug = false;
  if (std::find(argv, argv + argc, std::string("--debug")) != argv + argc) {
    debug = true;
  }

  habits::HttpServer http;
  http.listenHttp(debug);

  return 0;
}
