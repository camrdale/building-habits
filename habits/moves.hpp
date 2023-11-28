#pragma once

#include <map>
#include <nlohmann/json.hpp>
#include <string_view>

#include "position.hpp"

namespace habits {

// Determine the possible moves for the active color in the Position.
// Possible moves have not been verified to not result in check, so they may not
// be legal. The map's keys are the pieces and their current squares for the
// active color, the values are a bitboard of all the possible moves for the
// piece on that square. Pieces with no possible moves will not be present.
std::map<std::pair<ColoredPiece, int>, uint64_t> possibleMoves(
    const Position& p);

std::map<std::pair<ColoredPiece, int>, std::vector<int>> legalMoves(
    const Position& p);

// Determine the legal moves for the active color in the Position.
// The returned JSON maps the squares (in algebraic notation) that contain
// pieces, to the list of squares (in algeraic notation) that the piece can
// legally move to. Squares of pieces with no legal moves will not be present.
nlohmann::json legalMovesJson(const Position& p);

// Applies the move from `from_square` to `to_square` to the Position, promoting
// to `promote_to` if necessary. The active color is not changed.
int moveInternal(Position* p, int from_square, int to_square, Piece promote_to);

// Applies the move in UCI form (2-character algebraic notation for the source
// square, 2-character algebraic notation for the target square, optional
// character for the piece to promote to) to the Position. The active color is
// changed after the move is applied.
int move(Position* p, std::string_view move);

// Determine if the current active color of the Position is in check.
bool isActiveColorInCheck(const Position& p);

// Determine who controls the squares on the board.
// The map's keys are the squares of the board (squares that no piece can attack
// are not present). The values are the difference in the inverse value of
// attackers of the square for each side, positive meaning the active color has
// more inverse value for attackers of the square, negative if the opponent has
// more.
std::map<int, int> controlSquares(const Position& p);

// Calculated inverse value of the piece as an attacker.
// Pawns are valued the most, Kings the least.
int inverseAttackValue(int piece);

}  // namespace habits
