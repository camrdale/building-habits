#include "search.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "position.hpp"

namespace habits {

namespace {

TEST(SearchTest, TakeFreePieces) {
  EXPECT_EQ(
      Game(MIDGAME).bestMove(Position::FromFen(
          "rnb1kbnr/pppp1ppp/8/4p1q1/4P3/3P4/PPP2PPP/RNBQKBNR w KQkq - 2 3")),
      "c1g5");
}

TEST(SearchTest, DontHangFreePieces) {
  EXPECT_EQ(
      Game(MIDGAME).bestMove(Position::FromFen(
          "rnq1kbnr/ppp1pppp/b2p4/4N3/8/8/PP1P1P1P/RNB1K2R w KQkq - 0 1")),
      "e5f3");
}

TEST(SearchTest, SaveHighestValueFurthestAwayPiecesFirst) {
  EXPECT_EQ(Game(MIDGAME).bestMove(
                Position::FromFen("8/8/8/4n3/2R3Q1/8/8/8 w - - 0 1")),
            "g4d1");
  EXPECT_EQ(Game(MIDGAME).bestMove(
                Position::FromFen("8/8/2R5/4n3/2R5/8/8/8 w - - 0 1")),
            "c6c5");
  EXPECT_EQ(Game(MIDGAME).bestMove(
                Position::FromFen("8/8/2r5/4N3/2r5/8/8/8 b - - 0 1")),
            "c4c5");
}

TEST(SearchTest, InitialPawnMoves) {
  EXPECT_EQ(Game(INITIAL).bestMove(Position::FromFen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")),
            "e2e4");
}

TEST(SearchTest, DevelopingMoves) {
  EXPECT_EQ(
      Game(INITIAL).bestMove(Position::FromFen(
          "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1")),
      "g1f3");
}

TEST(SearchTest, TakingFreeDefendedPiece) {
  EXPECT_EQ(Game(INITIAL).bestMove(
                Position::FromFen("2rqr1k1/1ppbbppR/2n1pn2/3pN3/p2P1P2/2PBP1Q1/"
                                  "PP1N2PP/R1B3K1 b - - 1 15")),
            "f6h7");
}

}  // namespace
}  // namespace habits
