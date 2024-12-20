#include "moves.hpp"

#include <map>
#include <set>
#include <fstream>
#include <filesystem>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include "position.hpp"

namespace habits {

namespace {

TEST(MovesTest, CalculateLegalMovesWhitePawns) {
  Position p = Position::FromFen("8/3p4/8/6Pp/8/1p2K2p/P3P2P/8 w - h6 0 1");
  nlohmann::json json = LegalMoves(p).ToJson();

  EXPECT_THAT(json["a2"], testing::UnorderedElementsAre("a3", "a4", "b3"));
  EXPECT_THAT(json["e2"], testing::IsEmpty());
  EXPECT_THAT(json["h2"], testing::IsEmpty());
  EXPECT_THAT(json["g5"], testing::UnorderedElementsAre("h6", "g6"));
}

TEST(MovesTest, CalculateLegalMovesBlackPawns) {
  Position p = Position::FromFen("8/3p4/8/8/Pp6/2P5/P7/8 b - a3 0 1");
  nlohmann::json json = LegalMoves(p).ToJson();

  EXPECT_THAT(json["d7"], testing::UnorderedElementsAre("d6", "d5"));
  EXPECT_THAT(json["b4"], testing::UnorderedElementsAre("a3", "b3", "c3"));
}

TEST(MovesTest, CalculateLegalMovesKnight) {
  Position p = Position::FromFen("7N/8/3P1P2/2P3P1/4N3/N7/8/8 w - - 0 1");
  nlohmann::json json = LegalMoves(p).ToJson();

  EXPECT_THAT(json["a3"],
              testing::UnorderedElementsAre("b1", "c2", "c4", "b5"));
  EXPECT_THAT(json["h8"], testing::UnorderedElementsAre("g6", "f7"));
  EXPECT_THAT(json["e4"],
              testing::UnorderedElementsAre("d2", "c3", "f2", "g3"));
}

TEST(MovesTest, CalculateLegalMovesRook) {
  Position p = Position::FromFen("4Q2R/8/p3RP1p/8/p7/8/8/R3R3 w - - 0 1");
  nlohmann::json json = LegalMoves(p).ToJson();

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
  nlohmann::json json = LegalMoves(p).ToJson();

  EXPECT_THAT(json["a3"], testing::UnorderedElementsAre("b2"));
  EXPECT_THAT(json["e1"], testing::UnorderedElementsAre("e2", "f2", "f1"));
  EXPECT_THAT(json["f6"], testing::IsEmpty());
  EXPECT_THAT(json["d8"], testing::UnorderedElementsAre("c8", "c7", "e8"));
  EXPECT_THAT(json["h8"], testing::UnorderedElementsAre("h7", "g8"));

  // Check castling positions.
  EXPECT_THAT(LegalMoves(Position::FromFen(
                  "8/8/8/8/8/8/1Q1N1NP1/R3K2R w KQ - 0 1")).ToJson()["e1"],
              testing::UnorderedElementsAre("e2", "d1", "f1", "c1", "g1"));
  EXPECT_THAT(LegalMoves(Position::FromFen(
                  "8/8/8/8/8/8/1Q1N1NP1/RN2K1NR w KQ - 0 1")).ToJson()["e1"],
              testing::UnorderedElementsAre("e2", "d1", "f1"));
  EXPECT_THAT(
      LegalMoves(Position::FromFen(
          "rn1qkbnr/ppp2ppp/8/1b6/8/8/PPPP1PPP/RNBQK2R w KQkq - 3 4")).ToJson()["e1"],
      testing::IsEmpty());
  EXPECT_THAT(
      LegalMoves(Position::FromFen(
          "rnbqk1nr/ppp2ppp/8/8/1b6/8/PPP1PPPP/RNBQK2R w KQkq - 3 4")).ToJson()["e1"],
      testing::UnorderedElementsAre("f1"));
}

TEST(MovesTest, CalculateLegalMovesBlackKing) {
  EXPECT_THAT(LegalMoves(Position::FromFen(
                  "r3k2r/2qn1n2/8/8/8/8/1Q1N1NP1/RN2K1NR b - - 0 1")).ToJson()["e8"],
              testing::UnorderedElementsAre("e7", "d8", "f8"));

  // Check castling positions.
  EXPECT_THAT(LegalMoves(Position::FromFen(
                  "r3k2r/2qn1n2/8/8/8/8/1Q1N1NP1/RN2K1NR b kq - 0 1")).ToJson()["e8"],
              testing::UnorderedElementsAre("e7", "d8", "f8", "c8", "g8"));
  EXPECT_THAT(
      LegalMoves(Position::FromFen(
          "r2nk1nr/2qn1n2/8/8/8/8/1Q1N1NP1/RN2K1NR b KQkq - 0 1")).ToJson()["e8"],
      testing::UnorderedElementsAre("e7", "f8"));
  EXPECT_THAT(
      LegalMoves(Position::FromFen(
          "r3kbnr/pp2pppp/8/1B6/8/8/PPPP1PPP/RNBQK2R b KQkq - 0 1")).ToJson()["e8"],
      testing::UnorderedElementsAre("d8"));
  EXPECT_THAT(
      LegalMoves(Position::FromFen(
          "r3kbnr/pppp1ppp/8/6B1/8/8/PPP1PPPP/RN1QKB1R b KQkq - 0 1")).ToJson()["e8"],
      testing::IsEmpty());
}

TEST(MovesTest, CalculateLegalMovesBishop) {
  Position p =
      Position::FromFen("1B4B1/2r4n/3n4/4B3/2B5/1R4R1/4n3/B6B w - - 0 1");
  nlohmann::json json = LegalMoves(p).ToJson();

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
  nlohmann::json json = LegalMoves(p).ToJson();

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

TEST(MovesTest, MoveBasic) {
  Position p = Position::FromFen(
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  ASSERT_EQ(move(&p, "e2e4"), 0);
  EXPECT_EQ(p.ToFen(),
            "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
  ASSERT_EQ(move(&p, "c7c6"), 0);
  EXPECT_EQ(p.ToFen(),
            "rnbqkbnr/pp1ppppp/2p5/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
  ASSERT_EQ(move(&p, "g1f3"), 0);
  EXPECT_EQ(p.ToFen(),
            "rnbqkbnr/pp1ppppp/2p5/8/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2");
  ASSERT_EQ(move(&p, "d8a5"), 0);
  EXPECT_EQ(p.ToFen(),
            "rnb1kbnr/pp1ppppp/2p5/q7/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3");
}

TEST(MovesTest, MoveCapturing) {
  Position p = Position::FromFen(
      "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2");
  ASSERT_EQ(move(&p, "e4d5"), 0);
  EXPECT_EQ(p.ToFen(),
            "rnbqkbnr/ppp1pppp/8/3P4/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 2");
  ASSERT_EQ(move(&p, "d8d5"), 0);
  EXPECT_EQ(p.ToFen(),
            "rnb1kbnr/ppp1pppp/8/3q4/8/8/PPPP1PPP/RNBQKBNR w KQkq - 0 3");
}

TEST(MovesTest, MoveCastling) {
  Position p = Position::FromFen(
      "r3k2r/pbppqppp/np3n2/2b1p3/2B1P3/NP3N2/PBPPQPPP/R3K2R w KQkq - 6 8");
  ASSERT_EQ(move(&p, "e1g1"), 0);
  EXPECT_EQ(p.ToFen(),
            "r3k2r/pbppqppp/np3n2/2b1p3/2B1P3/NP3N2/PBPPQPPP/R4RK1 b kq - 7 8");
  ASSERT_EQ(move(&p, "e8c8"), 0);
  EXPECT_EQ(p.ToFen(),
            "2kr3r/pbppqppp/np3n2/2b1p3/2B1P3/NP3N2/PBPPQPPP/R4RK1 w - - 8 9");

  p = Position::FromFen(
      "r3k2r/pbppqppp/np3n2/2b1p3/2B1P3/NP3N2/PBPPQPPP/R3K2R w KQkq - 6 8");
  ASSERT_EQ(move(&p, "a1b1"), 0);
  EXPECT_EQ(
      p.ToFen(),
      "r3k2r/pbppqppp/np3n2/2b1p3/2B1P3/NP3N2/PBPPQPPP/1R2K2R b Kkq - 7 8");
  ASSERT_EQ(move(&p, "h8f8"), 0);
  EXPECT_EQ(
      p.ToFen(),
      "r3kr2/pbppqppp/np3n2/2b1p3/2B1P3/NP3N2/PBPPQPPP/1R2K2R w Kq - 8 9");
}

TEST(MovesTest, MoveEnPassant) {
  Position p = Position::FromFen(
      "rnbqkbnr/ppppppp1/7p/4P3/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 2");
  ASSERT_EQ(move(&p, "f7f5"), 0);
  EXPECT_EQ(p.ToFen(),
            "rnbqkbnr/ppppp1p1/7p/4Pp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3");
  ASSERT_EQ(move(&p, "e5f6"), 0);
  EXPECT_EQ(p.ToFen(),
            "rnbqkbnr/ppppp1p1/5P1p/8/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 3");

  p = Position::FromFen(
      "rnbqkbnr/pp1pp1p1/5P1p/5p2/2p5/7P/PPPP1PP1/RNBQKBNR w KQkq - 0 5");
  ASSERT_EQ(move(&p, "b2b4"), 0);
  EXPECT_EQ(
      p.ToFen(),
      "rnbqkbnr/pp1pp1p1/5P1p/5p2/1Pp5/7P/P1PP1PP1/RNBQKBNR b KQkq b3 0 5");
  ASSERT_EQ(move(&p, "c4b3"), 0);
  EXPECT_EQ(p.ToFen(),
            "rnbqkbnr/pp1pp1p1/5P1p/5p2/8/1p5P/P1PP1PP1/RNBQKBNR w KQkq - 0 6");
}

TEST(MovesTest, MovePromotion) {
  Position p = Position::FromFen("3k1n2/6P1/8/8/8/8/p7/1R4K1 w - - 0 30");
  ASSERT_EQ(move(&p, "g7g8q"), 0);
  EXPECT_EQ(p.ToFen(), "3k1nQ1/8/8/8/8/8/p7/1R4K1 b - - 0 30");
  ASSERT_EQ(move(&p, "a2a1n"), 0);
  EXPECT_EQ(p.ToFen(), "3k1nQ1/8/8/8/8/8/8/nR4K1 w - - 0 31");
}

TEST(MovesTest, IsActiveColorInCheck) {
  EXPECT_EQ(isActiveColorInCheck(Position::FromFen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")),
            false);
  EXPECT_EQ(
      isActiveColorInCheck(Position::FromFen(
          "rnbqkbnr/1ppp1Qp1/p6p/4p3/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 2 4")),
      true);
  EXPECT_EQ(isActiveColorInCheck(
                Position::FromFen("4k3/4q3/8/8/8/Q7/8/4K3 w - - 20 40")),
            true);
  EXPECT_EQ(isActiveColorInCheck(
                Position::FromFen("4k3/4q3/8/8/8/Q7/4P3/4K3 b - - 20 40")),
            false);
  EXPECT_EQ(isActiveColorInCheck(
                Position::FromFen("8/8/8/3k4/1b6/2N5/8/4K3 b - - 20 40")),
            true);
}

TEST(MovesTest, ControlSquaresBasic) {
  ControlSquares control_squares(
      Position::FromFen("7r/8/8/8/8/8/8/2R5 w - - 0 1"));
  EXPECT_EQ(control_squares.ToJson()["a1"], 10);
  EXPECT_EQ(control_squares.ToJson()["b1"], 10);
  EXPECT_EQ(control_squares.ToJson().count("c1"), 0);
  EXPECT_EQ(control_squares.ToJson()["d1"], 10);
  EXPECT_EQ(control_squares.ToJson()["h1"], 5);
  EXPECT_EQ(control_squares.ToJson()["c2"], 10);
  EXPECT_EQ(control_squares.ToJson()["c3"], 10);
  EXPECT_EQ(control_squares.ToJson()["c8"], 5);
  EXPECT_EQ(control_squares.ToJson()["d8"], -10);
  EXPECT_EQ(control_squares.ToJson()["g8"], -10);
  EXPECT_EQ(control_squares.ToJson().count("h8"), 0);
  EXPECT_TRUE(control_squares.IsSafeToMove(WROOK, Square("a1")));
  EXPECT_TRUE(control_squares.IsSafeToMove(WROOK, Square("b1")));
  EXPECT_TRUE(control_squares.IsSafeToMove(WROOK, Square("d1")));
  EXPECT_FALSE(control_squares.IsSafeToMove(WROOK, Square("h1")));
  EXPECT_TRUE(control_squares.IsSafeToMove(WROOK, Square("c2")));
  EXPECT_FALSE(control_squares.IsSafeToMove(WROOK, Square("c8")));
}

TEST(MovesTest, ControlSquaresDefendPieces) {
  ControlSquares control_squares(
      Position::FromFen("6r1/8/8/8/8/3P4/6P1/5B2 w - - 0 1"));
  EXPECT_EQ(control_squares.ToJson().count("f1"), 0);
  EXPECT_EQ(control_squares.ToJson().count("g1"), 0);
  EXPECT_EQ(control_squares.ToJson()["g2"], 5);
  EXPECT_EQ(control_squares.ToJson()["e2"], 10);
  EXPECT_EQ(control_squares.ToJson()["d3"], 10);
  EXPECT_EQ(control_squares.ToJson()["c4"], 10);
  EXPECT_EQ(control_squares.ToJson().count("b5"), 0);
}

TEST(MovesTest, ControlSquaresPawnAttackers) {
  ControlSquares control_squares(
      Position::FromFen("8/8/7p/1nb5/8/4BN2/1P6/8 w - - 0 1"));
  EXPECT_EQ(control_squares.ToJson()["a3"], 1);
  EXPECT_EQ(control_squares.ToJson()["c3"], 3);
  EXPECT_EQ(control_squares.ToJson()["g5"], 1);
}

// Test suite from https://github.com/schnitzi/rampart
TEST(MovesTest, TestSuite) {
  for (const std::filesystem::directory_entry& file : std::filesystem::directory_iterator("./testdata")) {
    std::filesystem::path path = file.path();
    if (path.extension() == ".json") {
      std::ifstream f(path);
      nlohmann::json testcases = nlohmann::json::parse(f);
      for (nlohmann::json testcase : testcases["testCases"]) {
        std::set<std::string> expected_end_positions_set;
        for (nlohmann::json expected : testcase["expected"]) {
          expected_end_positions_set.emplace(expected["fen"]);
        }

        std::set<std::string> end_positions_set;
        Position start = Position::FromFen(testcase["start"]["fen"].template get<std::string>());
        nlohmann::json json = LegalMoves(start).ToJson();
        for (const auto& [starting_square, ending_squares] : json.items()) {
          for (const std::string& ending_square : ending_squares) {
            Position end = start.Duplicate();
            move(&end, starting_square + ending_square);
            end_positions_set.emplace(end.ToFen());
          }
        }

        EXPECT_THAT(end_positions_set, testing::ContainerEq(expected_end_positions_set))
            << path.filename() << ": " << testcases["description"] 
            << " failed for starting position: " << testcase["start"]["description"];
      }
    }
  }
}

}  // namespace
}  // namespace habits
