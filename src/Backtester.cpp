#include "backtest/Backtester.h"

#include "backtest/Portfolio.h"

namespace bt {

BacktestResult runBacktest(const std::vector<Bar>& bars, Strategy& strategy,
                           double initialCash, double commissionRate) {
    Portfolio portfolio(initialCash, commissionRate);

    BacktestResult result{};
    result.initialCash = initialCash;
    result.equityCurve.reserve(bars.size());

    // The event loop. Order matters: decide, THEN execute, THEN mark to market.
    for (const auto& bar : bars) {
        const Signal signal = strategy.onBar(bar);   // uses only data up to this bar
        portfolio.handleSignal(signal, bar.close);   // fill at this bar's close
        result.equityCurve.push_back(portfolio.equity(bar.close));
    }

    result.finalEquity = result.equityCurve.empty() ? initialCash
                                                    : result.equityCurve.back();
    result.metrics = computeMetrics(result.equityCurve, portfolio.trades());
    return result;
}

} // namespace bt
