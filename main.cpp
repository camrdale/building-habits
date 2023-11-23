#include <curl/curl.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "expresscpp/console.hpp"
#include "expresscpp/expresscpp.hpp"
#include "expresscpp/middleware/serve_static_provider.hpp"
#include "habits/moves.hpp"
#include "habits/position.hpp"

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
  auto fen = req->GetQueryParams().find("fen");
  if (fen == req->GetQueryParams().end()) {
    res->SetStatus(400);
    res->Send("Missing 'fen' query param");
    return;
  }

  habits::Position p = habits::Position::FromFen(url_decode(fen->second));

  nlohmann::json response;
  response["legal"] = habits::calculateLegalMoves(p);

  res->Json(response.dump());
}

int main(int argc, char const* argv[]) {
  std::shared_ptr<expresscpp::ExpressCpp> expresscpp =
      std::make_shared<expresscpp::ExpressCpp>();
  expresscpp::Console::setLogLevel(expresscpp::LogLevel::kDebug);

  // Rest RPC API endpoints.
  expresscpp->Get("/engine/legal", legalMoves);

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
                 std::cout << "go to http://localhost:" << port << std::endl;
               })
      .Run();

  return 0;
}
