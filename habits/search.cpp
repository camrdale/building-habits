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
    std::pair<PieceOnSquare, std::vector<Square>>>
    initial_moves_white = {{{WPAWN, "e2"}, {{"e4"}}}, {{WPAWN, "d2"}, {{"d4"}}}};

const std::vector<
    std::pair<PieceOnSquare, std::vector<Square>>>
    initial_moves_black = {{{BPAWN, "e7"}, {{"e5"}}}, {{BPAWN, "d7"}, {{"d5"}}}};

const std::vector<
    std::pair<PieceOnSquare, std::vector<Square>>>
    developing_moves_white = {{{WKING, "e1"}, {{"g1"}, {"c1"}}},
                              {{WPAWN, "e2"}, {{"e4"}, {"e3"}}},
                              {{WKNIGHT, "g1"}, {{"f3"}, {"e2"}}},
                              {{WKNIGHT, "b1"}, {{"c3"}, {"d2"}}},
                              {{WBISHOP, "f1"}, {{"c4"}, {"e2"}, {"b5"}}},
                              {{WPAWN, "d2"}, {{"d3"}, {"d4"}}},
                              {{WBISHOP, "c1"}, {{"e3"}, {"f4"}, {"d2"}, {"g5"}}},
                              {{WQUEEN, "d1"}, {{"d2"}, {"e2"}}},
                              {{WROOK, "f1"}, {{"e1"}}},
                              {{WROOK, "a1"}, {{"d1"}, {"c1"}}},
                              {{WPAWN, "h2"}, {{"h3"}}}};

const std::vector<
    std::pair<PieceOnSquare, std::vector<Square>>>
    developing_moves_black = {{{BKING, "e8"}, {{"g8"}, {"c8"}}},
                              {{BPAWN, "e7"}, {{"e5"}, {"e6"}}},
                              {{BKNIGHT, "b8"}, {{"c6"}, {"d7"}}},
                              {{BKNIGHT, "g8"}, {{"f6"}, {"e7"}}},
                              {{BBISHOP, "f8"}, {{"c5"}, {"e7"}, {"b4"}}},
                              {{BPAWN, "d7"}, {{"d6"}, {"d5"}}},
                              {{BBISHOP, "c8"}, {{"e6"}, {"f5"}, {"d7"}, {"g4"}}},
                              {{BQUEEN, "d8"}, {{"d7"}, {"e7"}}},
                              {{BROOK, "f8"}, {{"e8"}}},
                              {{BROOK, "a8"}, {{"d8"}, {"c8"}}},
                              {{BPAWN, "h7"}, {{"h6"}}}};

std::string searchPresetMoves(
    const Position& p,
    const LegalMoves& legal_moves,
    std::map<Square, std::pair<int, int>>& control_squares,
    const std::vector<std::pair<PieceOnSquare,
                                std::vector<Square>>>& preset_moves) {
  for (const auto& [piece_and_square, tos] : preset_moves) {
    for (const Square& to_square : tos) {
      if (legal_moves.IsLegal(piece_and_square, to_square)) {
        bool controlled = control_squares.count(to_square) > 0;
        int control =
            controlled ? control_squares[to_square].second : pieceValue(KING);
        if (control >= pieceValue(piece_and_square.piece)) {
          std::cout << "Found preset move of " << piece_and_square.piece
                    << " from " << piece_and_square.square << " to " << to_square
                    << " control value " << control << std::endl;
          return piece_and_square.square.Algebraic() + to_square.Algebraic();
        }
      }
    }
  }
  return "";
}

