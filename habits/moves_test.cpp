#include "moves.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include "position.hpp"

namespace habits {

namespace {

TEST(MovesTest, CalculateLegalMovesWhitePawns) {
  Position p = Position::FromFen("8/3p4/8/6Pp/8/1p2K2p/P3P2P/8 w - h6 0 1");
  nlohmann::json json = calculateLegalMoves(p);

  EXPECT_THAT(json["a2"], testing::UnorderedElementsAre("a3", "a4", "b3"));
  EXPECT_THAT(json["e2"], testing::IsEmpty());
  EXPECT_THAT(json["h2"], testing::IsEmpty());
  EXPECT_THAT(json["g5"], testing::UnorderedElementsAre("h6", "g6"));
}

TEST(MovesTest, CalculateLegalMovesBlackPawns) {
  Position p = Position::FromFen("8/3p4/8/8/Pp6/2P5/P7/8 b - a3 0 1");
  nlohmann::json json = calculateLegalMoves(p);

  EXPECT_THAT(json["d7"], testing::UnorderedElementsAre("d6", "d5"));
  EXPECT_THAT(json["b4"], testing::UnorderedElementsAre("a3", "b3", "c3"));
}

TEST(MovesTest, CalculateLegalMovesKnight) {
  Position p = Position::FromFen("7N/8/3P1P2/2P3P1/4N3/N7/8/8 w - - 0 1");
  nlohmann::json json = calculateLegalMoves(p);

  EXPECT_THAT(json["a3"],
              testing::UnorderedElementsAre("b1", "c2", "c4", "b5"));
  EXPECT_THAT(json["h8"], testing::UnorderedElementsAre("g6", "f7"));
  EXPECT_THAT(json["e4"],
              testing::UnorderedElementsAre("d2", "c3", "f2", "g3"));
}

TEST(MovesTest, CalculateLegalMovesRook) {
  Position p = Position::FromFen("4Q2R/8/p3RP1p/8/p7/8/8/R3R3 w - - 0 1");
  nlohmann::json json = calculateLegalMoves(p);

  EXPECT_THAT(json["a1"], testing::UnorderedElementsAre("a2", "a3", "a4", "b1",
                                                        "c1", "d1"));
  EXPECT_THAT(json["e1"],
              testing::UnorderedElementsAre("e2", "e3", "e4", "e5", "f1", "g1",
                                            "h1", "b1", "c1", "d1"));
  EXPECT_THAT(json["e6"],
              testing::UnorderedElementsAre("e7", "e2", "e3", "e4", "e5", "a6",
                                            "b6", "c6", "d6"));
  EXPECT_THAT(json["h8"],
              testing::UnorderedElementsAre("h6", "h7", "g8", "f8"));
}

}  // namespace
}  // namespace habits
