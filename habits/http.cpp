#include "http.hpp"

#include <curl/curl.h>

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

#include "expresscpp/console.hpp"
#include "expresscpp/expresscpp.hpp"
#include "expresscpp/middleware/serve_static_provider.hpp"
#include "moves.hpp"
#include "position.hpp"

namespace habits {

namespace {

std::string url_decode(std::string encoded) {
  std::replace(encoded.begin(), encoded.end(), '+', ' ');
  int output_length;
  const auto decoded_value =
      curl_easy_unescape(nullptr, encoded.c_str(),
                         static_cast<int>(encoded.length()), &output_length);
  std::string result(decoded_value, output_length);
  curl_free(decoded_value);
  return result;
}

void legalMoves(expresscpp::request_t req, expresscpp::response_t res) {
  auto fen_param = req->GetQueryParams().find("fen");
  if (fen_param == req->GetQueryParams().end()) {
    res->SetStatus(400);
    res->Send("Missing 'fen' query param");
    return;
  }
  const std::string& fen = url_decode(fen_param->second);

  expresscpp::Console::Log("Request: legal moves in position: " + fen);

  habits::Position p = habits::Position::FromFen(fen);

  nlohmann::json response;
  response["legal"] = habits::legalMovesJson(p);
  response["turn"] = p.active_color == habits::WHITE ? "w" : "b";
  std::string response_string = response.dump();

  expresscpp::Console::Log("Response: " + response_string);

  res->Json(response_string);
}

void makeMove(expresscpp::request_t req, expresscpp::response_t res) {
  auto move_param = req->GetParams().find("move");
  if (move_param == req->GetParams().end()) {
    res->SetStatus(400);
    res->Send("Missing 'move' param");
    return;
  }
  const std::string& move = url_decode(move_param->second);
  auto fen_param = req->GetQueryParams().find("fen");
  if (fen_param == req->GetQueryParams().end()) {
    res->SetStatus(400);
    res->Send("Missing 'fen' query param");
    return;
  }
  const std::string& fen = url_decode(fen_param->second);

  expresscpp::Console::Log("Request: move " + move + " in position: " + fen);

  habits::Position p = habits::Position::FromFen(fen);
  int result = habits::move(&p, move);

  if (result != 0) {
    expresscpp::Console::Error("ERROR: illegal move");
    res->SetStatus(400);
    res->Send("Illegal move");
    return;
  }

  nlohmann::json response;
  response["fen"] = p.ToFen();
  response["legal"] = habits::legalMovesJson(p);
  bool is_check = habits::isActiveColorInCheck(p);
  response["in_check"] = is_check;
  response["in_checkmate"] = is_check && response["legal"].empty();
  response["in_draw"] = (!is_check && response["legal"].empty()) || p.IsDraw();
  std::string response_string = response.dump();

  expresscpp::Console::Log("Response: " + response_string);

  res->Json(response_string);
}

}  // namespace

void listenHttp(bool debug) {
  std::shared_ptr<expresscpp::ExpressCpp> expresscpp =
      std::make_shared<expresscpp::ExpressCpp>();
  if (debug) {
    expresscpp::Console::setLogLevel(expresscpp::LogLevel::kDebug);
  }

  // Rest RPC API endpoints.
  expresscpp->Get("/engine/legal", legalMoves);
  expresscpp->Get("/engine/move/:move", makeMove);

  // Fall back to attempting to serve static files.
  expresscpp->Use(expresscpp::StaticFileProvider("../static"));

  const uint16_t port = 8080u;

  // Start listening for requests and block until ctrl+C
  expresscpp
      ->Listen(port,
               [=](auto ec) {
                 if (ec) {
                   std::cerr << "ERROR: " << ec.message() << std::endl;
                   exit(1);
                 }
                 std::cout << "HTTP server running, go to http://localhost:"
                           << port << "/index.html" << std::endl;
               })
      .Run();
}

}  // namespace habits