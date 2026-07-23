#pragma once

#include "backtest/Bar.h"

#include <string>

namespace bt {

// What a strategy wants to do after seeing a bar.
//   Buy  = be long   (enter if flat)
//   Sell = be flat   (exit if long)
//   Hold = no change
enum class Signal { Hold, Buy, Sell };

// Base class for every trading strategy (the classic Strategy pattern).
//
// The engine calls onBar() once per bar, IN TIME ORDER, and never reveals a
// future bar. A strategy therefore physically cannot peek ahead — that is how
// this design rules out look-ahead bias, the #1 way backtests lie.
class Strategy {
public:
    virtual ~Strategy() = default;

    // Decide what to do given the newest bar (plus whatever state the strategy
    // has accumulated from earlier bars).
    virtual Signal onBar(const Bar& bar) = 0;

    // Human-readable name for reporting.
    virtual std::string name() const = 0;
};

} // namespace bt
