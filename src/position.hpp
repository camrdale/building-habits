#pragma once

#include <bitset>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

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

std::string algebraic(int square) {
  std::string an;
  an += static_cast<char>('a' + square % 8);
  an += static_cast<char>('1' + square / 8);
  return an;
}

struct Position {
  uint64_t bitboards[12] = {0};
  Color active_color = WHITE;
  bool castling[4] = {false};
  int en_passant_target_square = -1;
  int halfmove_clock = 0;
  int fullmove_number = 0;

  static Position FromFen(std::string_view fen) {
    Position p;
    auto it = fen.begin();
    int rank = 8;
    int file = 1;
    char c;
    while (it != fen.end()) {
      if (file > 8) {
        file = 1;
        rank--;
      }

      c = *it;
      it++;

      if (c == ' ') {
        break;
      }

      if (c >= '1' && c <= '8') {
        file += c - '0';
        continue;
      }
      if (c == '/') {
        continue;
      }

      int piece = FEN_PIECES.find(c);
      int i = (rank - 1) * 8 + file - 1;
      p.bitboards[piece] = p.bitboards[piece] | (1ull << i);
      file++;
    }

    c = *it;
    it++;
    if (c == 'w') {
      p.active_color = WHITE;
    } else {
      p.active_color = BLACK;
    }
    it++;

    while (it != fen.end()) {
      c = *it;
      it++;

      if (c == ' ') {
        break;
      }

      switch (c) {
        case 'K':
          p.castling[0] = true;
          break;
        case 'Q':
          p.castling[1] = true;
          break;
        case 'k':
          p.castling[2] = true;
          break;
        case 'q':
          p.castling[3] = true;
          break;
      }
    }

    c = *it;
    it++;
    if (c != '-') {
      int file = c - 'a';
      c = *it;
      it++;
      int rank = c - '1';
      p.en_passant_target_square = 8 * rank + file;
    }

    it++;
    while (it != fen.end()) {
      c = *it;
      it++;
      if (c == ' ') {
        break;
      }
      p.halfmove_clock = p.halfmove_clock * 10 + c - '0';
    }

    while (it != fen.end()) {
      c = *it;
      it++;
      if (c == ' ') {
        break;
      }
      p.fullmove_number = p.fullmove_number * 10 + c - '0';
    }

    return p;
  }

  std::string ToFen() {
    std::string fen = "";
    for (int rank = 8; rank >= 1; rank--) {
      int empty_files = 0;
      for (int file = 1; file <= 8; file++) {
        int i = (rank - 1) * 8 + file - 1;
        bool found = false;
        for (int piece = 0; piece < 12; piece++) {
          if (bitboards[piece] & (1ull << i)) {
            found = true;
            if (empty_files > 0) {
              fen += std::to_string(empty_files);
            }
            fen += FEN_PIECES[piece];
            empty_files = 0;
            break;
          }
        }
        if (!found) {
          empty_files++;
        }
      }
      if (empty_files > 0) {
        fen += std::to_string(empty_files);
      }
      if (rank > 1) {
        fen += '/';
      }
    }

    fen += ' ';
    fen += active_color == WHITE ? 'w' : 'b';

    std::string castling_string = "";
    if (castling[0]) {
      castling_string += 'K';
    }
    if (castling[1]) {
      castling_string += 'Q';
    }
    if (castling[2]) {
      castling_string += 'k';
    }
    if (castling[3]) {
      castling_string += 'q';
    }
    if (castling_string.empty()) {
      fen += " -";
    } else {
      fen += " " + castling_string;
    }

    if (en_passant_target_square >= 0) {
      fen += ' ';
      fen += algebraic(en_passant_target_square);
    } else {
      fen += " -";
    }

    fen += " " + std::to_string(halfmove_clock);
    fen += " " + std::to_string(fullmove_number);

    return fen;
  }
};

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
        switch (piece) {
          case WPAWN: {
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
            break;
          }
          case BPAWN: {
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
            break;
          }
          case WKNIGHT:
          case BKNIGHT:
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
            break;
          case WROOK:
          case BROOK:
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
            break;
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
