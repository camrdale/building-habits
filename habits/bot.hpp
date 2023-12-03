#pragma once

#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>

#include "position.hpp"
#include "search.hpp"

namespace habits {

class LichessGame {
 public:
  explicit LichessGame(const nlohmann::json& game, std::string token)
      : game_id_(game["gameId"].get<std::string>()),
        color_(game["color"].get<std::string>()[0]),
        token_(token) {}

  void startGame();

  std::string getGameId() { return game_id_; }

  void receiveGameState(std::string data);

 private:
  void initializeState(const nlohmann::json& state);
  void updateState(const nlohmann::json& state);
  bool myTurn() const;
  void makeBestMove();

  std::string game_id_;
  char color_;
  std::string token_;

  Game game_;
  std::string initial_fen_ =
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  Position position_ = Position::FromFen(
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  std::vector<std::string> moves_;
  std::string status_;
};

class LichessBot {
 public:
  explicit LichessBot(std::string token) : token_(token) {}

  int listenForChallenges();

  void receiveIncomingEvent(std::string data);

 private:
  void acceptChallenge(std::string challenge_id);
  bool rejectChallenge(nlohmann::json challenge);

  std::string token_;

  std::unique_ptr<LichessGame> current_game_;
  std::unique_ptr<std::thread> current_game_thread_;
};

}  // namespace habits
