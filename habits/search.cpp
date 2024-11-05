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
    const std::map<PieceOnSquare, std::vector<int>>& legal_moves,
    std::map<int, std::pair<int, int>>& control_squares,
    const std::vector<std::pair<std::pair<ColoredPiece, std::string>,
                                std::vector<std::string>>>& preset_moves) {
  for (const auto& [piece_and_square, tos] : preset_moves) {
    auto legal_moves_from = legal_moves.find(PieceOnSquare(
        piece_and_square.first, parseAlgebraic(piece_and_square.second)));
    if (legal_moves_from != legal_moves.end()) {
      const std::vector<int>& legal_moves_to = legal_moves_from->second;
      for (const std::string& to : tos) {
        int to_square = parseAlgebraic(to);
        int i =
            std::find(legal_moves_to.begin(), legal_moves_to.end(), to_square) -
            legal_moves_to.begin();
        if (i != legal_moves_to.size()) {
          bool controlled = control_squares.count(to_square) > 0;
          int control =
              controlled ? control_squares[to_square].second : pieceValue(KING);
          if (control >= pieceValue(piece_and_square.first)) {
            std::cout << "Found preset move of " << piece_and_square.first
                      << " from " << piece_and_square.second << " to " << to
                      << " control value " << control << std::endl;
            return piece_and_square.second + to;
          }
        }
      }
    }
  }
  return "";
}

int getOpponentPieceValue(const Position& p, int move_square) {
  int starting_piece = p.active_color == WHITE ? 6 : 0;
  uint64_t move_mask = 1ull << move_square;
  int opponent_piece;
  for (opponent_piece = starting_piece; opponent_piece < starting_piece + 6;
       opponent_piece++) {
    if ((p.bitboards[opponent_piece] & move_mask) != 0ull) {
      return pieceValue(opponent_piece);
    }
  }
  return 0;
}

}  // namespace

std::string Game::bestMove(const Position& p) {
  // Know how all the pieces move.
  std::map<PieceOnSquare, std::vector<int>> legal_moves =
      legalMoves(p);
  std::vector<std::pair<PieceOnSquare, std::vector<int>>>
      sorted_legal_moves(legal_moves.begin(), legal_moves.end());
  // Sort so highest value pieces furthest away are considered first.
  std::sort(
      sorted_legal_moves.begin(), sorted_legal_moves.end(),
      [p](std::pair<PieceOnSquare, std::vector<int>> left,
          std::pair<PieceOnSquare, std::vector<int>> right) {
        if (left.first.piece != right.first.piece) {
          return left.first.piece > right.first.piece;
        }
        if (p.active_color == WHITE) {
          return left.first.square > right.first.square;
        }
        return left.first.square < right.first.square;
      });

  std::string bestmove;

  std::map<int, std::pair<int, int>> control_squares = controlSquares(p);

  // 1. Don't hang free pieces.
  for (const auto& [piece_and_square, move_squares] : sorted_legal_moves) {
    bool controlled = control_squares.count(piece_and_square.square) > 0;
    if (controlled && control_squares[piece_and_square.square].first <
                          pieceValue(piece_and_square.piece)) {
      int max_control = -1;
      int max_control_square = -1;
      int max_opponent_piece_value = 0;
      int max_opponent_piece_value_square = -1;
      int max_opponent_piece_value_square_control = -1;
      for (int move_square : move_squares) {
        bool move_controlled = control_squares.count(move_square) > 0;
        int control = move_controlled ? control_squares[move_square].second
                                      : pieceValue(KING);
        if (control >= pieceValue(piece_and_square.piece) &&
            control > max_control) {
          max_control = control;
          max_control_square = move_square;
        }

        int opponent_piece_value = getOpponentPieceValue(p, move_square);
        if (opponent_piece_value > max_opponent_piece_value) {
          max_opponent_piece_value = opponent_piece_value;
          max_opponent_piece_value_square = move_square;
          max_opponent_piece_value_square_control = control;
        }
      }
      if (max_opponent_piece_value >= pieceValue(piece_and_square.piece) ||
          (max_opponent_piece_value_square_control >=
           pieceValue(piece_and_square.piece))) {
        std::cout << "Moving attacked piece " << piece_and_square.piece
                  << " from " << piece_and_square.square
                  << " to take piece of value " << max_opponent_piece_value
                  << " on square " << max_opponent_piece_value_square
                  << " (control value "
                  << max_opponent_piece_value_square_control << ")"
                  << std::endl;
        return algebraic(piece_and_square.square) +
               algebraic(max_opponent_piece_value_square);
      }
      if (max_control_square >= 0) {
        std::cout << "Moving attacked piece " << piece_and_square.piece
                  << " from " << piece_and_square.square << " (control value "
                  << (controlled
                          ? control_squares[piece_and_square.square].first
                          : 0)
                  << ") to " << max_control_square << " (control value "
                  << max_control << ")" << std::endl;
        return algebraic(piece_and_square.square) +
               algebraic(max_control_square);
      }
      if (max_opponent_piece_value > 0) {
        std::cout << "Sacking attacked piece " << piece_and_square.piece
                  << " from " << piece_and_square.square
                  << " to take piece of value " << max_opponent_piece_value
                  << " on square " << max_opponent_piece_value_square
                  << " (control value "
                  << max_opponent_piece_value_square_control << ")"
                  << std::endl;
        return algebraic(piece_and_square.square) +
               algebraic(max_opponent_piece_value_square);
      }
    }
  }
  // Need to consider moving other pieces to defend (block or take attackers).

  // 2. Take free pieces (pawns are not pieces).
  // Reverse sort so we attack with the lowest value pieces first.
  std::reverse(sorted_legal_moves.begin(), sorted_legal_moves.end());
  std::vector<std::pair<PieceOnSquare, int>> trades;
  for (const auto& [piece_and_square, move_squares] : sorted_legal_moves) {
    for (int move_square : move_squares) {
      int opponent_piece_value = getOpponentPieceValue(p, move_square);
      bool controlled = control_squares.count(move_square) > 0;
      int control =
          controlled ? control_squares[move_square].second : pieceValue(KING);
      if (opponent_piece_value > pieceValue(piece_and_square.piece) ||
          (opponent_piece_value > 0 &&
           control >= pieceValue(piece_and_square.piece))) {
        std::cout << "Taking free piece with " << piece_and_square.piece
                  << " from " << piece_and_square.square << " to "
                  << move_square << " (control value " << control << ")"
                  << std::endl;
        return algebraic(piece_and_square.square) + algebraic(move_square);
      }
      if (opponent_piece_value == pieceValue(piece_and_square.piece)) {
        trades.emplace_back(std::make_pair(piece_and_square, move_square));
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
  if (!trades.empty()) {
    // Trade the highest value piece first.
    auto& [piece_and_square, move_square] = trades[trades.size() - 1];
    std::cout << "Trading pieces with " << piece_and_square.piece << " from "
              << piece_and_square.square << " to " << move_square << std::endl;
    return algebraic(piece_and_square.square) + algebraic(move_square);
  }

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
  PieceOnSquare from = it->first;
  std::vector<int> tos = it->second;
  auto it2 = tos.begin();
  std::advance(it2, rand() % tos.size());
  int to = *it2;
  std::cout << "Randomly moving " << from.piece << " from " << from.square
            << " to " << to << std::endl;
  return algebraic(from.square) + algebraic(to);
}

}  // namespace habits
