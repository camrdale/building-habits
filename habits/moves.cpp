#include "moves.hpp"

#include <algorithm>
#include <bitset>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <string_view>
#include <utility>

#include "position.hpp"

namespace habits {

namespace {

// TIP: to view bitboards, see https://tearth.dev/bitboard-viewer/ (Layout 1)

constexpr uint64_t A_FILE = 0x101010101010101ull;
constexpr uint64_t RANK_1 = 0xffull;
constexpr uint64_t AB_FILES = A_FILE | (A_FILE << 1);
constexpr uint64_t GH_FILES = (A_FILE << 7) | (A_FILE << 6);
constexpr uint64_t RANK_12 = RANK_1 | (RANK_1 << 8);
constexpr uint64_t RANK_78 = (RANK_1 << 56) | (RANK_1 << 48);
constexpr uint64_t DIAGONAL_UP = 0x8040201008040201ull;
constexpr uint64_t DIAGONAL_DOWN = 0x102040810204080ull;

constexpr uint64_t KNIGHT_MOVES_C3 = 0xa1100110aull;
constexpr uint64_t KING_MOVES_B2 = 0x70507ull;

}  // namespace

std::map<PieceOnSquare, uint64_t> possibleMoves(
    const Position& p) {
  std::map<PieceOnSquare, uint64_t> legal;

  uint64_t active_pieces = 0ull;
  uint64_t opponent_pieces = 0ull;
  for (int piece = 0; piece < 12; piece++) {
    if ((p.active_color == WHITE && piece < 6) ||
        (p.active_color == BLACK && piece >= 6)) {
      active_pieces |= p.bitboards[piece];
    } else {
      opponent_pieces |= p.bitboards[piece];
    }
  }
  uint64_t all_pieces = active_pieces | opponent_pieces;
  uint64_t open_squares = ~all_pieces;
  uint64_t pawn_attack = opponent_pieces;
  if (p.en_passant_target_square >= 0) {
    pawn_attack |= (1ull << p.en_passant_target_square);
  }

  int starting_piece = p.active_color == WHITE ? 0 : 6;
  for (int piece = starting_piece; piece < starting_piece + 6; piece++) {
    uint64_t board = p.bitboards[piece];
    int square = 0;
    uint64_t mask = 1ull;
    while (square < 64) {
      if ((board & mask) != 0ull) {
        // There's a `piece` on `square`.
        int rank = square / 8 + 1;
        int file = square % 8 + 1;
        uint64_t move_board = 0ull;

        if (piece == WPAWN) {
          // Load board with attack squares.
          if (file != 1) {
            move_board |= (mask << 7) & pawn_attack;
          }
          if (file != 8) {
            move_board |= (mask << 9) & pawn_attack;
          }
          // Add move squares.
          uint64_t move_one = (mask << 8) & open_squares;
          if (move_one != 0ull) {
            move_board |= move_one;
            if (rank == 2) {
              move_board |= (mask << 16) & open_squares;
            }
          }

        } else if (piece == BPAWN) {
          // Load board with attack squares.
          if (file != 8) {
            move_board |= (mask >> 7) & pawn_attack;
          }
          if (file != 1) {
            move_board |= (mask >> 9) & pawn_attack;
          }
          // Add move squares.
          uint64_t move_one = (mask >> 8) & open_squares;
          if (move_one != 0ull) {
            move_board |= move_one;
            if (rank == 7) {
              move_board |= (mask >> 16) & open_squares;
            }
          }

        } else if (piece == WKNIGHT || piece == BKNIGHT) {
          // Set all the default moves for knights on C3.
          move_board = KNIGHT_MOVES_C3;
          // Shift to the actual position.
          if (rank > 3) {
            move_board <<= (rank - 3) * 8;
          } else if (rank < 3) {
            move_board >>= (3 - rank) * 8;
          }
          if (file > 3) {
            move_board <<= file - 3;
          } else if (file < 3) {
            move_board >>= 3 - file;
          }
          // Remove the ones that shifted to the other side/end of the board.
          if (rank <= 2) {
            move_board &= ~RANK_78;
          }
          if (rank >= 7) {
            move_board &= ~RANK_12;
          }
          if (file <= 2) {
            move_board &= ~GH_FILES;
          }
          if (file >= 7) {
            move_board &= ~AB_FILES;
          }
          // Remove friendly pieces.
          move_board &= ~active_pieces;

        } else if (piece == WKING || piece == BKING) {
          // Set all the default moves for kings on B2.
          move_board = KING_MOVES_B2;
          // Shift to the actual position.
          if (rank > 2) {
            move_board <<= (rank - 2) * 8;
          } else if (rank == 1) {
            move_board >>= 8;
          }
          if (file > 2) {
            move_board <<= file - 2;
          } else if (file == 1) {
            move_board >>= 1;
          }
          // Remove the ones that shifted to the other side/end of the board.
          if (rank == 1) {
            move_board &= ~RANK_78;
          }
          if (rank == 8) {
            move_board &= ~RANK_12;
          }
          if (file == 1) {
            move_board &= ~GH_FILES;
          }
          if (file == 8) {
            move_board &= ~AB_FILES;
          }
          // Remove friendly pieces.
          move_board &= ~active_pieces;
          // Add in castling moves
          if (piece == WKING) {
            if (p.castling[WOO] && (0x60ull & all_pieces) == 0ull) {
              Position tmpP = p.Duplicate();
              tmpP.bitboards[WKING] |= 0x70ull;
              if (!isActiveColorInCheck(tmpP)) {
                move_board |= 0x40ull;
              }
            }
            if (p.castling[WOOO] && (0xeull & all_pieces) == 0ull) {
              Position tmpP = p.Duplicate();
              tmpP.bitboards[WKING] |= 0x1cull;
              if (!isActiveColorInCheck(tmpP)) {
                move_board |= 0x4ull;
              }
            }
          } else {
            if (p.castling[BOO] &&
                (0x6000000000000000ull & all_pieces) == 0ull) {
              Position tmpP = p.Duplicate();
              tmpP.bitboards[BKING] |= 0x7000000000000000ull;
              if (!isActiveColorInCheck(tmpP)) {
                move_board |= 0x4000000000000000ull;
              }
            }
            if (p.castling[BOOO] &&
                (0xe00000000000000ull & all_pieces) == 0ull) {
              Position tmpP = p.Duplicate();
              tmpP.bitboards[BKING] |= 0x1c00000000000000ull;
              if (!isActiveColorInCheck(tmpP)) {
                move_board |= 0x400000000000000ull;
              }
            }
          }
        }

        if (piece == WROOK || piece == BROOK || piece == WQUEEN ||
            piece == BQUEEN) {
          if (rank < 8) {
            uint64_t up_move = A_FILE << (square + 8);
            uint64_t nogo_board =
                up_move & (active_pieces | (opponent_pieces << 8));
            if (nogo_board == 0ull) {
              // No blockers found in this direction so all are valid.
              move_board |= up_move;
            } else {
              // Uses __builtin_ctzll which may not work on all compilers.
              int first_nogo_square = __builtin_ctzll(nogo_board);
              uint64_t up_move_mask = A_FILE << first_nogo_square;
              move_board |= up_move & ~up_move_mask;
            }
          }

          if (file < 8) {
            uint64_t right_move =
                (RANK_1 << (square + 1)) & (RANK_1 << ((rank - 1) * 8));
            uint64_t nogo_board =
                right_move & (active_pieces | (opponent_pieces << 1));
            if (nogo_board == 0ull) {
              // No blockers found in this direction so all are valid.
              move_board |= right_move;
            } else {
              int first_nogo_square = __builtin_ctzll(nogo_board);
              uint64_t right_move_mask = RANK_1 << first_nogo_square;
              move_board |= right_move & ~right_move_mask;
            }
          }

          if (rank > 1) {
            uint64_t down_move = A_FILE >> (64 - square);
            uint64_t nogo_board =
                down_move & (active_pieces | (opponent_pieces >> 8));
            if (nogo_board == 0ull) {
              // No blockers found in this direction so all are valid.
              move_board |= down_move;
            } else {
              int last_nogo_square = 63 - __builtin_clzll(nogo_board);
              uint64_t down_move_mask = A_FILE >> (56 - last_nogo_square);
              move_board |= down_move & ~down_move_mask;
            }
          }

          if (file > 1) {
            uint64_t left_move = ((RANK_1 << 56) >> (64 - square)) &
                                 (RANK_1 << ((rank - 1) * 8));
            uint64_t nogo_board =
                left_move & (active_pieces | (opponent_pieces >> 1));
            if (nogo_board == 0ull) {
              // No blockers found in this direction so all are valid.
              move_board |= left_move;
            } else {
              int last_nogo_square = 63 - __builtin_clzll(nogo_board);
              uint64_t left_move_mask =
                  (RANK_1 << 56) >> (63 - last_nogo_square);
              move_board |= left_move & ~left_move_mask;
            }
          }
        }

        if (piece == WBISHOP || piece == BBISHOP || piece == WQUEEN ||
            piece == BQUEEN) {
          if (square < 55) {
            // Up to the right from the piece
            uint64_t up_right_move = DIAGONAL_UP << (square + 9);
            // Remove diagonal that gets shifted to the other side.
            int move_mask_square = 72 - 8 * (file - rank);
            if (move_mask_square < 64) {
              up_right_move &= ~(DIAGONAL_UP << move_mask_square);
            }
            uint64_t nogo_board =
                up_right_move & (active_pieces | (opponent_pieces << 9));
            if (nogo_board == 0ull) {
              // No blockers found in this direction so all are valid.
              move_board |= up_right_move;
            } else {
              int first_nogo_square = __builtin_ctzll(nogo_board);
              uint64_t up_right_move_mask = DIAGONAL_UP << first_nogo_square;
              move_board |= up_right_move & ~up_right_move_mask;
            }
          }

          if (square > 8) {
            // Down to the left from the piece
            uint64_t down_left_move = DIAGONAL_UP >> (63 - square + 9);
            // Remove diagonal that gets shifted to the other side.
            int move_mask_square = 8 * (rank - file) - 9;
            if (move_mask_square >= 0) {
              down_left_move &= ~(DIAGONAL_UP >> (63 - move_mask_square));
            }
            uint64_t nogo_board =
                down_left_move & (active_pieces | (opponent_pieces >> 9));
            if (nogo_board == 0ull) {
              // No blockers found in this direction so all are valid.
              move_board |= down_left_move;
            } else {
              int last_nogo_square = 63 - __builtin_clzll(nogo_board);
              uint64_t down_left_move_mask =
                  DIAGONAL_UP >> (63 - last_nogo_square);
              move_board |= down_left_move & ~down_left_move_mask;
            }
          }

          if (square < 56) {
            // Up to the left from the piece
            uint64_t up_left_move = DIAGONAL_DOWN << square;
            // Remove diagonal that gets shifted to the other side.
            int move_mask_square = 8 * (file + rank) - 9;
            if (move_mask_square < 64) {
              up_left_move &= ~(DIAGONAL_DOWN << (move_mask_square - 7));
            }
            uint64_t nogo_board =
                up_left_move & (active_pieces | (opponent_pieces << 7));
            if (nogo_board == 0ull) {
              // No blockers found in this direction so all are valid.
              move_board |= up_left_move;
            } else {
              int first_nogo_square = __builtin_ctzll(nogo_board);
              uint64_t up_left_move_mask = DIAGONAL_DOWN
                                           << (first_nogo_square - 7);
              move_board |= up_left_move & ~up_left_move_mask;
            }
          }

          if (square > 7) {
            // Down to the right from the piece
            uint64_t down_right_move = DIAGONAL_DOWN >> (63 - square);
            // Remove diagonal that gets shifted to the other side.
            int move_mask_shift = 8 * (16 - rank - file);
            if (move_mask_shift < 64) {
              down_right_move &= ~(DIAGONAL_DOWN >> move_mask_shift);
            }
            uint64_t nogo_board =
                down_right_move & (active_pieces | (opponent_pieces >> 7));
            if (nogo_board == 0ull) {
              // No blockers found in this direction so all are valid.
              move_board |= down_right_move;
            } else {
              int last_nogo_square = 63 - __builtin_clzll(nogo_board);
              uint64_t down_right_move_mask =
                  DIAGONAL_DOWN >> (63 - last_nogo_square - 7);
              move_board |= down_right_move & ~down_right_move_mask;
            }
          }
        }

        if (move_board != 0ull) {
          legal[PieceOnSquare(static_cast<ColoredPiece>(piece), square)] =
              move_board;
        }
      }
      square++;
      mask <<= 1;
    }
  }
  return legal;
}

std::map<PieceOnSquare, std::vector<int>> legalMoves(
    const Position& p) {
  std::map<PieceOnSquare, std::vector<int>> legal;

  std::map<PieceOnSquare, uint64_t> possible_move_boards =
      possibleMoves(p);
  for (const auto& [piece_and_square, move_board] : possible_move_boards) {
    if (move_board != 0ull) {
      std::vector<int> targets;
      int move_square = 0;
      uint64_t move_mask = 1ull;
      while (move_square < 64) {
        if ((move_board & move_mask) != 0ull) {
          // Try the move (promotion type can't affect check).
          Position tmpP = p.Duplicate();
          moveInternal(&tmpP, piece_and_square.square, move_square, QUEEN);
          // Don't add it if it results in being in check.
          if (!isActiveColorInCheck(tmpP)) {
            targets.push_back(move_square);
          }
        }
        move_square++;
        move_mask <<= 1;
      }
      if (p.active_color == BLACK) {
        // Move square list should be sorted from nearest to furthest.
        std::reverse(targets.begin(), targets.end());
      }
      if (!targets.empty()) {
        legal[piece_and_square] = std::move(targets);
      }
    }
  }

  return legal;
}

nlohmann::json legalMovesJson(const Position& p) {
  nlohmann::json legal;

  std::map<PieceOnSquare, uint64_t> possible_move_boards =
      possibleMoves(p);
  for (const auto& [piece_and_square, move_board] : possible_move_boards) {
    if (move_board != 0ull) {
      nlohmann::json targets = nlohmann::json::array();
      int move_square = 0;
      uint64_t move_mask = 1ull;
      while (move_square < 64) {
        if ((move_board & move_mask) != 0ull) {
          // Try the move (promotion type can't affect check).
          Position tmpP = p.Duplicate();
          moveInternal(&tmpP, piece_and_square.square, move_square, QUEEN);
          // Don't add it if it results in being in check.
          if (!isActiveColorInCheck(tmpP)) {
            targets.push_back(algebraic(move_square));
          }
        }
        move_square++;
        move_mask <<= 1;
      }
      if (!targets.empty()) {
        legal[algebraic(piece_and_square.square)] = std::move(targets);
      }
    }
  }

  return legal;
}

bool isActiveColorInCheck(const Position& p) {
  std::map<PieceOnSquare, uint64_t> opponent_moves =
      possibleMoves(p.ForOpponent());
  uint64_t king_board = p.bitboards[p.active_color == WHITE ? WKING : BKING];
  for (const auto& [square, move_board] : opponent_moves) {
    if ((king_board & move_board) != 0ull) {
      return true;
      break;
    }
  }
  return false;
}

int moveInternal(Position* p, int from_square, int to_square,
                 Piece promote_to) {
  uint64_t from_mask = 1ull << from_square;
  uint64_t to_mask = 1ull << to_square;

  // Find which peice moved.
  int starting_piece = p->active_color == WHITE ? 0 : 6;
  int piece = starting_piece;
  for (; piece < starting_piece + 6; piece++) {
    if ((from_mask & p->bitboards[piece]) != 0ull) {
      break;
    }
  }
  if (piece >= starting_piece + 6) {
    std::cout << "Failed to find a piece for " << p->active_color
              << " on square " << from_square << std::endl;
    return 1;
  }
  p->halfmove_clock++;

  // Find if an opponent peice is on the target square.
  starting_piece = p->active_color == WHITE ? 6 : 0;
  int opponent_piece = starting_piece;
  for (; opponent_piece < starting_piece + 6; opponent_piece++) {
    if ((to_mask & p->bitboards[opponent_piece]) != 0ull) {
      break;
    }
  }
  if (opponent_piece < starting_piece + 6) {
    // Remove the opponent's piece from the target square.
    p->bitboards[opponent_piece] &= ~to_mask;
    p->halfmove_clock = 0;
  }

  // Check for en passant capture.
  if (piece % 6 == PAWN && p->en_passant_target_square == to_square) {
    int en_passant_square = to_square - (p->active_color == WHITE ? 8 : -8);
    p->bitboards[6 - piece] &= ~(1ull << en_passant_square);
    p->halfmove_clock = 0;
  }
  p->en_passant_target_square = -1;

  // Remove the from square from the piece's board.
  p->bitboards[piece] &= ~from_mask;

  if (piece % 6 == PAWN && (to_square >= 56 || to_square <= 7)) {
    // Promote to queen by default
    int promotion_piece = piece + promote_to - PAWN;
    p->bitboards[promotion_piece] |= to_mask;

  } else {
    // Set the piece on the target square.
    p->bitboards[piece] |= to_mask;
  }

  // Check for castling.
  if (piece % 6 == KING && abs(from_square - to_square) == 2) {
    int rook_piece = piece - 2;
    int rook_square;
    if (from_square > to_square) {
      // O-O-O
      rook_square = from_square - 4;
    } else {
      // O-O
      rook_square = from_square + 3;
    }
    p->bitboards[rook_piece] &= ~(1ull << rook_square);
    p->bitboards[rook_piece] |= 1ull << ((from_square + to_square) / 2);
  }

  // Update castling availability, en passant and halfmove clock.
  switch (piece) {
    case WPAWN:
      p->halfmove_clock = 0;
      if (to_square - from_square == 16) {
        p->en_passant_target_square = from_square + 8;
      }
      break;
    case BPAWN:
      p->halfmove_clock = 0;
      if (to_square - from_square == -16) {
        p->en_passant_target_square = from_square - 8;
      }
      break;
    case WROOK:
      if (from_square == 0) {
        p->castling[WOOO] = false;
      } else if (from_square == 7) {
        p->castling[WOO] = false;
      }
      break;
    case BROOK:
      if (from_square == 56) {
        p->castling[BOOO] = false;
      } else if (from_square == 63) {
        p->castling[BOO] = false;
      }
      break;
    case WKING:
      p->castling[WOOO] = false;
      p->castling[WOO] = false;
      break;
    case BKING:
      p->castling[BOOO] = false;
      p->castling[BOO] = false;
      break;
  }

  if (p->active_color == BLACK) {
    p->fullmove_number++;
  }

  return 0;
}

int move(Position* p, std::string_view move) {
  int from_square = parseAlgebraic(move.substr(0, 2));
  int to_square = parseAlgebraic(move.substr(2, 2));

  // Promote to queen by default
  Piece promotion_piece = QUEEN;
  if (move.length() > 4) {
    promotion_piece = parsePromotion(move[4]);
  }

  int result = moveInternal(p, from_square, to_square, promotion_piece);
  if (result != 0) {
    return result;
  }

  if (p->active_color == WHITE) {
    p->active_color = BLACK;
  } else {
    p->active_color = WHITE;
  }
  return 0;
}

int pieceValue(int piece) {
  switch (piece % 6) {
    case PAWN:
      return 1;
    case KNIGHT:
      return 3;
    case BISHOP:
      return 3;
    case ROOK:
      return 5;
    case QUEEN:
      return 9;
    case KING:
      return 10;
  }
  return 0;
}

std::map<int, std::pair<int, int>> controlSquares(const Position& p) {
  std::map<int, std::pair<int, int>> control_squares;

  const std::map<PieceOnSquare, uint64_t> active_moves =
      possibleMoves(p);
  const std::map<PieceOnSquare, uint64_t> opponent_moves =
      possibleMoves(p.ForOpponent());

  uint64_t active_pieces = 0ull;
  uint64_t opponent_pieces = 0ull;
  for (int piece = 0; piece < 12; piece++) {
    if ((p.active_color == WHITE && piece < 6) ||
        (p.active_color == BLACK && piece >= 6)) {
      active_pieces |= p.bitboards[piece];
    } else {
      opponent_pieces |= p.bitboards[piece];
    }
  }

  int square = 0;
  uint64_t mask = 1ull;
  while (square < 64) {
    std::map<PieceOnSquare, uint64_t> temp_active_moves =
        active_moves;
    std::map<PieceOnSquare, uint64_t> temp_opponent_moves =
        opponent_moves;
    if ((mask & active_pieces) == 0ull) {
      // There's no piece on the square for the current player. Need to put one
      // there so the opponent can attack it.
      Position tempP = p.Duplicate();
      tempP.bitboards[p.active_color == WHITE ? WPAWN : BPAWN] |= mask;
      // Also make sure any piece there for the opponent is removed.
      int starting_piece = p.active_color == WHITE ? 6 : 0;
      for (int piece = starting_piece; piece < starting_piece + 6; piece++) {
        tempP.bitboards[piece] &= ~mask;
      }
      temp_opponent_moves = possibleMoves(tempP.ForOpponent());
    }
    if ((mask & opponent_pieces) == 0ull) {
      // There's no piece on the square for the opponent. Need to put one there
      // so the current player can attack it.
      Position tempP = p.Duplicate();
      tempP.bitboards[p.active_color == WHITE ? BPAWN : WPAWN] |= mask;
      // Also make sure any piece there for the current player is removed.
      int starting_piece = p.active_color == WHITE ? 0 : 6;
      for (int piece = starting_piece; piece < starting_piece + 6; piece++) {
        tempP.bitboards[piece] &= ~mask;
      }
      temp_active_moves = possibleMoves(tempP);
    }
    int defenders = 0;
    int min_defender_value = pieceValue(WKING);
    for (const auto& [piece_and_square, move_board] : temp_active_moves) {
      if ((move_board & mask) != 0ull) {
        defenders++;
        min_defender_value =
            std::min(min_defender_value, pieceValue(piece_and_square.piece));
      }
    }
    int attackers = 0;
    int min_attacker_value = pieceValue(WKING);
    for (const auto& [piece_and_square, move_board] : temp_opponent_moves) {
      if ((move_board & mask) != 0ull) {
        attackers++;
        min_attacker_value =
            std::min(min_attacker_value, pieceValue(piece_and_square.piece));
      }
    }

    if (attackers != 0 || defenders != 0) {
      bool defended = defenders - attackers >= 0;
      int min_defended_piece =
          defended ? min_attacker_value : -min_defender_value;
      if (!defended && min_defender_value < min_attacker_value) {
        min_defended_piece = min_defender_value;
      }
      // if (defended && min_attacker_value < min_defender_value) {
      //   min_defended_piece = -min_attacker_value;
      // }

      bool can_move = defenders - attackers >= 1;
      int min_move_piece = can_move ? min_attacker_value : -min_defender_value;
      if (!can_move && defenders - attackers == 0 &&
          min_defender_value != pieceValue(PAWN)) {
        min_move_piece = pieceValue(PAWN);
      }

      control_squares[square] =
          std::make_pair(min_defended_piece, min_move_piece);
    }

    square++;
    mask <<= 1;
  }

  return control_squares;
}

nlohmann::json controlSquaresJson(const Position& p) {
  nlohmann::json control_squares;
  for (auto [square, control] : controlSquares(p)) {
    control_squares[algebraic(square)] = control.first;
  }
  return control_squares;
}

}  // namespace habits
