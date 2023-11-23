#pragma once

#include <cstdint>
#include <string>

namespace habits {

enum Piece : int {
  PAWN = 1,
  KNIGHT = 2,
  BISHOP = 3,
  ROOK = 4,
  QUEEN = 5,
  KING = 6,
};

enum Color : int {
  WHITE = 1,
  BLACK = -1,
};

enum ColoredPiece : int {
  WPAWN = 0,
  WKNIGHT = 1,
  WBISHOP = 2,
  WROOK = 3,
  WQUEEN = 4,
  WKING = 5,
  BPAWN = 6,
  BKNIGHT = 7,
  BBISHOP = 8,
  BROOK = 9,
  BQUEEN = 10,
  BKING = 11,
};

enum Castle : int {
  OO = 1,
  OOO = 2,
};

enum ColoredCastle : int {
  WOO = 0,
  WOOO = 1,
  BOO = 2,
  BOOO = 3,
};

std::string algebraic(int square);

struct Position {
  uint64_t bitboards[12] = {0};
  Color active_color = WHITE;
  bool castling[4] = {false};
  int en_passant_target_square = -1;
  int halfmove_clock = 0;
  int fullmove_number = 0;

  static Position FromFen(std::string_view fen);
  std::string ToFen() const;
};

}  // namespace habits
