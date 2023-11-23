#pragma once

#include <nlohmann/json.hpp>
#include <string_view>

#include "position.hpp"

namespace habits {

nlohmann::json calculateLegalMoves(const Position& p);

int move(Position* p, std::string_view move);

}  // namespace habits
