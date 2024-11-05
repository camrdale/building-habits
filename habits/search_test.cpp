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

TEST(SearchTest, AttackKnightsBishopsOnB4B5G4G5) {
  EXPECT_EQ(
      Game(INITIAL).bestMove(Position::FromFen(
          "rnbqk1nr/pppp1ppp/8/4p3/1b2P3/2N5/PPPP1PPP/R1BQKBNR w KQkq - 0 1")),
      "a2a3");
  EXPECT_EQ(
      Game(INITIAL).bestMove(Position::FromFen(
          "rn1qkbnr/ppp1pppp/3p4/8/4P1b1/5N2/PPPP1PPP/RNBQKB1R w KQkq - 0 1")),
      "h2h3");
  EXPECT_EQ(
      Game(INITIAL).bestMove(Position::FromFen(
          "rnbqkbnr/pppp1ppp/8/1N2p3/8/8/PPPPPPPP/R1BQKBNR b KQkq - 0 1")),
      "a7a6");
  EXPECT_EQ(
      Game(INITIAL).bestMove(Position::FromFen(
          "rnbqkbnr/ppp1pppp/8/3p2N1/8/8/PPPPPPPP/RNBQKB1R b KQkq - 0 1")),
      "h7h6");
  // Taking free pieces overrides the attack.
  EXPECT_EQ(
      Game(INITIAL).bestMove(Position::FromFen(
          "rnbqkbnr/pppp1ppp/8/4p1N1/8/8/PPPPPPPP/RNBQKB1R b KQkq - 0 1")),
      "d8g5");
}

TEST(SearchTest, CastleAsSoonAsPossible) {
  EXPECT_EQ(
      Game(INITIAL).bestMove(Position::FromFen(
          "r1bqk1nr/pppp1ppp/2n5/2b1p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 1")),
      "e1g1");
  EXPECT_EQ(
      Game(INITIAL).bestMove(Position::FromFen(
          "r1bqk2r/pppp1ppp/2n2n2/2b1p3/2B1P3/2N2N2/PPPP1PPP/R1BQ2KR b KQkq - 0 1")),
      "e8g8");
}

}  // namespace
}  // namespace habits
