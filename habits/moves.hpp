#pragma once

#include <map>
#include <nlohmann/json.hpp>
#include <string_view>

#include "position.hpp"

namespace habits {

// A piece and its current position on a square on the board.
struct PieceOnSquare {
  ColoredPiece piece;
  Square square;

  PieceOnSquare(ColoredPiece piece, Square square) :
      piece(piece),
      square(square) {}

  PieceOnSquare(ColoredPiece piece, std::string algebraic_square) :
      piece(piece),
      square(algebraic_square) {}

  // Custom comparison operator for std::map.
  bool operator<(const PieceOnSquare& other) const {
      if (piece != other.piece) {
        return piece < other.piece;
      } else {
        return square < other.square;
      }
  }

  // Equals operator needed for std::find.
  bool operator==(const PieceOnSquare& other) const {
    return (this->piece == other.piece) && (this->square == other.square);
  }
};

// Determine the possible moves for the active color in the Position.
// Possible moves have not been verified to not result in check, so they may not
// be legal. The map's keys are the pieces and their current squares for the
// active color, the values are a bitboard of all the possible moves for the
// piece on that square. Pieces with no possible moves will not be present.
std::map<PieceOnSquare, uint64_t> possibleMoves(
    const Position& p);

// Representation for a piece on a square, and all the moves it can make from there.
struct PieceMoves {
  PieceOnSquare piece_on_square;
  std::vector<Square> moves;

  PieceMoves(const PieceOnSquare& piece_on_square, const std::vector<Square>& moves) :
      piece_on_square(piece_on_square), moves(moves) {}
};

class LegalMoves {
 public:
  LegalMoves(const Position& p);

  // Sort so highest value pieces furthest away are considered first.
  std::vector<PieceMoves> Sorted() const;

  // Check if there si a legal move for a piece on a square to a destination square.
  bool IsLegal(PieceOnSquare piece_on_square, Square to_square) const;

  // Choose a piece and a move for it at random. The returned PieceMoves
  // will always contain only a single move.
  PieceMoves RandomMove() const;

 private:
  Color active_color_;
  std::map<PieceOnSquare, std::vector<Square>> legal_moves_;
};

// std::map<PieceOnSquare, std::vector<Square>> legalMoves(
//     const Position& p);

// Determine the legal moves for the active color in the Position.
// The returned JSON maps the squares (in algebraic notation) that contain
// pieces, to the list of squares (in algeraic notation) that the piece can
// legally move to. Squares of pieces with no legal moves will not be present.
nlohmann::json legalMovesJson(const Position& p);

// Applies the move from `from_square` to `to_square` to the Position, promoting
// to `promote_to` if necessary. The active color is not changed.
int moveInternal(Position* p, Square from_square, Square to_square, Piece promote_to);

// Applies the move in UCI form (2-character algebraic notation for the source
// square, 2-character algebraic notation for the target square, optional
// character for the piece to promote to) to the Position. The active color is
// changed after the move is applied.
int move(Position* p, std::string_view move);

// Determine if the current active color of the Position is in check.
bool isActiveColorInCheck(const Position& p);

// Determine who controls the squares on the board.
// The map's keys are the squares of the board (squares that no piece can attack
// are not present). The values are a pair, with the first value being the most
// valuable piece that the controller can have on the square safely, positive
// meaning the active color controls the square, negative if the opponent
// controls it. The second value is the most valuable piece that the controller
// can move to the square safely.
std::map<Square, std::pair<int, int>> controlSquares(const Position& p);

// Determine who controls the squares on the board, in JSON format.
// The keys are the squares of the board (squares that no piece can attack
// are not present). The values are the most valuable piece that the controller
// can have on the square safely, positive meaning the active color controls
// the square, negative if the opponent controls it.
nlohmann::json controlSquaresJson(const Position& p);

// Calculated value of the piece, for control squares.
// Kings are valued the most, Pawns the least.
int pieceValue(int piece);

}  // namespace habits
