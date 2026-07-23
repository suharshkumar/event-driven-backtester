#pragma once

#include <string>

namespace bt {

// One OHLCV price bar (e.g. one trading day).
struct Bar {
    std::string date;
    double open;
    double high;
    double low;
    double close;
    double volume;
};

} // namespace bt
