#include "backtest/Portfolio.h"

namespace bt {

Portfolio::Portfolio(double initialCash, double commissionRate)
    : cash_(initialCash), commission_(commissionRate) {}

void Portfolio::handleSignal(Signal signal, double price) {
    if (signal == Signal::Buy && shares_ == 0.0) {
        // Deploy all cash. Reserve the commission so we don't go negative:
        // cash = notional + notional*commission  =>  notional = cash / (1+comm).
        const double notional = cash_ / (1.0 + commission_);
        shares_ = notional / price;
        cash_   = 0.0;
        ++trades_;
    } else if (signal == Signal::Sell && shares_ > 0.0) {
        const double proceeds = shares_ * price;
        cash_   += proceeds * (1.0 - commission_);
        shares_  = 0.0;
        ++trades_;
    }
    // Any other case (Hold, Buy-while-long, Sell-while-flat) is a no-op.
}

double Portfolio::equity(double price) const {
    return cash_ + shares_ * price;
}

} // namespace bt
