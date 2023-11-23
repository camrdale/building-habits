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

}  // namespace habits
