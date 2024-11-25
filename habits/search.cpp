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
                              {{WKNIGHT, "g1"}, {{"f3"}, {"e2"}}},
                              {{WKNIGHT, "b1"}, {{"c3"}, {"d2"}}},
                              {{WBISHOP, "f1"}, {{"c4"}, {"d3"}, {"e2"}, {"b5"}}},
                              {{WPAWN, "d2"}, {{"d3"}, {"d4"}}},
                              {{WROOK, "f1"}, {{"e1"}}},
                              {{WBISHOP, "c1"}, {{"f4"}, {"e3"}, {"d2"}, {"g5"}}},
                              {{WPAWN, "e2"}, {{"e4"}, {"e3"}}},
                              {{WQUEEN, "d1"}, {{"d2"}, {"e2"}}},
                              {{WROOK, "a1"}, {{"d1"}, {"c1"}}},
                              {{WPAWN, "h2"}, {{"h3"}}}};

const std::vector<
    std::pair<PieceOnSquare, std::vector<Square>>>
    developing_moves_black = {{{BKING, "e8"}, {{"g8"}, {"c8"}}},
                              {{BKNIGHT, "b8"}, {{"c6"}, {"d7"}}},
                              {{BKNIGHT, "g8"}, {{"f6"}, {"e7"}}},
                              {{BBISHOP, "f8"}, {{"c5"}, {"d6"}, {"e7"}, {"b4"}}},
                              {{BPAWN, "d7"}, {{"d6"}, {"d5"}}},
                              {{BROOK, "f8"}, {{"e8"}}},
                              {{BBISHOP, "c8"}, {{"f5"}, {"e6"}, {"d7"}, {"g4"}}},
                              {{BPAWN, "e7"}, {{"e5"}, {"e6"}}},
                              {{BQUEEN, "d8"}, {{"d7"}, {"e7"}}},
                              {{BROOK, "a8"}, {{"d8"}, {"c8"}}},
                              {{BPAWN, "h7"}, {{"h6"}}}};

std::string searchPresetMoves(
    const Position& p,
    const LegalMoves& legal_moves,
    const ControlSquares& control_squares,
    const std::vector<std::pair<PieceOnSquare,
                                std::vector<Square>>>& preset_moves) {
  for (const auto& [piece_and_square, tos] : preset_moves) {
    for (const Square& to_square : tos) {
      if (legal_moves.IsLegal(piece_and_square, to_square)) {
        if (control_squares.IsSafeToMove(piece_and_square.piece, to_square)) {
          std::cout << "Found preset move of " << piece_and_square.piece
                    << " from " << piece_and_square.square << " to " << to_square
                    << std::endl;
          return piece_and_square.square.Algebraic() + to_square.Algebraic();
        }
      }
    }
  }
  return "";
}

}  // namespace

std::string Game::bestMove(const Position& p) {
  std::string bestmove;

  // Know how all the pieces move.
  LegalMoves legal_moves(p);

  ControlSquares control_squares = ControlSquares(p);

  std::vector<PieceMoves> sorted_legal_moves = legal_moves.Sorted();

  // 1. Don't hang free pieces.
  for (const auto& [piece_and_square, move_squares] : sorted_legal_moves) {
    if (control_squares.IsPieceAttacked(piece_and_square)) {
      PieceMove best_take = control_squares.BestTake(piece_and_square.piece, move_squares);
      if (best_take.IsSet()) {
        std::cout << "Moving attacked piece " << piece_and_square.piece
                  << " from " << piece_and_square.square
                  << " to take piece on square " << best_take
                  << std::endl;
        return piece_and_square.square.Algebraic() +
               best_take.Algebraic();
      }

      PieceMove max_control_square = control_squares.SafestMove(piece_and_square.piece, move_squares);
      if (max_control_square.IsSet()) {
        std::cout << "Moving attacked piece " << piece_and_square.piece
                  << " from " << piece_and_square.square << " to safest square "
                  << max_control_square << std::endl;
        return piece_and_square.square.Algebraic() +
               max_control_square.Algebraic();
      }

      PieceMove best_sack = control_squares.BestSack(piece_and_square.piece, move_squares);
      if (best_sack.IsSet()) {
        std::cout << "Sacking attacked piece " << piece_and_square.piece
                  << " from " << piece_and_square.square
                  << " to take on square " << best_sack
                  << std::endl;
        return piece_and_square.square.Algebraic() +
               best_sack.Algebraic();
      }
    }
  }
  // Need to consider moving other pieces to defend (block or take attackers).

  // 2. Take free pieces (pawns are not pieces).
  // Reverse sort so we attack with the lowest value pieces first.
  std::reverse(sorted_legal_moves.begin(), sorted_legal_moves.end());
  for (const auto& [piece_and_square, move_squares] : sorted_legal_moves) {
    PieceMove first_hanging = control_squares.FirstHanging(piece_and_square.piece, move_squares);
    if (first_hanging.IsSet()) {
      std::cout << "Taking free piece with " << piece_and_square.piece
                << " from " << piece_and_square.square << " to "
                << first_hanging << std::endl;
      return piece_and_square.square.Algebraic() + first_hanging.Algebraic();
    }
  }

  // 3. Capture pieces of equal or greater value whenever possible (pawns are
  // not pieces). 3a. Capture towards the center with pawns.
  std::vector<PieceMoves> trades;
  for (const auto& [piece_and_square, move_squares] : sorted_legal_moves) {
    PieceMoves piece_trades = control_squares.Trades(piece_and_square, move_squares);
    if (!piece_trades.moves.empty()) {
      trades.emplace_back(piece_trades);
    }
  }
  if (!trades.empty()) {
    // Trade the highest value piece first.
    PieceMoves piece_trades = trades[trades.size() - 1];
    std::cout << "Trading pieces with " << piece_trades.piece_on_square.piece << " from "
              << piece_trades.piece_on_square.square << " to " << piece_trades.moves[0] << std::endl;
    return piece_trades.piece_on_square.square.Algebraic() + piece_trades.moves[0].Algebraic();
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
