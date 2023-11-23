#include "position.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

namespace habits {

namespace {

TEST(PositionTest, AlgebraicNotation) {
  EXPECT_EQ(algebraic(0), "a1");
  EXPECT_EQ(algebraic(1), "b1");
  EXPECT_EQ(algebraic(8), "a2");
  EXPECT_EQ(algebraic(55), "h7");
  EXPECT_EQ(algebraic(56), "a8");
  EXPECT_EQ(algebraic(63), "h8");
}

TEST(PositionTest, FromFenStartPos) {
  Position p = Position::FromFen(
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

  EXPECT_EQ(p.bitboards[WPAWN], 0x000000000000ff00ull);
  EXPECT_EQ(p.bitboards[BPAWN], 0x00ff000000000000ull);
  EXPECT_EQ(p.bitboards[WKING], 1ull << 4);
  EXPECT_EQ(p.bitboards[BKING], 1ull << 60);
  EXPECT_EQ(p.active_color, WHITE);
  EXPECT_EQ(p.castling[0], true);
  EXPECT_EQ(p.castling[1], true);
  EXPECT_EQ(p.castling[2], true);
  EXPECT_EQ(p.castling[3], true);
  EXPECT_EQ(p.halfmove_clock, 0);
  EXPECT_EQ(p.fullmove_number, 1);
}

TEST(PositionTest, FromFenWithSpaces) {
  Position p = Position::FromFen("8/3p2p1/8/8/8/8/P2P3P/8 b - - 56 199");

  EXPECT_EQ(p.bitboards[WPAWN], (1ull << 8) | (1ull << 11) | (1ull << 15));
  EXPECT_EQ(p.bitboards[BPAWN], (1ull << 51) | (1ull << 54));
  EXPECT_EQ(p.bitboards[WKING], 0ull);
  EXPECT_EQ(p.bitboards[BKING], 0ull);
  EXPECT_EQ(p.active_color, BLACK);
  EXPECT_EQ(p.castling[0], false);
  EXPECT_EQ(p.castling[1], false);
  EXPECT_EQ(p.castling[2], false);
  EXPECT_EQ(p.castling[3], false);
  EXPECT_EQ(p.halfmove_clock, 56);
  EXPECT_EQ(p.fullmove_number, 199);
}

TEST(PositionTest, ToFen) {
  Position p = Position::FromFen("8/3p2p1/8/8/8/8/P2P3P/8 b - - 56 199");

  p.bitboards[WPAWN] = (1ull << 8) | (1ull << 11) | (1ull << 15);
  p.bitboards[BPAWN] = (1ull << 51) | (1ull << 54);
  p.active_color = BLACK;
  p.halfmove_clock = 56;
  p.fullmove_number = 199;

  EXPECT_EQ(p.ToFen(), "8/3p2p1/8/8/8/8/P2P3P/8 b - - 56 199");
}

}  // namespace
}  // namespace habits
