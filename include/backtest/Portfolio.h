#pragma once

#include "backtest/Strategy.h"

namespace bt {

// A simple long/flat portfolio: it is either fully invested (long) or fully in
// cash. A Buy signal deploys all cash; a Sell signal liquidates to cash. Each
// trade pays a proportional commission. Redundant signals (Buy while already
// long, Sell while already flat) are ignored.
class Portfolio {
public:
    Portfolio(double initialCash, double commissionRate);

    void handleSignal(Signal signal, double price);

    double equity(double price) const;   // cash + shares marked at `price`
    double cash()   const { return cash_; }
    double shares() const { return shares_; }
    int    trades() const { return trades_; }

private:
    double cash_;
    double shares_     = 0.0;
    double commission_;
    int    trades_     = 0;
};

} // namespace bt
