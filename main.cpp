#include <wordexp.h>

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "habits/bot.hpp"
#include "habits/http.hpp"

int lichessMode(int argc, char *argv[]) {
  std::string token_file = "~/.lichess-token";
  for (int i = 1; i < argc; i++) {
    std::string s(argv[i]);
    if (s.rfind("--token_file", 0) == 0) {
      if (s.rfind("--token_file=", 0) == 0) {
        token_file = argv[i] + 13;
      } else if (i + 1 < argc) {
        token_file = argv[i + 1];
      } else {
        std::cerr << "--token_file flag was not followed by a file name"
                  << std::endl;
        return 13;
      }
      std::cout << "Using overridden token_file: " << token_file << std::endl;
      break;
    }
  }
  wordexp_t parsed_token_file;
  int result = wordexp(token_file.c_str(), &parsed_token_file, 0);
  if (result != 0) {
    std::cerr << "Failed to parse token file: " << token_file << std::endl;
    return result;
  }
  if (parsed_token_file.we_wordc == 0 || parsed_token_file.we_wordc > 1) {
    std::cerr << "Failed to find token file in: " << token_file << std::endl;
    return 10;
  }
  std::ifstream input_file(parsed_token_file.we_wordv[0]);
  if (!input_file.good()) {
    std::cerr << "Failed to open token file: " << parsed_token_file.we_wordv[0]
              << std::endl;
    return 9;
  }
  std::string token;
  std::getline(input_file, token);
  if (token.empty()) {
    std::cerr << "Failed to find token in file: "
              << parsed_token_file.we_wordv[0] << std::endl;
    return 11;
  }
  wordfree(&parsed_token_file);

  habits::LichessBot bot(token);
  return bot.listenForChallenges();
}

int httpMode(int argc, char *argv[]) {
  bool debug = false;
  if (std::find(argv, argv + argc, std::string("--debug")) != argv + argc) {
    debug = true;
  }

  habits::HttpServer http;
  http.listenHttp(debug);
  return 0;
}

int main(int argc, char *argv[]) {
  if (std::find(argv, argv + argc, std::string("--help")) != argv + argc) {
    std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options" << std::endl;
    std::cout << "  --help       = Print usage information and exit."
              << std::endl;
    std::cout << "  --lichess    = Switch to Lichess Bot mode." << std::endl;
    std::cout << std::endl;
    std::cout << "Options for HTTP mode (the default)" << std::endl;
    std::cout << "  --debug      = Print HTTP debugging messages." << std::endl;
    std::cout << std::endl;
    std::cout << "Options for Lichess Bot mode (started with --lichess)"
              << std::endl;
    std::cout << "  --token_file = Specify the file to get the OAUTH2 token "
                 "from. Defaults to ~/.lichess-token"
              << std::endl;
    std::cout << std::endl;
    return 0;
  }

  if (std::find(argv, argv + argc, std::string("--lichess")) != argv + argc) {
    return lichessMode(argc, argv);
  }

  return httpMode(argc, argv);
}
