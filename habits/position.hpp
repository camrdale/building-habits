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

constexpr std::string_view FEN_PIECES = "PNBRQKpnbrqk";

constexpr uint64_t A_FILE = 0x101010101010101ull;
constexpr uint64_t RANK_1 = 0xffull;
constexpr uint64_t AB_FILES = A_FILE | (A_FILE << 1);
constexpr uint64_t GH_FILES = (A_FILE << 7) | (A_FILE << 6);
constexpr uint64_t RANK_12 = RANK_1 | (RANK_1 << 8);
constexpr uint64_t RANK_78 = (RANK_1 << 56) | (RANK_1 << 48);

constexpr uint64_t KNIGHT_MOVES_C3 = 0xa1100110aull;

enum Castle : int {
  OO = 1,
  OOO = 2,
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
