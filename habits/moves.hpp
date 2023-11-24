#pragma once

#include <map>
#include <nlohmann/json.hpp>
#include <string_view>

#include "position.hpp"

namespace habits {

// Determine the legal moves for the active color in the Position.
// The map's keys are squares that currently contain a piece for the active
// color, the values are a bitboard of all the legal moves for the piece on that
// square. Squares of pieces with no legal moves will not be present.
std::map<int, uint64_t> possibleMoves(const Position& p);

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

}  // namespace habits
