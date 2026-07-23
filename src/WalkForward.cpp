#include "backtest/WalkForward.h"

#include "backtest/Backtester.h"
#include "backtest/Portfolio.h"
#include "backtest/strategies/MovingAverageCross.h"

#include <algorithm>

namespace bt {

WalkForwardResult walkForwardMA(const std::vector<Bar>& bars,
                                const std::vector<MAParams>& grid,
                                int trainWindow, int testWindow,
                                double initialCash, double commission) {
    WalkForwardResult result{};
    const int n = static_cast<int>(bars.size());
    if (grid.empty() || trainWindow <= 0 || testWindow <= 0 || n < trainWindow + testWindow)
        return result;

    double runningEquity = initialCash;

    for (int start = 0; start + trainWindow + testWindow <= n; start += testWindow) {
        const int oosStart = start + trainWindow;
        const int oosEnd   = std::min(oosStart + testWindow, n);

        // --- 1) In-sample: score every grid config, keep the best by Sharpe ---
        const std::vector<Bar> inSample(bars.begin() + start, bars.begin() + oosStart);
        MAParams best = grid.front();
        double   bestSharpe = -1e18;
        std::vector<double> trialSharpes;
        trialSharpes.reserve(grid.size());

        for (const MAParams& p : grid) {
            MovingAverageCross strat(p.shortW, p.longW);
            const BacktestResult r = runBacktest(inSample, strat, initialCash, commission);
            trialSharpes.push_back(r.metrics.sharpe);
            if (r.metrics.sharpe > bestSharpe) { bestSharpe = r.metrics.sharpe; best = p; }
        }
        result.chosen.push_back(best);
        result.lastWindowTrialSharpes = trialSharpes;   // keep the most recent grid

        // --- 2) Out-of-sample: trade the chosen config on the NEXT window -----
        // Warm the strategy (and let it carry a position) on the tail of the
        // training window; only record equity once we cross into OOS.
        MovingAverageCross strat(best.shortW, best.longW);
        Portfolio pf(runningEquity, commission);
        const int warmupStart = std::max(0, oosStart - best.longW);
        for (int j = warmupStart; j < oosEnd; ++j) {
            const Signal sig = strat.onBar(bars[j]);
            pf.handleSignal(sig, bars[j].close);
            if (j >= oosStart)
                result.oosEquity.push_back(pf.equity(bars[j].close));
        }
        if (!result.oosEquity.empty())
            runningEquity = result.oosEquity.back();
    }

    result.metrics = computeMetrics(result.oosEquity, static_cast<int>(result.chosen.size()));
    return result;
}

} // namespace bt
