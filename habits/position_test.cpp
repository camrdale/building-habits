#include "position.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

namespace habits {

namespace {

TEST(PositionTest, AlgebraicNotation) {
  EXPECT_EQ(Square(0).Algebraic(), "a1");
  EXPECT_EQ(Square(1).Algebraic(), "b1");
  EXPECT_EQ(Square(8).Algebraic(), "a2");
  EXPECT_EQ(Square(55).Algebraic(), "h7");
  EXPECT_EQ(Square(56).Algebraic(), "a8");
  EXPECT_EQ(Square(63).Algebraic(), "h8");
}

TEST(PositionTest, ParseAlgebraicNotation) {
  EXPECT_EQ(Square("a1").index, 0);
  EXPECT_EQ(Square("b1").index, 1);
  EXPECT_EQ(Square("a2").index, 8);
  EXPECT_EQ(Square("h7").index, 55);
  EXPECT_EQ(Square("a8").index, 56);
  EXPECT_EQ(Square("h8").index, 63);
}

TEST(PositionTest, ParsePromotion) {
  EXPECT_EQ(parsePromotion('q'), QUEEN);
  EXPECT_EQ(parsePromotion('n'), KNIGHT);
  EXPECT_EQ(parsePromotion('N'), KNIGHT);
  EXPECT_EQ(parsePromotion('?'), QUEEN);
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

TEST(PositionTest, IsDraw) {
  EXPECT_EQ(Position::FromFen("8/7k/7P/8/8/8/8/4K3 b - - 56 199").IsDraw(),
            false);
  EXPECT_EQ(Position::FromFen("8/8/7k/8/8/8/8/4K3 w - - 56 199").IsDraw(),
            true);
  EXPECT_EQ(Position::FromFen("8/7k/7P/8/8/8/8/4K3 b - - 100 199").IsDraw(),
            true);
}

TEST(PositionTest, ForOpponent) {
  Position p = Position::FromFen("8/3p2p1/8/8/8/8/P2P3P/8 b - - 56 199");
  Position opponent = p.ForOpponent();

  EXPECT_EQ(opponent.bitboards[WPAWN], p.bitboards[WPAWN]);
  EXPECT_EQ(opponent.bitboards[BPAWN], p.bitboards[BPAWN]);
  EXPECT_EQ(opponent.active_color, WHITE);
  EXPECT_EQ(opponent.halfmove_clock, p.halfmove_clock);
  EXPECT_EQ(opponent.fullmove_number, p.fullmove_number);
}

TEST(PositionTest, Duplicate) {
  Position p = Position::FromFen("8/3p2p1/8/8/8/8/P2P3P/8 b - - 56 199");
  Position copy = p.Duplicate();

  EXPECT_EQ(copy.bitboards[WPAWN], p.bitboards[WPAWN]);
  EXPECT_EQ(copy.bitboards[BPAWN], p.bitboards[BPAWN]);
  EXPECT_EQ(copy.active_color, p.active_color);
  EXPECT_EQ(copy.halfmove_clock, p.halfmove_clock);
  EXPECT_EQ(copy.fullmove_number, p.fullmove_number);
}

}  // namespace
}  // namespace habits
