#pragma once

#include <nlohmann/json.hpp>

#include "position.hpp"

namespace habits {

nlohmann::json calculateLegalMoves(const Position& p);

}  // namespace habits
