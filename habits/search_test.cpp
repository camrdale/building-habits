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

TEST(SearchTest, InitialPawnMoves) {
  EXPECT_EQ(
      Game(INITIAL).bestMove(Position::FromFen(
          "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")),
      "e2e4");
}

TEST(SearchTest, DevelopingMoves) {
  EXPECT_EQ(
      Game(INITIAL).bestMove(Position::FromFen(
          "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1")),
      "g1f3");
}

}  // namespace
}  // namespace habits
