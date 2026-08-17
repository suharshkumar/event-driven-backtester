# Event-Driven Backtesting Engine (C++17)

A backtesting framework that runs pluggable strategies over historical price
bars, simulates fills with commission, and reports Sharpe, max drawdown and
annualised return along with an equity curve.

The event loop is built so that look-ahead bias can't happen. There's also
walk-forward optimisation and a deflated Sharpe ratio for catching overfitting,
which turned out to be the more interesting half of the project.

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

`Strategy` is an abstract base class, so a new strategy just subclasses it and
implements `onBar()`. Two come with it: `MovingAverageCross` for trend following
and `MeanReversion` on a z-score.

`Portfolio` does long/flat cash accounting with proportional commission.
`Backtester` is the event loop (decide, execute at the close, mark to market).
`Metrics` computes Sharpe, max drawdown, and total and annualised return.

## Why look-ahead bias can't happen here

Letting a backtest see the future is the single most common way results end up
lying to you, and it's usually accidental. The engine hands the strategy one bar
at a time, in order, and never a future bar, so a strategy physically has no way
to peek. Signals also fill at the close of the same bar they were computed from,
never at an earlier price.

## Results

`./build/demo data/sample.csv` over a 756-day synthetic series:

| Strategy | Total return | Annualised | Sharpe | Max drawdown | Trades |
|---|---|---|---|---|---|
| Buy & Hold | +28.9% | +8.8% | 0.53 | 31.7% | 0 |
| MA-cross(20/50) | +57.5% | +16.4% | 1.12 | 12.0% | 10 |
| MeanReversion(20) | -6.7% | -2.3% | -0.10 | 36.9% | 29 |

The trend follower beat buy-and-hold and roughly halved the drawdown, mostly by
sitting out the bear phase. Mean reversion lost money in a trending market. That
gap is regime, not skill, which is worth keeping in mind before reading much
into any single backtest.

## Overfitting: deflated Sharpe and walk-forward

A backtest that grid-searches parameters and reports the winner isn't telling
you much, because the best of many trials looks good on luck alone.
`overfitting_demo` runs 25 MA-cross configurations and then applies the two
things that expose it:

```
In-sample best MA(15/50)  Sharpe 1.20   <-- cherry-picked, looks great
Probabilistic Sharpe                 0.981
Deflated Sharpe (best-of-25)         0.496   <-- a coin flip once corrected
Walk-forward (train 252 / test 63)   OOS Sharpe 0.58, return +9.8%, maxDD 9%
```

[Deflated Sharpe](include/backtest/Statistics.h) (Bailey and Lopez de Prado)
corrects for the number of trials, the sample length, skew and fat tails.
[Walk-forward](include/backtest/WalkForward.h) re-optimises on a rolling
in-sample window and only ever measures on the next unseen one.

An in-sample 1.20 collapsing to 0.58 out of sample is the overfitting, made
measurable instead of argued about.

## Build & run

```bash
cmake -S . -B build
cmake --build build

./build/demo data/sample.csv              # strategies vs buy-and-hold
./build/overfitting_demo data/sample.csv  # deflated Sharpe + walk-forward
ctest --test-dir build                    # 14 unit tests
```

Point `demo` at any CSV with a `Date,Open,High,Low,Close,Volume` header, which
is what Yahoo Finance exports.

## Layout

```
include/backtest/   Bar, Strategy, Portfolio, Metrics, CsvLoader, Backtester,
                    Statistics (deflated Sharpe), WalkForward
include/backtest/strategies/   MovingAverageCross.h, MeanReversion.h
src/                Portfolio, Metrics, CsvLoader, Backtester, Statistics,
                    WalkForward, main, overfitting_demo
tests/              test_backtester.cpp, test_statistics.cpp   (GoogleTest)
data/               sample.csv            (synthetic OHLCV series)
```

## What the tests check

- Max drawdown and returns against hand-checked series
- Portfolio round-trips (buy then sell), ignores redundant signals, and that
  commission actually reduces the result
- The moving-average strategy fires a golden cross on rising prices
- End to end: entering an uptrend makes money

## Roadmap

- [ ] Execute at the next bar's open, which is more realistic than same-bar close
- [ ] Short selling, position sizing, risk limits
- [ ] Slippage model and per-trade logging
- [ ] Multiple instruments, and a proper parameter sweep
