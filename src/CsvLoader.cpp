#include "backtest/CsvLoader.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace bt {

std::vector<Bar> loadBarsFromCsv(const std::string& path) {
    std::ifstream in(path);
    if (!in)
        throw std::runtime_error("cannot open CSV file: " + path);

    std::vector<Bar> bars;
    std::string line;
    bool isHeader = true;

    while (std::getline(in, line)) {
        if (line.empty()) continue;
        if (isHeader) { isHeader = false; continue; }  // skip the header row

        std::stringstream ss(line);
        std::string cell;
        Bar bar{};

        std::getline(ss, bar.date, ',');
        std::getline(ss, cell, ','); bar.open   = std::stod(cell);
        std::getline(ss, cell, ','); bar.high   = std::stod(cell);
        std::getline(ss, cell, ','); bar.low    = std::stod(cell);
        std::getline(ss, cell, ','); bar.close  = std::stod(cell);
        std::getline(ss, cell, ','); bar.volume = cell.empty() ? 0.0 : std::stod(cell);

        bars.push_back(bar);
    }
    return bars;
}

} // namespace bt
