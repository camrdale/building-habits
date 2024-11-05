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

// A representation of a single square on a chess board.
struct Square {
  // The index of the square, starting at 0 for the bottome right (white's
  // Queen-side Rook's square) to 63 in the top left (black's King-side Rook's
  // square).
  int index;

  // Create a Square initialized to a square on a board;
  Square(int index) : index(index) {}

  // Create an uninitialized Square that represents no square on a board;
  Square() : index(-1) {}

  // Create a square from the rank and file of a board.
  Square(int rank, int file) : index(8 * (rank - 1) + file - 1) {}

  // Create a square from algebraic notation (e.g. "e4") for a square.
  Square(std::string_view algebraic) :
      index((algebraic[1] - '1') * 8 + (algebraic[0] - 'a')) {}

  // The rank (rows, 1-8) of the square on a board.
  int Rank() const {
    return index / 8 + 1;
  }

  // The file (columns, 1-8) of the square on a board.
  int File() const {
    return index % 8 + 1;
  }

  // Whether the Square has been initialized.
  bool IsSet() const {
    return index != -1;
  }

  // Create a bitboard with a single bit set for this square.
  uint64_t BitboardMask() const {
    return 1ull << index;
  }

  // Get the algebraic notation (e.g. "e4") for a square.
  std::string Algebraic() const;

  // Advance to the next square on the board, returns true if there is a next
  // square. Calling this on the unitialized Square advances to the 0 square.
  bool Next() {
    if (index >= 63) {
      return false;
    }
    index++;
    return true;
  }

  bool operator<(const Square& other) const {
    return index < other.index;
  }

  bool operator==(const Square& other) const {
    return this->index == other.index;
  }

  friend std::ostream& operator<<(std::ostream& stream, const Square& square) {
    return stream << square.Algebraic();
  }
};

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
  // The square that a pawn passed over in the previous move, that can be the
  // target of en passant. Initially unset.
  Square en_passant_target_square;
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
