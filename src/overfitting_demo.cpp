// Demonstrates the two disciplines that separate a real backtest from a fantasy:
// (1) DEFLATING a cherry-picked Sharpe for the number of configs you tried, and
// (2) WALK-FORWARD testing out-of-sample. The flashy in-sample number shrinks
// under both — and that shrinkage IS the overfitting you must account for.

#include "backtest/Backtester.h"
#include "backtest/CsvLoader.h"
#include "backtest/Statistics.h"
#include "backtest/WalkForward.h"
#include "backtest/strategies/MovingAverageCross.h"

#include <cstdio>
#include <string>
#include <vector>

using namespace bt;

static std::vector<double> returnsOf(const std::vector<double>& equity) {
    std::vector<double> r;
    r.reserve(equity.size());
    for (std::size_t i = 1; i < equity.size(); ++i)
        if (equity[i - 1] > 0.0) r.push_back(equity[i] / equity[i - 1] - 1.0);
    return r;
}

int main(int argc, char** argv) {
    const std::string path = (argc > 1) ? argv[1] : "data/sample.csv";
    std::vector<Bar> bars;
    try {
        bars = loadBarsFromCsv(path);
    } catch (const std::exception& e) {
        std::printf("Error: %s\n(run from repo root)\n", e.what());
        return 1;
    }

    std::vector<MAParams> grid;
    for (int s : {5, 10, 15, 20, 30})
        for (int l : {50, 80, 110, 150, 200})
            if (s < l) grid.push_back({s, l});

    // --- The naive way: grid-search the whole sample, keep the winner ---------
    double bestSharpe = -1e18;
    MAParams best{};
    Metrics bestMetrics{};
    std::vector<double> trialSharpes, bestReturns;
    for (const MAParams& p : grid) {
        MovingAverageCross strat(p.shortW, p.longW);
        const BacktestResult r = runBacktest(bars, strat);
        const std::vector<double> rr = returnsOf(r.equityCurve);
        const double sp = perPeriodSharpe(rr);
        trialSharpes.push_back(sp);
        if (sp > bestSharpe) { bestSharpe = sp; best = p; bestReturns = rr; bestMetrics = r.metrics; }
    }
    std::printf("Grid search over %zu MA-cross configs (full sample):\n", grid.size());
    std::printf("   best config       : MA(%d/%d)\n", best.shortW, best.longW);
    std::printf("   in-sample Sharpe  : %.2f (annualised)   <-- looks great\n\n", bestMetrics.sharpe);

    // --- Discipline 1: deflate for the fact that we tried N configs -----------
    const double psr = probabilisticSharpe(bestReturns, 0.0);
    const double dsr = deflatedSharpe(bestReturns, trialSharpes);
    std::printf("...but we cherry-picked the best of %zu trials:\n", grid.size());
    std::printf("   PSR   P[true Sharpe > 0]        : %.3f\n", psr);
    std::printf("   DSR   deflated for best-of-N     : %.3f   <-- the honest confidence\n\n", dsr);

    // --- Discipline 2: honest out-of-sample via walk-forward -----------------
    const WalkForwardResult wf = walkForwardMA(bars, grid, 252, 63);
    std::printf("Walk-forward (train 252 / test 63, re-optimised each step):\n");
    std::printf("   OOS return        : %+.1f%%\n", wf.metrics.totalReturn * 100);
    std::printf("   OOS Sharpe        : %.2f (annualised)\n", wf.metrics.sharpe);
    std::printf("   OOS max drawdown  : %.1f%%\n", wf.metrics.maxDrawdown * 100);
    std::printf("   re-optimisations  : %zu windows\n\n", wf.chosen.size());

    std::printf("Takeaway: the in-sample Sharpe shrinks once deflated for the number of\n"
                "trials AND tested out-of-sample. That gap is exactly the overfitting a\n"
                "systematic desk expects you to measure, not hide.\n");
    return 0;
}
