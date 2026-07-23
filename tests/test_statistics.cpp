#include "backtest/Statistics.h"
#include "backtest/WalkForward.h"

#include <gtest/gtest.h>

#include <vector>

using namespace bt;

TEST(Statistics, NormalCdfAndInverseRoundTrip) {
    EXPECT_NEAR(normalCdf(0.0), 0.5, 1e-12);
    EXPECT_NEAR(inverseNormalCdf(0.5), 0.0, 1e-6);
    EXPECT_NEAR(inverseNormalCdf(0.975), 1.959964, 1e-4);   // the classic 1.96
    for (double p : {0.1, 0.3, 0.5, 0.8, 0.95})
        EXPECT_NEAR(normalCdf(inverseNormalCdf(p)), p, 1e-6);
}

TEST(Statistics, SkewAndKurtosis) {
    EXPECT_NEAR(skewness({1, 2, 3, 4, 5}), 0.0, 1e-9);       // symmetric -> 0
    EXPECT_GT(skewness({1, 1, 1, 1, 10}), 0.0);              // right tail -> positive
    EXPECT_GT(kurtosis({-5, 0, 0, 0, 0, 0, 0, 5}), 3.0);     // fat tails -> > 3
}

TEST(Statistics, ProbabilisticSharpeIsMonotonic) {
    std::vector<double> r;
    for (int i = 0; i < 200; ++i) r.push_back((i % 2 ? 0.012 : -0.004));  // net positive
    const double low  = probabilisticSharpe(r, -5.0);
    const double mid  = probabilisticSharpe(r,  0.0);
    const double high = probabilisticSharpe(r,  5.0);
    EXPECT_GT(low, mid);
    EXPECT_GT(mid, high);
    EXPECT_NEAR(low, 1.0, 1e-3);
    EXPECT_NEAR(high, 0.0, 1e-3);
}

TEST(Statistics, DeflatedSharpeNotAboveUndeflated) {
    std::vector<double> r;
    for (int i = 0; i < 300; ++i) r.push_back((i % 3 ? 0.01 : -0.008));
    std::vector<double> trials = {0.02, 0.05, 0.03, 0.08, 0.01, 0.06, 0.04, 0.07};
    const double dsr = deflatedSharpe(r, trials);
    const double psr0 = probabilisticSharpe(r, 0.0);
    EXPECT_LE(dsr, psr0 + 1e-9);      // deflation can only lower confidence
    EXPECT_GE(dsr, 0.0);
    EXPECT_LE(dsr, 1.0);
}

TEST(WalkForward, ProducesOutOfSampleCurve) {
    // A rising synthetic series so the optimiser has something to chew on.
    std::vector<Bar> bars;
    double v = 100.0;
    for (int i = 0; i < 800; ++i) { v *= (i % 5 == 0 ? 0.995 : 1.004); bars.push_back(Bar{"d", v, v, v, v, 1}); }
    std::vector<MAParams> grid = {{5, 20}, {10, 50}, {20, 100}};
    const WalkForwardResult wf = walkForwardMA(bars, grid, 252, 63);
    EXPECT_FALSE(wf.oosEquity.empty());
    EXPECT_FALSE(wf.chosen.empty());
    EXPECT_EQ(wf.lastWindowTrialSharpes.size(), grid.size());
}
