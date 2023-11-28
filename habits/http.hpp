#pragma once

#include "expresscpp/expresscpp.hpp"
#include "search.hpp"

namespace habits {

class HttpServer {
 public:
  HttpServer() = default;

  void listenHttp(bool debug = false);

 private:
  void newGame(expresscpp::request_t req, expresscpp::response_t res);
  void makeMove(expresscpp::request_t req, expresscpp::response_t res);
  void search(expresscpp::request_t req, expresscpp::response_t res);

  Game game_;
};

}  // namespace habits
