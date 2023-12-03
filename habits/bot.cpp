#include "bot.hpp"

#include <curl/curl.h>

#include <iostream>
#include <iterator>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <vector>

#include "moves.hpp"
#include "position.hpp"
#include "search.hpp"

namespace habits {

namespace {

static size_t ReceiveGameState(void *contents, size_t size, size_t nmemb,
                               void *game) {
  size_t realsize = size * nmemb;
  std::string str;
  str.assign((char *)contents, realsize);
  ((LichessGame *)game)->receiveGameState(str);
  return realsize;
}

static size_t ReceiveIncomingEvent(void *contents, size_t size, size_t nmemb,
                                   void *bot) {
  size_t realsize = size * nmemb;
  std::string str;
  str.assign((char *)contents, realsize);
  ((LichessBot *)bot)->receiveIncomingEvent(str);
  return realsize;
}

static size_t LogResponse(void *contents, size_t size, size_t nmemb,
                          void *bot) {
  size_t realsize = size * nmemb;
  std::string str;
  str.assign((char *)contents, realsize);
  std::cout << "Received response: " << str << std::endl;
  return realsize;
}

}  // namespace

void LichessGame::startGame() {
  curl_global_init(CURL_GLOBAL_DEFAULT);

  CURL *curl = curl_easy_init();
  if (!curl) {
    std::cerr << "Failed to initialize CURL for game state" << std::endl;
    return;
  }

  CURLcode res;
  char errbuf[CURL_ERROR_SIZE];
  std::string url = "https://lichess.org/api/bot/game/stream/" + game_id_;
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_BEARER);
  curl_easy_setopt(curl, CURLOPT_XOAUTH2_BEARER, token_.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ReceiveGameState);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)this);
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
  errbuf[0] = 0;
  res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    std::cerr << "ERROR getting game state " << curl_easy_strerror(res) << ": "
              << errbuf << std::endl;
    return;
  }
  curl_easy_cleanup(curl);
  curl_global_cleanup();
  std::cout << "Game loop exiting" << std::endl;
}

void LichessGame::initializeState(const nlohmann::json &state) {
  position_ = Position::FromFen(initial_fen_);
  status_ = state["status"].get<std::string>();

  std::string initial_moves = state["moves"].get<std::string>();
  std::istringstream iss(initial_moves);
  std::string initial_move;
  moves_.clear();
  while (std::getline(iss, initial_move, ' ')) {
    int result = habits::move(&position_, initial_move);
    if (result != 0) {
      std::cerr << "Received illegal move " << initial_move
                << " in position: " << position_.ToFen() << std::endl;
    }
    if (myTurn()) {
      game_.opponentMove(initial_move);
    }
    moves_.push_back(initial_move);
  }
}

void LichessGame::updateState(const nlohmann::json &state) {
  status_ = state["status"].get<std::string>();
  std::string new_moves = state["moves"].get<std::string>();
  std::istringstream iss(new_moves);
  std::string new_move;
  int currentMoveIndex = 0;
  while (std::getline(iss, new_move, ' ')) {
    if (currentMoveIndex < moves_.size() &&
        moves_[currentMoveIndex].compare(new_move) == 0) {
      currentMoveIndex++;
      continue;
    }
    if (currentMoveIndex < moves_.size()) {
      std::cerr << "New list of moves doesn't match existing list: "
                << new_moves << std::endl;
      // Reinitialize the state from the initial FEN with these moves.
      initializeState(new_moves);
      return;
    }

    int result = habits::move(&position_, new_move);
    if (result != 0) {
      std::cerr << "Received illegal move " << new_move
                << " in position: " << position_.ToFen() << std::endl;
    }
    game_.opponentMove(new_move);
    moves_.push_back(new_move);
    currentMoveIndex++;
  }
}

bool LichessGame::myTurn() const {
  return status_.compare("started") == 0 &&
         ((position_.active_color == WHITE && color_ == 'w') ||
          (position_.active_color == BLACK && color_ == 'b'));
}

