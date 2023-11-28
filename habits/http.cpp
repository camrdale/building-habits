#include "http.hpp"

#include <curl/curl.h>

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

#include "expresscpp/console.hpp"
#include "expresscpp/expresscpp.hpp"
#include "expresscpp/middleware/serve_static_provider.hpp"
#include "moves.hpp"
#include "position.hpp"
#include "search.hpp"

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

nlohmann::json buildResponse(const Position& p, const std::string& last_move) {
  nlohmann::json response;
  response["fen"] = p.ToFen();
  response["last_move"] = last_move;
  response["turn"] = p.active_color == WHITE ? "w" : "b";
  response["legal"] = legalMovesJson(p);
  bool is_check = isActiveColorInCheck(p);
  response["in_check"] = is_check;
  response["in_checkmate"] = is_check && response["legal"].empty();
  response["in_draw"] = (!is_check && response["legal"].empty()) || p.IsDraw();
  return response;
}

}  // namespace

void HttpServer::newGame(expresscpp::request_t req,
                         expresscpp::response_t res) {
  auto fen_param = req->GetQueryParams().find("fen");
  if (fen_param == req->GetQueryParams().end()) {
    res->SetStatus(400);
    res->Send("Missing 'fen' query param");
    return;
  }
  const std::string& fen = url_decode(fen_param->second);

  expresscpp::Console::Log("Request: new game in position: " + fen);

  Position p = Position::FromFen(fen);

  game_ = Game();

  nlohmann::json response = buildResponse(p, "");
  std::string response_string = response.dump();
  expresscpp::Console::Log("Response: " + response_string);
  res->Json(response_string);
}

void HttpServer::makeMove(expresscpp::request_t req,
                          expresscpp::response_t res) {
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

  Position p = Position::FromFen(fen);
  int result = habits::move(&p, move);

  if (result != 0) {
    expresscpp::Console::Error("ERROR: illegal move");
    res->SetStatus(400);
    res->Send("Illegal move");
    return;
  }

  game_.opponentMove(move);

  nlohmann::json response = buildResponse(p, move);
  std::string response_string = response.dump();
  expresscpp::Console::Log("Response: " + response_string);
  res->Json(response_string);
}

void HttpServer::search(expresscpp::request_t req, expresscpp::response_t res) {
  auto fen_param = req->GetQueryParams().find("fen");
  if (fen_param == req->GetQueryParams().end()) {
    res->SetStatus(400);
    res->Send("Missing 'fen' query param");
    return;
  }
  const std::string& fen = url_decode(fen_param->second);

  expresscpp::Console::Log("Request: find best move in position: " + fen);

  Position p = Position::FromFen(fen);

  std::string move = game_.bestMove(p);

  expresscpp::Console::Log("Intermediate: found best move: " + move);

  int result = habits::move(&p, move);

  if (result != 0) {
    expresscpp::Console::Error("ERROR: illegal move");
    res->SetStatus(400);
    res->Send("Illegal move");
    return;
  }

  nlohmann::json response = buildResponse(p, move);
  std::string response_string = response.dump();
  expresscpp::Console::Log("Response: " + response_string);
  res->Json(response_string);
}

void HttpServer::listenHttp(bool debug) {
  std::shared_ptr<expresscpp::ExpressCpp> expresscpp =
      std::make_shared<expresscpp::ExpressCpp>();
  if (debug) {
    expresscpp::Console::setLogLevel(expresscpp::LogLevel::kDebug);
  }

  // Rest RPC API endpoints.
  expresscpp->Get("/engine/newgame",
                  [this](expresscpp::request_t req,
                         expresscpp::response_t res) { newGame(req, res); });
  expresscpp->Get("/engine/move/:move",
                  [this](expresscpp::request_t req,
                         expresscpp::response_t res) { makeMove(req, res); });
  expresscpp->Get("/engine/search",
                  [this](expresscpp::request_t req,
                         expresscpp::response_t res) { search(req, res); });

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