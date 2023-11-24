#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace habits {

enum Piece : int {
  PAWN = 0,
  KNIGHT = 1,
  BISHOP = 2,
  ROOK = 3,
  QUEEN = 4,
  KING = 5,
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

// Get the algebraic notation (e.g. "e4") for a square.
std::string algebraic(int square);
// Parse an algebraic notation to a square.
int parseAlgebraic(std::string_view algebraic);
// Parse a promotion character from UCI move notation.
Piece parsePromotion(char promotion);

struct Position {
  // Boards representing the current positions of all pieces of each
  // ColoredPiece.
  uint64_t bitboards[12] = {0ull};
  // The current active color in this position.
  Color active_color = WHITE;
  // Castling availability for each ColoredCastle.
  bool castling[4] = {false};
  // If not negative, the square that a pawn passed over in the previous move,
  // that can be the target of en passant.
  int en_passant_target_square = -1;
  // The number of halfmoves since the last capture or pawn advance.
  int halfmove_clock = 0;
  // The number of full moves, starting at 1, incrementing after Black's move.
  int fullmove_number = 0;

  // Create a Position by parsing a FEN string:
  // https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation
  static Position FromFen(std::string_view fen);

  // Convert the current Position into a FEN string.
  std::string ToFen() const;

  // Determine if the current position is drawn. Only 50-move rule and lack of
  // mating material are considered, not stalemate.
  bool IsDraw() const;

  // Create a copy of this position, but with the opponent to move.
  Position ForOpponent() const;

  // Create a copy of this position.
  Position Duplicate() const;
};

}  // namespace habits
