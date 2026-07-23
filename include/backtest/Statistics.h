#pragma once

#include <vector>

namespace bt {

// --- distribution helpers ---------------------------------------------------
double normalCdf(double x);
double inverseNormalCdf(double p);     // quantile function (Acklam's algorithm)

// --- sample moments of a return series --------------------------------------
double skewness(const std::vector<double>& x);
double kurtosis(const std::vector<double>& x);   // non-excess (Normal = 3)
double perPeriodSharpe(const std::vector<double>& returns);

// --- overfitting-aware Sharpe statistics (Bailey & Lopez de Prado) ----------
//
// Probabilistic Sharpe Ratio: the probability that the TRUE Sharpe exceeds a
// benchmark, correcting the observed Sharpe for sample length, skew and fat tails.
double probabilisticSharpe(const std::vector<double>& returns, double srBenchmark);

// Deflated Sharpe Ratio: PSR measured against the Sharpe you'd EXPECT to see as
// the best of N random trials. If you grid-searched N configs and kept the best,
// this tells you how much of that best Sharpe is real vs. luck. `trialSharpes`
// are the (per-period) Sharpes of every configuration you tried.
double deflatedSharpe(const std::vector<double>& returns,
                      const std::vector<double>& trialSharpes);

} // namespace bt
