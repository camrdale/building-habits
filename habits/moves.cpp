#include "moves.hpp"

#include <bitset>
#include <cstdint>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>

#include "position.hpp"

namespace habits {

namespace {

constexpr uint64_t A_FILE = 0x101010101010101ull;
constexpr uint64_t RANK_1 = 0xffull;
constexpr uint64_t AB_FILES = A_FILE | (A_FILE << 1);
constexpr uint64_t GH_FILES = (A_FILE << 7) | (A_FILE << 6);
constexpr uint64_t RANK_12 = RANK_1 | (RANK_1 << 8);
constexpr uint64_t RANK_78 = (RANK_1 << 56) | (RANK_1 << 48);

constexpr uint64_t KNIGHT_MOVES_C3 = 0xa1100110aull;
constexpr uint64_t KING_MOVES_B2 = 0x70507ull;
constexpr uint64_t WOO_CASTLE_SQUARES = 0x60ull;
constexpr uint64_t WOOO_CASSQUARES = 0x60ull;

}  // namespace

nlohmann::json calculateLegalMoves(const Position& p) {
  nlohmann::json legal;

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
          // TODO: check for opponents attacking castling squares
          if (piece == WKING) {
            if (p.castling[WOO] && (0x60ull & all_pieces) == 0ull) {
              move_board |= 0x40ull;
            }
            if (p.castling[WOOO] && (0xeull & all_pieces) == 0ull) {
              move_board |= 0x4ull;
            }
          } else {
            if (p.castling[BOO] &&
                (0x6000000000000000ull & all_pieces) == 0ull) {
              move_board |= 0x4000000000000000ull;
            }
            if (p.castling[BOOO] &&
                (0xe00000000000000ull & all_pieces) == 0ull) {
              move_board |= 0x400000000000000ull;
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
        }

        if (move_board != 0ull) {
          nlohmann::json targets = nlohmann::json::array();
          int move_square = 0;
          uint64_t move_mask = 1ull;
          while (move_square < 64) {
            if ((move_board & move_mask) != 0ull) {
              targets.push_back(algebraic(move_square));
            }
            move_square++;
            move_mask <<= 1;
          }
          legal[algebraic(square)] = std::move(targets);
        }
      }
      square++;
      mask <<= 1;
    }
  }
  return legal;
}

}  // namespace habits
