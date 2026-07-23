#include "backtest/Backtester.h"
#include "backtest/CsvLoader.h"
#include "backtest/strategies/MeanReversion.h"
#include "backtest/strategies/MovingAverageCross.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

using namespace bt;

// A tiny unicode sparkline of the equity curve for the terminal.
static std::string sparkline(const std::vector<double>& v, int width = 64) {
    static const char* blocks[] = {"▁","▂","▃","▄",
                                   "▅","▆","▇","█"};
    if (v.size() < 2) return "";
    std::vector<double> s;
    for (int i = 0; i < width; ++i)
        s.push_back(v[static_cast<std::size_t>(static_cast<double>(i) / (width - 1) * (v.size() - 1))]);
    const double lo = *std::min_element(s.begin(), s.end());
    const double hi = *std::max_element(s.begin(), s.end());
    std::string out;
    for (double x : s) {
        const int idx = (hi > lo) ? static_cast<int>((x - lo) / (hi - lo) * 7) : 0;
        out += blocks[idx];
    }
    return out;
}

static void report(const std::string& title, const BacktestResult& r) {
    const Metrics& m = r.metrics;
    std::printf("%-22s | return %+7.1f%% | annual %+6.1f%% | Sharpe %5.2f | maxDD %5.1f%% | trades %d\n",
                title.c_str(), m.totalReturn * 100, m.annualReturn * 100,
                m.sharpe, m.maxDrawdown * 100, m.trades);
}

int main(int argc, char** argv) {
    const std::string path = (argc > 1) ? argv[1] : "data/sample.csv";

    std::vector<Bar> bars;
    try {
        bars = loadBarsFromCsv(path);
    } catch (const std::exception& e) {
        std::printf("Error: %s\n(run from the repo root so data/sample.csv is found)\n", e.what());
        return 1;
    }
    std::printf("Loaded %zu bars from %s   (close %.2f -> %.2f)\n\n",
                bars.size(), path.c_str(), bars.front().close, bars.back().close);

    // Buy-and-hold benchmark: what you'd get doing nothing.
    std::vector<double> hold;
    hold.reserve(bars.size());
    const double c0 = bars.front().close;
    for (const auto& b : bars) hold.push_back(100000.0 * b.close / c0);
    BacktestResult holdResult{};
    holdResult.equityCurve = hold;
    holdResult.initialCash = 100000.0;
    holdResult.finalEquity = hold.back();
    holdResult.metrics     = computeMetrics(hold, 0);

    MovingAverageCross ma(20, 50);
    MeanReversion      mr(20, 1.0);
    const BacktestResult maResult = runBacktest(bars, ma);
    const BacktestResult mrResult = runBacktest(bars, mr);

    std::printf("%-22s | %s\n", "Strategy", "Performance vs. doing nothing");
    std::printf("--------------------------------------------------------------------------------------\n");
    report("Buy & Hold", holdResult);
    report(ma.name(), maResult);
    report(mr.name(), mrResult);

    std::printf("\nEquity curve, %s:\n  %s\n", ma.name().c_str(), sparkline(maResult.equityCurve).c_str());
    return 0;
}
