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

}  // namespace

std::string Square::Algebraic() const {
  std::string an;
  an += static_cast<char>('a' + this->index % 8);
  an += static_cast<char>('1' + this->index / 8);
  return an;
}

Piece parsePromotion(char promotion) {
  int piece = FEN_PIECES.find(std::toupper(promotion));
  if (piece == std::string_view::npos) {
    return PAWN;
  }
  return static_cast<Piece>(piece);
}

char toPromotion(Piece piece) {
  return std::tolower(FEN_PIECES[piece]);
}

bool Position::IsDraw() const {
  if (halfmove_clock >= 100) {
    return true;
  }
  for (int piece = WPAWN; piece < BKING; piece++) {
    if (piece != WKING && bitboards[piece] != 0ull) {
      return false;
    }
  }
  return true;
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
    int file = c - 'a' + 1;
    c = *it;
    it++;
    int rank = c - '1' + 1;
    p.en_passant_target_square = Square(rank, file);
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

  if (en_passant_target_square.IsSet()) {
    fen += ' ';
    fen += en_passant_target_square.Algebraic();
  } else {
    fen += " -";
  }

  fen += " " + std::to_string(halfmove_clock);
  fen += " " + std::to_string(fullmove_number);

  return fen;
}

Position Position::ForOpponent() const {
  Position p = Duplicate();
  p.active_color = active_color == WHITE ? BLACK : WHITE;
  p.en_passant_target_square = Square();
  return p;
}

Position Position::Duplicate() const {
  Position p;
  p.active_color = active_color;
  for (int piece = 0; piece < 12; piece++) {
    p.bitboards[piece] = bitboards[piece];
  }
  p.castling[0] = castling[0];
  p.castling[1] = castling[1];
  p.castling[2] = castling[2];
  p.castling[3] = castling[3];
  p.en_passant_target_square = en_passant_target_square;
  p.halfmove_clock = halfmove_clock;
  p.fullmove_number = fullmove_number;
  return p;
}

}  // namespace habits
