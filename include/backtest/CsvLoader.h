#pragma once

#include "backtest/Bar.h"

#include <string>
#include <vector>

namespace bt {

// Load OHLCV bars from a CSV whose header is: Date,Open,High,Low,Close,Volume
// (the format Yahoo Finance exports). Throws std::runtime_error if unreadable.
std::vector<Bar> loadBarsFromCsv(const std::string& path);

} // namespace bt
