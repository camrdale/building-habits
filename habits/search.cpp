#include "search.hpp"

#include <algorithm>
#include <bitset>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <utility>

#include "moves.hpp"
#include "position.hpp"

namespace habits {

namespace {

const std::vector<
    std::pair<std::pair<ColoredPiece, std::string>, std::vector<std::string>>>
    initial_moves_white = {{{WPAWN, "e2"}, {"e4"}}, {{WPAWN, "d2"}, {"d4"}}};

const std::vector<
    std::pair<std::pair<ColoredPiece, std::string>, std::vector<std::string>>>
    initial_moves_black = {{{BPAWN, "e7"}, {"e5"}}, {{BPAWN, "d7"}, {"d5"}}};

const std::vector<
    std::pair<std::pair<ColoredPiece, std::string>, std::vector<std::string>>>
    developing_moves_white = {{{WPAWN, "e2"}, {"e4", "e3"}},
                              {{WKNIGHT, "g1"}, {"f3", "e2"}},
                              {{WKNIGHT, "b1"}, {"c3", "d2"}},
                              {{WBISHOP, "f1"}, {"c4", "e2", "b5"}},
                              {{WKING, "e1"}, {"g1", "c1"}},
                              {{WPAWN, "d2"}, {"d3", "d4"}},
                              {{WBISHOP, "c1"}, {"e3", "f4", "d2", "g5"}},
                              {{WQUEEN, "d1"}, {"d2", "e2"}},
                              {{WROOK, "f1"}, {"e1"}},
                              {{WROOK, "a1"}, {"d1", "c1"}},
                              {{WPAWN, "h2"}, {"h3"}}};

const std::vector<
    std::pair<std::pair<ColoredPiece, std::string>, std::vector<std::string>>>
    developing_moves_black = {{{BPAWN, "e7"}, {"e5", "e6"}},
                              {{BKNIGHT, "b8"}, {"c6", "d7"}},
                              {{BKNIGHT, "g8"}, {"f6", "e7"}},
                              {{BBISHOP, "f8"}, {"c5", "e7", "b4"}},
                              {{BKING, "e8"}, {"g8", "c8"}},
                              {{BPAWN, "d7"}, {"d6", "d5"}},
                              {{BBISHOP, "c8"}, {"e6", "f5", "d7", "g4"}},
                              {{BQUEEN, "d8"}, {"d7", "e7"}},
                              {{BROOK, "f8"}, {"e8"}},
                              {{BROOK, "a8"}, {"d8", "c8"}},
                              {{BPAWN, "h7"}, {"h6"}}};

std::string searchPresetMoves(
    const Position& p,
    const std::map<std::pair<ColoredPiece, int>, std::vector<int>>& legal_moves,
    std::map<int, int>& control_squares,
    const std::vector<std::pair<std::pair<ColoredPiece, std::string>,
                                std::vector<std::string>>>& preset_moves) {
  for (const auto& [piece_and_square, tos] : preset_moves) {
    auto legal_moves_from = legal_moves.find(std::make_pair(
        piece_and_square.first, parseAlgebraic(piece_and_square.second)));
    if (legal_moves_from != legal_moves.end()) {
      const std::vector<int>& legal_moves_to = legal_moves_from->second;
      for (const std::string& to : tos) {
        int to_square = parseAlgebraic(to);
        int i =
            std::find(legal_moves_to.begin(), legal_moves_to.end(), to_square) -
            legal_moves_to.begin();
        if (i != legal_moves_to.size()) {
          int control = control_squares[to_square];
          // Remove the attack value of this piece from the square, since moving
          // it there will mean it no longer attacks the square. Pawns don't
          // control **preset** squares they move to.
          if ((piece_and_square.first % 6) != PAWN) {
            control -= inverseAttackValue(piece_and_square.first);
          }
          if (control >= 0) {
            std::cout << "Found preset move of " << piece_and_square.first
                      << " from " << piece_and_square.second << " to " << to
                      << std::endl;
            return piece_and_square.second + to;
          }
        }
      }
    }
  }
  return "";
}

int pieceValue(int piece) {
  switch (piece % 6) {
    case PAWN:
      return 1;
    case KNIGHT:
      return 3;
    case BISHOP:
      return 3;
    case ROOK:
      return 5;
    case QUEEN:
      return 9;
    case KING:
      return 100;
  }
  return 0;
}

}  // namespace

std::string Game::bestMove(const Position& p) {
  // Know how all the piece move.
  std::map<std::pair<ColoredPiece, int>, std::vector<int>> legal_moves =
      legalMoves(p);

  std::string bestmove;

  std::map<int, int> control_squares = controlSquares(p);
  uint64_t opponent_pieces = 0ull;
  for (int piece = 0; piece < 12; piece++) {
    if ((p.active_color == WHITE && piece >= 6) ||
        (p.active_color == BLACK && piece < 6)) {
      opponent_pieces |= p.bitboards[piece];
    }
  }

  // 1. Don't hang free pieces.
  std::vector<std::pair<std::pair<ColoredPiece, int>, std::vector<int>>>
      sorted_legal_moves(legal_moves.begin(), legal_moves.end());
  // Sort highest value pieces furthest away to save first.
  std::sort(
      sorted_legal_moves.begin(), sorted_legal_moves.end(),
      [p](std::pair<std::pair<ColoredPiece, int>, std::vector<int>> left,
          std::pair<std::pair<ColoredPiece, int>, std::vector<int>> right) {
        if (left.first.first != right.first.first) {
          return left.first.first > right.first.first;
        }
        if (p.active_color == WHITE) {
          return left.first.second < right.first.second;
        }
        return left.first.second < right.first.second;
      });
  for (const auto& [piece_and_square, move_squares] : legal_moves) {
    if ((piece_and_square.first % 6) != PAWN &&
        control_squares[piece_and_square.second] < 0) {
      int max_control = -1;
      int safest_square = -1;
      for (int move_square : move_squares) {
        int control = control_squares[move_square];
        // Remove the attack value of this piece from the square, since moving
        // it there will mean it no longer attacks the square.
        control -= inverseAttackValue(piece_and_square.first);
        if (control > max_control) {
          max_control = control;
          safest_square = move_square;
        }
      }
      if (safest_square >= 0) {
        std::cout << "Moving attacked piece " << piece_and_square.first
                  << " from " << piece_and_square.second << " (control value "
                  << control_squares[piece_and_square.second] << ") to "
                  << safest_square << " (control value " << max_control << ")"
                  << std::endl;
        return algebraic(piece_and_square.second) + algebraic(safest_square);
      }
    }
  }

  // 2. Take free pieces (pawns are not pieces).
  // Reverse sort so we attack with the lowest value pieces first.
  std::reverse(sorted_legal_moves.begin(), sorted_legal_moves.end());
  for (const auto& [piece_and_square, move_squares] : legal_moves) {
    for (int move_square : move_squares) {
      if ((opponent_pieces & (1ull << move_square)) != 0ull) {
        int control = control_squares[move_square];
        // Remove the attack value of this piece from the square, since moving
        // it there will mean it no longer attacks the square.
        control -= inverseAttackValue(piece_and_square.first);
        if (control >= 0) {
          std::cout << "Taking free piece with " << piece_and_square.first
                    << " from " << piece_and_square.second << " to "
                    << move_square << " (control value "
                    << control_squares[move_square] << ")" << std::endl;
          return algebraic(piece_and_square.second) + algebraic(move_square);
        }
      }
    }
  }

  if (stage_ == INITIAL) {
    bestmove = searchPresetMoves(
        p, legal_moves, control_squares,
        p.active_color == WHITE ? initial_moves_white : initial_moves_black);
    if (bestmove.empty()) {
      stage_ = DEVELOPING;
    } else {
      return bestmove;
    }
  }

  // 3. Capture pieces of equal or greater value whenever possible (pawns are
  // not pieces). 3a. Capture towards the center with pawns.

  // 4. Always attack a Bishop or Knight on g4/g5 b4/b5 with the a or h pawn
  // immediately.

  // 5. Castle as soon as possible.
  // 6. Make an escape square for the king once finished development.
  if (stage_ == DEVELOPING) {
    bestmove =
        searchPresetMoves(p, legal_moves, control_squares,
                          p.active_color == WHITE ? developing_moves_white
                                                  : developing_moves_black);
    if (bestmove.empty()) {
      stage_ = MIDGAME;
    } else {
      return bestmove;
    }
  }

  // 7. Active King in the endgame (11 or less piece value remain).

  // 8. Attack pawns in the endgame.

  // 9. Control the center.

  // 10. Move towards the center.

  // Spend just as much time on moves as your opponent (or less!). Use your time
  // to think, but don't get low on time.

  // Spend a lot of time at the beginning to follow all the rules.

  // Push pass pawns.

  // Give a check.

  // Random pawn moves not on the king side.

  // Nothing else? make a random move.
  auto it = legal_moves.begin();
  std::advance(it, rand() % legal_moves.size());
  std::pair<ColoredPiece, int> from = it->first;
  std::vector<int> tos = it->second;
  auto it2 = tos.begin();
  std::advance(it2, rand() % tos.size());
  int to = *it2;
  std::cout << "Randomly moving " << from.first << " from " << from.second
            << " to " << to << std::endl;
  return algebraic(from.second) + algebraic(to);
}

}  // namespace habits
