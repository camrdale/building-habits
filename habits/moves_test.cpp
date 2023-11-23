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

TEST(MovesTest, CalculateLegalMovesWhiteKing) {
  Position p = Position::FromFen(
      "1n1K3K/2p1PPP1/4PKP1/4PPP1/pn6/Kn6/nn6/R3K2R w - - 0 1");
  nlohmann::json json = calculateLegalMoves(p);

  EXPECT_THAT(json["a3"],
              testing::UnorderedElementsAre("a2", "a4", "b2", "b3", "b4"));
  EXPECT_THAT(json["e1"],
              testing::UnorderedElementsAre("d1", "d2", "e2", "f2", "f1"));
  EXPECT_THAT(json["f6"], testing::IsEmpty());
  EXPECT_THAT(json["d8"],
              testing::UnorderedElementsAre("c8", "c7", "d7", "e8"));
  EXPECT_THAT(json["h8"], testing::UnorderedElementsAre("h7", "g8"));

  // Check castling positions.
  EXPECT_THAT(calculateLegalMoves(Position::FromFen(
                  "8/8/8/8/8/8/1Q1N1NP1/R3K2R w KQ - 0 1"))["e1"],
              testing::UnorderedElementsAre("e2", "d1", "f1", "c1", "g1"));
  EXPECT_THAT(calculateLegalMoves(Position::FromFen(
                  "8/8/8/8/8/8/1Q1N1NP1/RN2K1NR w KQ - 0 1"))["e1"],
              testing::UnorderedElementsAre("e2", "d1", "f1"));
}

TEST(MovesTest, CalculateLegalMovesBlackKing) {
  EXPECT_THAT(calculateLegalMoves(Position::FromFen(
                  "r3k2r/2qn1n2/8/8/8/8/1Q1N1NP1/RN2K1NR b - - 0 1"))["e8"],
              testing::UnorderedElementsAre("e7", "d8", "f8"));

  // Check castling positions.
  EXPECT_THAT(calculateLegalMoves(Position::FromFen(
                  "r3k2r/2qn1n2/8/8/8/8/1Q1N1NP1/RN2K1NR b kq - 0 1"))["e8"],
              testing::UnorderedElementsAre("e7", "d8", "f8", "c8", "g8"));
  EXPECT_THAT(
      calculateLegalMoves(Position::FromFen(
          "r2nk1nr/2qn1n2/8/8/8/8/1Q1N1NP1/RN2K1NR b KQkq - 0 1"))["e8"],
      testing::UnorderedElementsAre("e7", "f8"));
}

TEST(MovesTest, CalculateLegalMovesBishop) {
  Position p =
      Position::FromFen("1B4B1/2r4n/3n4/4B3/2B5/1R4R1/4n3/B6B w - - 0 1");
  nlohmann::json json = calculateLegalMoves(p);

  EXPECT_THAT(json["a1"], testing::UnorderedElementsAre("b2", "c3", "d4"));
  EXPECT_THAT(json["h1"], testing::UnorderedElementsAre("g2", "f3", "e4", "d5",
                                                        "c6", "b7", "a8"));
  EXPECT_THAT(json["c4"], testing::UnorderedElementsAre("d5", "e6", "f7", "d3",
                                                        "e2", "b5", "a6"));
  EXPECT_THAT(json["e5"], testing::UnorderedElementsAre(
                              "f6", "g7", "h8", "d4", "c3", "b2", "f4", "d6"));
  EXPECT_THAT(json["b8"], testing::UnorderedElementsAre("a7", "c7"));
  EXPECT_THAT(json["g8"],
              testing::UnorderedElementsAre("f7", "e6", "d5", "h7"));
}

TEST(MovesTest, CalculateLegalMovesQueen) {
  Position p =
      Position::FromFen("Qr1RRR1Q/r2RQR2/3RrR2/8/4R3/2rQ4/8/Q1R2Q2 w - - 0 1");
  nlohmann::json json = calculateLegalMoves(p);

  EXPECT_THAT(json["a1"],
              testing::UnorderedElementsAre("b2", "c3", "b1", "a2", "a3", "a4",
                                            "a5", "a6", "a7"));
  EXPECT_THAT(json["f1"],
              testing::UnorderedElementsAre("g1", "h1", "e1", "d1", "f2", "f3",
                                            "f4", "f5", "g2", "h3", "e2"));
  EXPECT_THAT(json["d3"], testing::UnorderedElementsAre(
                              "d4", "d5", "d2", "d1", "c3", "e3", "f3", "g3",
                              "h3", "c2", "b1", "e2", "c4", "b5", "a6"));
  EXPECT_THAT(json["e7"], testing::UnorderedElementsAre("e6"));
  EXPECT_THAT(json["a8"],
              testing::UnorderedElementsAre("a7", "b8", "b7", "c6", "d5"));
  EXPECT_THAT(json["h8"],
              testing::UnorderedElementsAre("g8", "g7", "h7", "h6", "h5", "h4",
                                            "h3", "h2", "h1"));
}

}  // namespace
}  // namespace habits
