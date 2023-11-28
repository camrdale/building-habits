#pragma once

#include <string>

#include "position.hpp"

namespace habits {

enum Stage : int { INITIAL, DEVELOPING, MIDGAME, ENDGAME };

class Game {
 public:
  explicit Game(Stage stage = INITIAL) : stage_(stage) {}

  void opponentMove(std::string move) { lastMove_ = move; }
  std::string bestMove(const Position& p);

 private:
  Stage stage_;
  std::string lastMove_;
};

}  // namespace habits
