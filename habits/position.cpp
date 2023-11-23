#include "position.hpp"

#include <bitset>
#include <cctype>
#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>

namespace habits {

namespace {

constexpr std::string_view FEN_PIECES = "PNBRQKpnbrqk";

} // namespace

std::string algebraic(int square) {
  std::string an;
  an += static_cast<char>('a' + square % 8);
  an += static_cast<char>('1' + square / 8);
  return an;
}

int parseAlgebraic(std::string_view algebraic) {
  return (algebraic[1] - '1') * 8 + (algebraic[0] - 'a');
}

Piece parsePromotion(char promotion) {
  int piece = FEN_PIECES.find(std::toupper(promotion));
  if (piece == std::string_view::npos) {
    // Promote to queen by default.
    return QUEEN;
  }
  return static_cast<Piece>(piece);
}

Position Position::FromFen(std::string_view fen) {
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
        p.castling[WOO] = true;
        break;
      case 'Q':
        p.castling[WOOO] = true;
        break;
      case 'k':
        p.castling[BOO] = true;
        break;
      case 'q':
        p.castling[BOOO] = true;
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

std::string Position::ToFen() const {
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

}  // namespace habits
