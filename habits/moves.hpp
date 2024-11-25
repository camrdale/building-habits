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

  // Returns true if the piece is a pawn in a position to promote.
  bool CanPromote() const {
    if (piece == WPAWN && square.Rank() == 7) {
      return true;
    }
    if (piece == BPAWN && square.Rank() == 2) {
      return true;
    }
    return false;
  }

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

// A destination square for a piece move, with an optional piece to promote to.
struct PieceMove {
  Square square;
  // Optional promotion piece, will be PAWN if there is no promotion.
  Piece promote_to;

  PieceMove() : square(Square()), promote_to(PAWN) {}
  PieceMove(const Square& square) : square(square), promote_to(PAWN) {}
  PieceMove(const Square& square, Piece promote_to) : square(square), promote_to(promote_to) {}

  // Whether the Square has been initialized.
  bool IsSet() const {
    return square.IsSet();
  }

  bool operator==(const PieceMove& other) const {
    return square == other.square && promote_to == other.promote_to;
  }

  std::string Algebraic() const {
    std::string algebraic = square.Algebraic();
    if (promote_to != PAWN) {
      algebraic += toPromotion(promote_to);
    }
    return algebraic;
  }
  
  friend std::ostream& operator<<(std::ostream& stream, const PieceMove& move) {
    return stream << move.Algebraic();
  }
};

// Representation for a piece on a square, and all the moves it can make from there.
struct PieceMoves {
  PieceOnSquare piece_on_square;
  std::vector<PieceMove> moves;

  PieceMoves(const PieceOnSquare& piece_on_square, const std::vector<PieceMove>& moves) :
      piece_on_square(piece_on_square), moves(moves) {}
};

// All the legal moves for the active color in the given position.
class LegalMoves {
 public:
  LegalMoves(const Position& p);

  // Sort so highest value pieces furthest away are considered first.
  std::vector<PieceMoves> Sorted() const;

  // Check if there is a legal move for a piece on a square to a destination square.
  bool IsLegal(PieceOnSquare piece_on_square, Square to_square) const;

  // Choose a piece and a move for it at random. The returned PieceMoves
  // will always contain only a single move.
  PieceMoves RandomMove() const;

  // Convert the legal moves for the active color in the Position to JSON.
  // The returned JSON maps the squares (in algebraic notation) that contain
  // pieces, to the list of squares (in algeraic notation) that the piece can
  // legally move to. Squares of pieces with no legal moves will not be present.
  nlohmann::json ToJson() const;

 private:
  // The current active color in the position.
  Color active_color_;
  // A map of the pieces on their squares, to the squares the piece can
  // legally move to.
  std::map<PieceOnSquare, std::vector<PieceMove>> legal_moves_;
};

// Applies the move in UCI form (2-character algebraic notation for the source
// square, 2-character algebraic notation for the target square, optional
// character for the piece to promote to) to the Position. The active color is
// changed after the move is applied.
int move(Position* p, std::string_view move);

// Determine if the current active color of the Position is in check.
bool isActiveColorInCheck(const Position& p);

struct ControlValues {
  // The most valuable piece that the controller can have on the square safely,
  // positive meaning the active color controls the square, negative if the
  // opponent controls it.
  int safe_piece;
  // The most valuable piece that the controller can move to the square safely.
  int safe_move;

  ControlValues(int safe_piece, int safe_move) :
      safe_piece(safe_piece), safe_move(safe_move) {}
};

// Determine who controls the squares on the board.
class ControlSquares {
 public:
  ControlSquares(const Position &p);

  // Check if the given square is safe for the piece to move to.
  bool IsSafeToMove(ColoredPiece piece, const Square& square) const;

  // Check if the piece on its current square is being attacked.
  bool IsPieceAttacked(const PieceOnSquare& piece_on_square) const;

  // Find the safest move for a piece from a list of legal moves. Returns an
  // unset Square if there are no safe moves for the piece.
  PieceMove SafestMove(ColoredPiece piece, const std::vector<PieceMove>& moves) const;

  // Find the best take of an opponent's piece from a list of legal moves.
  // Returns an unset Square if there are no safe takes for the piece.
  PieceMove BestTake(ColoredPiece piece, const std::vector<PieceMove>& moves) const;

  // Find the best way to sack a piece for an opponent's piece.
  // Returns an unset Square if there are no available sacks for the piece.
  PieceMove BestSack(ColoredPiece piece, const std::vector<PieceMove>& moves) const;

  // Find the first hanging opponent's piece from a list of legal moves.
  // Returns an unset Square if there are no hanging opponent pieces.
  PieceMove FirstHanging(ColoredPiece piece, const std::vector<PieceMove>& moves) const;

  // Find all the trades available for the given piece from a list of legal moves.
  PieceMoves Trades(const PieceOnSquare& piece_on_square, const std::vector<PieceMove>& moves) const;

  // Convert the control of the squares on the board to JSON format.
  // The keys are the squares of the board (squares that no piece can attack
  // are not present). The values are the most valuable piece that the controller
  // can have on the square safely, positive meaning the active color controls
  // the square, negative if the opponent controls it.
  nlohmann::json ToJson() const;

  // Calculated value of the piece, for control squares.
  // Kings are valued the most, Pawns the least.
  static int pieceValue(int piece);

 private:
  // Get the value of any opponent's piece on the square. Returns 0 if there
  // is no opponent piece on the square.
  int getOpponentPieceValue(Square square) const;

  // The current position.
  Position p_;
  // The map's keys are the squares of the board (squares that no piece can attack
  // are not present). The values are a pair, with the first value being the most
  // valuable piece that the controller can have on the square safely, positive
  // meaning the active color controls the square, negative if the opponent
  // controls it. The second value is the most valuable piece that the controller
  // can move to the square safely.
  std::map<Square, ControlValues> control_squares_;
};

}  // namespace habits