void LichessGame::makeBestMove() {
  std::string move = game_.bestMove(position_);
  int result = habits::move(&position_, move);
  if (result != 0) {
    std::cerr << "Best move was illegal move " << move
              << " in position: " << position_.ToFen() << std::endl;
  }
  moves_.push_back(move);

  CURL *curl = curl_easy_init();
  if (!curl) {
    std::cerr << "Failed to initialize CURL to make a move" << std::endl;
    return;
  }

  CURLcode res;
  char errbuf[CURL_ERROR_SIZE];
  std::string url =
      "https://lichess.org/api/bot/game/" + game_id_ + "/move/" + move;
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 0);
  curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_BEARER);
  curl_easy_setopt(curl, CURLOPT_XOAUTH2_BEARER, token_.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, LogResponse);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)this);
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
  errbuf[0] = 0;
  res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    std::cerr << "ERROR making a move " << curl_easy_strerror(res) << ": "
              << errbuf << std::endl;
    return;
  }
  curl_easy_cleanup(curl);
}

void LichessGame::receiveGameState(std::string data) {
  if (data.find_first_not_of(" \t\n\r\f\v") == std::string::npos) {
    // Ignore empty keep-alive message.
    return;
  }

  nlohmann::json json = nlohmann::json::parse(data, nullptr, false);
  if (json.is_discarded()) {
    std::cerr << "Failed to parse JSON from data: " << data << std::endl;
    return;
  }
  if (json.empty()) {
    std::cout << "Ignoring keep-alive empty json." << std::endl;
    return;
  }
  if (!json.contains("type")) {
    std::cerr << "Ignoring message with no type: " << data << std::endl;
    return;
  }

  std::string type = json["type"].get<std::string>();
  if (type.compare("gameFull") == 0) {
    std::cout << "Full game state: " << json << std::endl;
    initial_fen_ = json["initialFen"].get<std::string>();
    if (initial_fen_.compare("startpos") == 0) {
      initial_fen_ = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    }
    initializeState(json["state"]);
    if (myTurn()) {
      makeBestMove();
    }
    return;
  }
  if (type.compare("gameState") == 0) {
    std::cout << "Received game state update: " << json << std::endl;
    updateState(json);
    if (myTurn()) {
      makeBestMove();
    }
    return;
  }
  if (type.compare("chatLine") == 0) {
    std::cout << "Received chat meesage: " << json << std::endl;
    return;
  }
  if (type.compare("opponentGone") == 0) {
    std::cout << "Opponent might be gone: " << json << std::endl;
    return;
  }
  std::cerr << "Received invalid type for /api/bot/game/stream/" << game_id_
            << " " << type << ": " << json << std::endl;
}

void LichessBot::acceptChallenge(std::string challenge_id) {
  CURL *curl = curl_easy_init();
  if (!curl) {
    std::cerr << "Failed to initialize CURL to accept challenge" << std::endl;
    return;
  }

  CURLcode res;
  char errbuf[CURL_ERROR_SIZE];
  std::string url =
      "https://lichess.org/api/challenge/" + challenge_id + "/accept";
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 0);
  curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_BEARER);
  curl_easy_setopt(curl, CURLOPT_XOAUTH2_BEARER, token_.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, LogResponse);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)this);
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
  errbuf[0] = 0;
  res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    std::cerr << "ERROR accepting challenge " << curl_easy_strerror(res) << ": "
              << errbuf << std::endl;
    return;
  }
  curl_easy_cleanup(curl);
}

bool LichessBot::rejectChallenge(nlohmann::json challenge) {
  std::string reason;
  if (current_game_ != nullptr) {
    reason = "later";
  } else if (challenge["timeControl"]["type"].get<std::string>().compare(
                 "clock") != 0) {
    reason = "timeControl";
  } else if (challenge["timeControl"]["limit"].get<int>() > 600) {
    reason = "tooSlow";
  } else if (challenge["timeControl"]["limit"].get<int>() < 180) {
    reason = "tooFast";
  } else if (challenge["variant"]["key"].get<std::string>().compare(
                 "standard") != 0) {
    reason = "standard";
  } else if (!challenge["challenger"]["title"].is_null() &&
             challenge["challenger"]["title"].get<std::string>().compare(
                 "BOT") == 0) {
    reason = "noBot";
  }

  if (reason.empty()) {
    return false;
  }

  std::cout << "Rejecting a challenge with reason " << reason << ": "
            << challenge << std::endl;
  std::string challenge_id = challenge["id"].get<std::string>();
  CURL *curl = curl_easy_init();
  if (!curl) {
    std::cerr << "Failed to initialize CURL to reject challenge" << std::endl;
    return true;
  }

  CURLcode res;
  char errbuf[CURL_ERROR_SIZE];
  std::string url =
      "https://lichess.org/api/challenge/" + challenge_id + "/decline";
  std::string body = "reason=" + reason;
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.size());
  curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_BEARER);
  curl_easy_setopt(curl, CURLOPT_XOAUTH2_BEARER, token_.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, LogResponse);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)this);
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
  errbuf[0] = 0;
  res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    std::cerr << "ERROR rejecting challenge " << curl_easy_strerror(res) << ": "
              << errbuf << std::endl;
  }
  curl_easy_cleanup(curl);
  return true;
}

