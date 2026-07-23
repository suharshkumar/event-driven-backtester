# Event-Driven Backtesting Engine (C++17)

A backtesting framework that runs pluggable trading **strategies** over historical
price bars, simulates fills with commission, and reports the metrics a quant
actually looks at — **Sharpe ratio, max drawdown, annualised return** — plus an
equity curve. Built around an event loop that structurally **prevents look-ahead bias**.

## Architecture

```
CSV bars ──▶ Backtester (event loop) ──▶ Strategy.onBar()  ──▶ Signal
                     │                                            │
                     ▼                                            ▼
              mark-to-market  ◀──────────────  Portfolio.handleSignal()
                     │
                     ▼
              equity curve ──▶ Metrics (Sharpe, drawdown, return)
```

- **`Strategy`** — an abstract base class (the Strategy pattern). New strategies
  just subclass it and implement `onBar()`. Two are included:
  `MovingAverageCross` (trend following) and `MeanReversion` (z-score).
- **`Portfolio`** — long/flat cash accounting with proportional commission.
- **`Backtester`** — the event loop: decide → execute at the close → mark to market.
- **`Metrics`** — Sharpe, max drawdown, total & annualised return.

## No look-ahead bias — by construction

Look-ahead bias (letting a backtest "see" the future) is the #1 way results lie.
Here the engine hands the strategy **one bar at a time, in order, and never a
future bar** — so a strategy physically cannot peek ahead. Signals are also filled
at the *same* bar's close they were computed from, never an earlier price.

## Results (`./build/demo data/sample.csv`, 756-day synthetic series)

| Strategy | Total return | Annualised | Sharpe | Max drawdown | Trades |
|---|---|---|---|---|---|
| Buy & Hold | +28.9% | +8.8% | 0.53 | 31.7% | 0 |
| **MA-cross(20/50)** | **+57.5%** | **+16.4%** | **1.12** | **12.0%** | 10 |
| MeanReversion(20) | −6.7% | −2.3% | −0.10 | 36.9% | 29 |

The trend-follower beat buy-and-hold **and** more than halved the drawdown by
sitting out the bear phase; mean-reversion lost money in a trending market — a
reminder that a strategy's edge depends entirely on the regime.

## Build & run

```bash
cmake -S . -B build
cmake --build build

./build/demo data/sample.csv   # compares strategies vs. buy-and-hold
ctest --test-dir build         # 9 unit tests
```

Point `demo` at any CSV with a `Date,Open,High,Low,Close,Volume` header
(the format Yahoo Finance exports).

## Project layout

```
include/backtest/   Bar, Strategy, Portfolio, Metrics, CsvLoader, Backtester
include/backtest/strategies/   MovingAverageCross.h, MeanReversion.h
src/                Portfolio, Metrics, CsvLoader, Backtester, main
tests/              test_backtester.cpp   (GoogleTest)
data/               sample.csv            (synthetic OHLCV series)
```

## What the tests enforce

- Max drawdown and returns computed exactly on hand-checked series
- Portfolio round-trips (buy→sell) and ignores redundant signals; commission bites
- Moving-average strategy fires a golden cross on rising prices
- End-to-end: entering an uptrend produces a profit

## Roadmap

- [ ] Execute at next bar's open (more realistic than same-bar close)
- [ ] Short selling and position sizing / risk limits
- [ ] Slippage model and per-trade logging
- [ ] Portfolio of instruments; parameter sweep / walk-forward optimisation