int getOpponentPieceValue(const Position& p, Square move_square) {
  int starting_piece = p.active_color == WHITE ? 6 : 0;
  uint64_t move_mask = move_square.BitboardMask();
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
  std::string bestmove;

  // Know how all the pieces move.
  LegalMoves legal_moves(p);

  std::map<Square, std::pair<int, int>> control_squares = controlSquares(p);

  std::vector<PieceMoves> sorted_legal_moves = legal_moves.Sorted();

  // 1. Don't hang free pieces.
  for (const auto& [piece_and_square, move_squares] : sorted_legal_moves) {
    bool controlled = control_squares.count(piece_and_square.square) > 0;
    if (controlled && control_squares[piece_and_square.square].first <
                          pieceValue(piece_and_square.piece)) {
      int max_control = -1;
      Square max_control_square;
      int max_opponent_piece_value = 0;
      Square max_opponent_piece_value_square;
      int max_opponent_piece_value_square_control = -1;
      for (Square move_square : move_squares) {
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
        return piece_and_square.square.Algebraic() +
               max_opponent_piece_value_square.Algebraic();
      }
      if (max_control_square.IsSet()) {
        std::cout << "Moving attacked piece " << piece_and_square.piece
                  << " from " << piece_and_square.square << " (control value "
                  << (controlled
                          ? control_squares[piece_and_square.square].first
                          : 0)
                  << ") to " << max_control_square << " (control value "
                  << max_control << ")" << std::endl;
        return piece_and_square.square.Algebraic() +
               max_control_square.Algebraic();
      }
      if (max_opponent_piece_value > 0) {
        std::cout << "Sacking attacked piece " << piece_and_square.piece
                  << " from " << piece_and_square.square
                  << " to take piece of value " << max_opponent_piece_value
                  << " on square " << max_opponent_piece_value_square
                  << " (control value "
                  << max_opponent_piece_value_square_control << ")"
                  << std::endl;
        return piece_and_square.square.Algebraic() +
               max_opponent_piece_value_square.Algebraic();
      }
    }
  }
  // Need to consider moving other pieces to defend (block or take attackers).

  // 2. Take free pieces (pawns are not pieces).
  // Reverse sort so we attack with the lowest value pieces first.
  std::reverse(sorted_legal_moves.begin(), sorted_legal_moves.end());
  std::vector<std::pair<PieceOnSquare, Square>> trades;
  for (const auto& [piece_and_square, move_squares] : sorted_legal_moves) {
    for (Square move_square : move_squares) {
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
        return piece_and_square.square.Algebraic() + move_square.Algebraic();
      }
      if (opponent_piece_value == pieceValue(piece_and_square.piece)) {
        trades.emplace_back(std::make_pair(piece_and_square, move_square));
      }
    }
  }

  // 3. Capture pieces of equal or greater value whenever possible (pawns are
  // not pieces). 3a. Capture towards the center with pawns.
  if (!trades.empty()) {
    // Trade the highest value piece first.
    auto& [piece_and_square, move_square] = trades[trades.size() - 1];
    std::cout << "Trading pieces with " << piece_and_square.piece << " from "
              << piece_and_square.square << " to " << move_square << std::endl;
    return piece_and_square.square.Algebraic() + move_square.Algebraic();
  }

  // 4. Always attack a Bishop or Knight on g4/g5 b4/b5 with the a or h pawn
  // immediately.
  if (((p.bitboards[BBISHOP] | p.bitboards[BKNIGHT]) & Square("b4").BitboardMask()) != 0ull
      && legal_moves.IsLegal(PieceOnSquare(WPAWN, Square("a2")), Square("a3"))) {
    return "a2a3";
  }
  if (((p.bitboards[BBISHOP] | p.bitboards[BKNIGHT]) & Square("g4").BitboardMask()) != 0ull
      && legal_moves.IsLegal(PieceOnSquare(WPAWN, Square("h2")), Square("h3"))) {
    return "h2h3";
  }
  if (((p.bitboards[WBISHOP] | p.bitboards[WKNIGHT]) & Square("b5").BitboardMask()) != 0ull
      && legal_moves.IsLegal(PieceOnSquare(BPAWN, Square("a7")), Square("a6"))) {
    return "a7a6";
  }
  if (((p.bitboards[WBISHOP] | p.bitboards[WKNIGHT]) & Square("g5").BitboardMask()) != 0ull
      && legal_moves.IsLegal(PieceOnSquare(BPAWN, Square("h7")), Square("h6"))) {
    return "h7h6";
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
  PieceMoves random_move = legal_moves.RandomMove();
  std::cout << "Randomly moving " << random_move.piece_on_square.piece
            << " from " << random_move.piece_on_square.square
            << " to " << random_move.moves[0] << std::endl;
  return random_move.piece_on_square.square.Algebraic() + random_move.moves[0].Algebraic();
}

}  // namespace habits
