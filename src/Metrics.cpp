#include "backtest/Metrics.h"

#include <cmath>
#include <cstddef>

namespace bt {

double maxDrawdown(const std::vector<double>& equity) {
    if (equity.empty()) return 0.0;
    double peak  = equity.front();
    double worst = 0.0;
    for (double e : equity) {
        if (e > peak) peak = e;
        if (peak > 0.0) {
            const double dd = (peak - e) / peak;       // how far below the running peak
            if (dd > worst) worst = dd;
        }
    }
    return worst;
}

double sharpeRatio(const std::vector<double>& equity, double periodsPerYear) {
    if (equity.size() < 3) return 0.0;

    std::vector<double> returns;
    returns.reserve(equity.size() - 1);
    for (std::size_t i = 1; i < equity.size(); ++i)
        if (equity[i - 1] > 0.0)
            returns.push_back(equity[i] / equity[i - 1] - 1.0);
    if (returns.size() < 2) return 0.0;

    double mean = 0.0;
    for (double r : returns) mean += r;
    mean /= returns.size();

    double var = 0.0;
    for (double r : returns) var += (r - mean) * (r - mean);
    var /= (returns.size() - 1);                        // sample variance

    const double sd = std::sqrt(var);
    if (sd <= 0.0) return 0.0;
    return (mean / sd) * std::sqrt(periodsPerYear);     // annualise, risk-free = 0
}

Metrics computeMetrics(const std::vector<double>& equity, int trades, double periodsPerYear) {
    Metrics m{};
    m.trades = trades;
    if (equity.size() < 2) return m;

    const double initial = equity.front();
    const double final    = equity.back();
    m.totalReturn = final / initial - 1.0;

    const double years = static_cast<double>(equity.size() - 1) / periodsPerYear;
    m.annualReturn = (years > 0.0) ? std::pow(final / initial, 1.0 / years) - 1.0 : 0.0;

    m.sharpe      = sharpeRatio(equity, periodsPerYear);
    m.maxDrawdown = maxDrawdown(equity);
    return m;
}

} // namespace bt
