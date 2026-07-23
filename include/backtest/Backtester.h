#pragma once

#include "backtest/Bar.h"
#include "backtest/Metrics.h"
#include "backtest/Strategy.h"

#include <vector>

namespace bt {

struct BacktestResult {
    std::vector<double> equityCurve;   // portfolio value after each bar
    Metrics             metrics;
    double              initialCash = 0.0;
    double              finalEquity = 0.0;
};

// The event loop. Feeds bars to `strategy` one at a time, executes each signal
// at that bar's close, marks the portfolio to market, and records the equity
// curve. Because the strategy only ever sees the current/earlier bars, the
// result is free of look-ahead bias.
BacktestResult runBacktest(const std::vector<Bar>& bars, Strategy& strategy,
                           double initialCash = 100000.0, double commissionRate = 0.0005);

} // namespace bt
