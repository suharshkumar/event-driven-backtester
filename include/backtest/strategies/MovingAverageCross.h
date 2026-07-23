#pragma once

#include "backtest/Strategy.h"

#include <deque>
#include <string>

namespace bt {

// Classic trend-following rule: go long when the short moving average crosses
// ABOVE the long one (a "golden cross"), go flat when it crosses back below
// (a "death cross"). Averages are kept with running sums over sliding windows,
// so each bar is O(1).
class MovingAverageCross : public Strategy {
public:
    MovingAverageCross(int shortWindow, int longWindow)
        : shortW_(shortWindow), longW_(longWindow) {}

    Signal onBar(const Bar& bar) override {
        slide(shortPrices_, shortSum_, shortW_, bar.close);
        slide(longPrices_,  longSum_,  longW_,  bar.close);

        if (static_cast<int>(longPrices_.size()) < longW_)
            return Signal::Hold;                       // still warming up

        const double shortAvg   = shortSum_ / shortPrices_.size();
        const double longAvg    = longSum_  / longPrices_.size();
        const bool   shortAbove = shortAvg > longAvg;

        Signal signal = Signal::Hold;
        if (initialised_) {
            if (shortAbove && !prevAbove_)       signal = Signal::Buy;    // golden cross
            else if (!shortAbove && prevAbove_)  signal = Signal::Sell;   // death cross
        }
        prevAbove_   = shortAbove;
        initialised_ = true;                           // baseline set on first full window
        return signal;
    }

    std::string name() const override {
        return "MA-cross(" + std::to_string(shortW_) + "/" + std::to_string(longW_) + ")";
    }

private:
    static void slide(std::deque<double>& window, double& sum, int size, double x) {
        window.push_back(x);
        sum += x;
        if (static_cast<int>(window.size()) > size) {
            sum -= window.front();
            window.pop_front();
        }
    }

    int shortW_, longW_;
    std::deque<double> shortPrices_, longPrices_;
    double shortSum_ = 0.0, longSum_ = 0.0;
    bool   prevAbove_ = false, initialised_ = false;
};

} // namespace bt
