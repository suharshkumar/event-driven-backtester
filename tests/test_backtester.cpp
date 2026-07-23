#include "backtest/Backtester.h"
#include "backtest/CsvLoader.h"
#include "backtest/Metrics.h"
#include "backtest/Portfolio.h"
#include "backtest/strategies/MeanReversion.h"
#include "backtest/strategies/MovingAverageCross.h"

#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <vector>

using namespace bt;

static Bar barAt(double close) { return Bar{"2020-01-01", close, close, close, close, 1000}; }

// --- Metrics ----------------------------------------------------------------

TEST(Metrics, MaxDrawdownIsExact) {
    EXPECT_NEAR(maxDrawdown({100, 120, 90, 110}), 0.25, 1e-12);  // (120-90)/120
    EXPECT_NEAR(maxDrawdown({100, 110, 121}),      0.0,  1e-12); // only ever rises
}

TEST(Metrics, TotalAndAnnualReturn) {
    const Metrics m = computeMetrics({100, 150}, 1);
    EXPECT_NEAR(m.totalReturn, 0.5, 1e-12);
}

TEST(Metrics, SharpeIsPositiveForUptrend) {
    std::vector<double> equity;
    double v = 100.0;
    for (int i = 0; i < 60; ++i) { v *= (i % 2 ? 1.004 : 1.012); equity.push_back(v); }
    EXPECT_GT(sharpeRatio(equity), 0.0);
}

// --- Portfolio --------------------------------------------------------------

TEST(Portfolio, BuyThenSellRealisesProfit) {
    Portfolio p(100000.0, 0.0);            // no commission
    p.handleSignal(Signal::Buy, 100.0);    // 1000 shares
    EXPECT_NEAR(p.shares(), 1000.0, 1e-9);
    EXPECT_NEAR(p.cash(),      0.0, 1e-9);
    p.handleSignal(Signal::Sell, 110.0);   // +10%
    EXPECT_NEAR(p.cash(), 110000.0, 1e-6);
    EXPECT_EQ(p.trades(), 2);
}

TEST(Portfolio, RedundantSignalsAreNoOps) {
    Portfolio p(100000.0, 0.0);
    p.handleSignal(Signal::Sell, 100.0);   // flat already -> nothing
    EXPECT_EQ(p.trades(), 0);
    p.handleSignal(Signal::Buy, 100.0);
    p.handleSignal(Signal::Buy, 100.0);    // already long -> nothing
    EXPECT_EQ(p.trades(), 1);
}

TEST(Portfolio, CommissionReducesProceeds) {
    Portfolio p(100000.0, 0.001);          // 10 bps
    p.handleSignal(Signal::Buy, 100.0);
    p.handleSignal(Signal::Sell, 100.0);   // round trip at the same price
    EXPECT_LT(p.equity(100.0), 100000.0);  // must have lost the commissions
}

// --- Strategy ---------------------------------------------------------------

TEST(Strategy, MaCrossFiresGoldenCross) {
    MovingAverageCross ma(2, 4);
    const std::vector<double> prices = {10, 10, 10, 10, 11, 12, 13, 14, 15, 16};
    bool sawBuy = false;
    for (double px : prices)
        if (ma.onBar(barAt(px)) == Signal::Buy) sawBuy = true;
    EXPECT_TRUE(sawBuy);
}

// --- End to end -------------------------------------------------------------

TEST(Backtester, MakesMoneyEnteringAnUptrend) {
    std::vector<Bar> bars;
    for (int i = 0; i < 40; ++i) bars.push_back(barAt(100.0));      // flat, sets baseline
    double v = 100.0;
    for (int i = 0; i < 160; ++i) { v *= 1.01; bars.push_back(barAt(v)); }  // then trends up

    MovingAverageCross ma(5, 20);
    const BacktestResult r = runBacktest(bars, ma, 100000.0, 0.0);
    EXPECT_GT(r.finalEquity, 100000.0);    // golden cross -> long -> profits
    EXPECT_GT(r.metrics.trades, 0);
}

TEST(CsvLoader, ParsesRowsCorrectly) {
    const char* path = "test_tmp_bars.csv";
    {
        std::ofstream f(path);
        f << "Date,Open,High,Low,Close,Volume\n";
        f << "2021-01-01,10,11,9,10.5,1000\n";
        f << "2021-01-02,10.5,12,10,11.5,2000\n";
    }
    const std::vector<Bar> bars = loadBarsFromCsv(path);
    std::remove(path);

    ASSERT_EQ(bars.size(), 2u);
    EXPECT_EQ(bars[0].date, "2021-01-01");
    EXPECT_NEAR(bars[0].close, 10.5, 1e-9);
    EXPECT_NEAR(bars[1].high, 12.0, 1e-9);
}
