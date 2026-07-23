#pragma once

#include <vector>

namespace bt {

// Summary performance statistics for an equity curve.
struct Metrics {
    double totalReturn  = 0.0;   // final / initial - 1
    double annualReturn = 0.0;   // geometric, annualised
    double sharpe       = 0.0;   // annualised, risk-free rate assumed 0
    double maxDrawdown  = 0.0;   // worst peak-to-trough drop, as a positive fraction
    int    trades       = 0;
};

// Worst peak-to-trough decline over the curve (0.25 == a 25% drawdown).
double maxDrawdown(const std::vector<double>& equity);

// Annualised Sharpe ratio of the curve's period-over-period returns.
double sharpeRatio(const std::vector<double>& equity, double periodsPerYear = 252.0);

// Everything above, bundled.
Metrics computeMetrics(const std::vector<double>& equity, int trades,
                       double periodsPerYear = 252.0);

} // namespace bt
