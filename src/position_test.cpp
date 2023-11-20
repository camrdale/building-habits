#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <nlohmann/json.hpp>

#include "position.hpp"

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
  Position p = Position::FromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

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

TEST(PositionTest, CalculateLegalMovesWhitePawns) {
  Position p = Position::FromFen("8/3p4/8/6Pp/8/1p2K2p/P3P2P/8 w - h6 0 1");
  nlohmann::json json = calculateLegalMoves(p);

  EXPECT_THAT(json["a2"], testing::UnorderedElementsAre("a3", "a4", "b3"));
  EXPECT_THAT(json["e2"], testing::IsEmpty());
  EXPECT_THAT(json["h2"], testing::IsEmpty());
  EXPECT_THAT(json["g5"], testing::UnorderedElementsAre("h6", "g6"));
}

TEST(PositionTest, CalculateLegalMovesBlackPawns) {
  Position p = Position::FromFen("8/3p4/8/8/Pp6/2P5/P7/8 b - a3 0 1");
  nlohmann::json json = calculateLegalMoves(p);

  EXPECT_THAT(json["d7"], testing::UnorderedElementsAre("d6", "d5"));
  EXPECT_THAT(json["b4"], testing::UnorderedElementsAre("a3", "b3", "c3"));
}

TEST(PositionTest, CalculateLegalMovesKnight) {
  Position p = Position::FromFen("7N/8/3P1P2/2P3P1/4N3/N7/8/8 w - - 0 1");
  nlohmann::json json = calculateLegalMoves(p);

  EXPECT_THAT(json["a3"], testing::UnorderedElementsAre("b1", "c2", "c4", "b5"));
  EXPECT_THAT(json["h8"], testing::UnorderedElementsAre("g6", "f7"));
  EXPECT_THAT(json["e4"], testing::UnorderedElementsAre("d2", "c3", "f2", "g3"));
}

} // namespace
} // namespace habits
