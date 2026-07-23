#pragma once

#include "backtest/Bar.h"
#include "backtest/Metrics.h"

#include <vector>

namespace bt {

struct MAParams {
    int shortW;
    int longW;
};

struct WalkForwardResult {
    std::vector<double>   oosEquity;            // stitched out-of-sample equity curve
    Metrics               metrics;              // metrics of that OOS curve
    std::vector<MAParams> chosen;               // parameters picked for each OOS window
    std::vector<double>   lastWindowTrialSharpes; // grid Sharpes from the final IS window
};

// Rolling walk-forward optimisation for the moving-average-cross strategy.
// For each step it picks the best `grid` parameters on an in-sample window
// (by Sharpe), then trades them on the NEXT, unseen out-of-sample window — the
// honest test of whether a parameter choice generalises rather than overfits.
WalkForwardResult walkForwardMA(const std::vector<Bar>& bars,
                                const std::vector<MAParams>& grid,
                                int trainWindow, int testWindow,
                                double initialCash = 100000.0,
                                double commission = 0.0005);

} // namespace bt
