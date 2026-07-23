#pragma once

#include "backtest/Strategy.h"

#include <cmath>
#include <deque>
#include <string>

namespace bt {

// Mean-reversion rule: measure how many standard deviations the latest price is
// from its rolling mean (a z-score). Buy when the price is unusually CHEAP
// (z below -entry), sell when it is unusually RICH (z above +entry). Mean and
// variance are kept incrementally with running sum / sum-of-squares.
class MeanReversion : public Strategy {
public:
    MeanReversion(int window, double zEntry)
        : window_(window), zEntry_(zEntry) {}

    Signal onBar(const Bar& bar) override {
        prices_.push_back(bar.close);
        sum_   += bar.close;
        sumSq_ += bar.close * bar.close;
        if (static_cast<int>(prices_.size()) > window_) {
            const double old = prices_.front();
            prices_.pop_front();
            sum_   -= old;
            sumSq_ -= old * old;
        }
        if (static_cast<int>(prices_.size()) < window_)
            return Signal::Hold;                       // not enough history yet

        const double n    = static_cast<double>(prices_.size());
        const double mean = sum_ / n;
        const double var  = (sumSq_ - sum_ * sum_ / n) / (n - 1.0);
        const double sd   = std::sqrt(var > 0.0 ? var : 0.0);
        if (sd <= 0.0)
            return Signal::Hold;

        const double z = (bar.close - mean) / sd;
        if (z < -zEntry_) return Signal::Buy;          // cheap -> buy
        if (z >  zEntry_) return Signal::Sell;         // rich  -> sell
        return Signal::Hold;
    }

    std::string name() const override {
        return "MeanReversion(win=" + std::to_string(window_) + ")";
    }

private:
    int    window_;
    double zEntry_;
    std::deque<double> prices_;
    double sum_ = 0.0, sumSq_ = 0.0;
};

} // namespace bt