int LichessBot::listenForChallenges() {
  curl_global_init(CURL_GLOBAL_DEFAULT);

  CURL *curl = curl_easy_init();
  if (!curl) {
    std::cerr << "Failed to initialize CURL" << std::endl;
    return 2;
  }

  std::cout << "Listening for incoming challenge requests. Challenge the bot "
               "at https://lichess.org/@/camrdale-test-bot"
            << std::endl;
  CURLcode res;
  char errbuf[CURL_ERROR_SIZE];
  curl_easy_setopt(curl, CURLOPT_URL, "https://lichess.org/api/stream/event");
  curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_BEARER);
  curl_easy_setopt(curl, CURLOPT_XOAUTH2_BEARER, token_.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ReceiveIncomingEvent);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)this);
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
  errbuf[0] = 0;
  res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    std::cerr << "ERROR " << curl_easy_strerror(res) << ": " << errbuf
              << std::endl;
    return 3;
  }
  std::cout << "Event stream ended, shutting down." << std::endl;
  curl_easy_cleanup(curl);
  curl_global_cleanup();

  return 0;
}

void LichessBot::receiveIncomingEvent(std::string data) {
  if (data.find_first_not_of(" \t\n\r\f\v") == std::string::npos) {
    // Ignore empty keep-alive message.
    return;
  }

  nlohmann::json json = nlohmann::json::parse(data, nullptr, false);
  if (json.is_discarded()) {
    std::cerr << "Failed to parse JSON from data: " << data << std::endl;
    return;
  }
  if (json.empty()) {
    std::cout << "Ignoring keep-alive empty json." << std::endl;
    return;
  }
  if (!json.contains("type")) {
    std::cerr << "Ignoring message with no type: " << data << std::endl;
    return;
  }

  std::string type = json["type"].get<std::string>();
  if (type.compare("challenge") == 0) {
    if (rejectChallenge(json["challenge"])) {
      return;
    }
    std::cout << "Accepting challenge: " << json["challenge"] << std::endl;
    acceptChallenge(json["challenge"]["id"].get<std::string>());
    return;
  }
  if (type.compare("challengeCanceled") == 0) {
    std::cout << "Challenge was cancelled: " << json["challenge"] << std::endl;
    return;
  }
  if (type.compare("challengeDeclined") == 0) {
    std::cout << "Declined challenge: " << json["challenge"] << std::endl;
    return;
  }
  if (type.compare("gameStart") == 0) {
    std::cout << "Game started: " << json["game"] << std::endl;
    if (current_game_ != nullptr) {
      if (current_game_->getGameId().compare(
              json["game"]["gameId"].get<std::string>()) == 0) {
        std::cerr << "Received gameStart for already started game: "
                  << json["game"] << std::endl;
        return;
      }
      std::cerr << "Received gameStart but already playing game "
                << current_game_->getGameId() << ": " << json["game"]
                << std::endl;
      return;
    }
    current_game_ = std::make_unique<LichessGame>(json["game"], token_);
    current_game_thread_ = std::make_unique<std::thread>(
        &LichessGame::startGame, current_game_.get());
    return;
  }
  if (type.compare("gameFinish") == 0) {
    if (current_game_ != nullptr &&
        current_game_->getGameId().compare(
            json["game"]["gameId"].get<std::string>()) == 0) {
      std::cout << "Waiting for game thread to exit: " << json["game"]
                << std::endl;
      current_game_thread_->join();
      std::cout << "Cleaning up finished game" << std::endl;
      current_game_thread_.reset();
      current_game_.reset();
    } else {
      std::cerr << "Received gameFinish for unknown game: " << json["game"]
                << std::endl;
    }
    return;
  }
  std::cerr << "Received invalid type for /api/stream/event " << type << ": "
            << json << std::endl;
}

}  // namespace habits
