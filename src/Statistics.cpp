#include "backtest/Statistics.h"

#include <cmath>
#include <cstddef>

namespace bt {

namespace {
constexpr double kInvSqrt2 = 0.7071067811865476;
constexpr double kEuler    = 0.5772156649015329;  // Euler-Mascheroni constant
}

double normalCdf(double x) { return 0.5 * std::erfc(-x * kInvSqrt2); }

// Peter Acklam's rational approximation of the inverse normal CDF (~1e-9 accurate).
double inverseNormalCdf(double p) {
    if (p <= 0.0) return -1e10;
    if (p >= 1.0) return  1e10;
    static const double a[] = {-3.969683028665376e+01, 2.209460984245205e+02,
                               -2.759285104469687e+02, 1.383577518672690e+02,
                               -3.066479806614716e+01, 2.506628277459239e+00};
    static const double b[] = {-5.447609879822406e+01, 1.615858368580409e+02,
                               -1.556989798598866e+02, 6.680131188771972e+01,
                               -1.328068155288572e+01};
    static const double c[] = {-7.784894002430293e-03, -3.223964580411365e-01,
                               -2.400758277161838e+00, -2.549732539343734e+00,
                                4.374664141464968e+00,  2.938163982698783e+00};
    static const double d[] = { 7.784695709041462e-03,  3.224671290700398e-01,
                                2.445134137142996e+00,  3.754408661907416e+00};
    const double pLow = 0.02425, pHigh = 1.0 - pLow;
    double q, r;
    if (p < pLow) {
        q = std::sqrt(-2.0 * std::log(p));
        return (((((c[0]*q+c[1])*q+c[2])*q+c[3])*q+c[4])*q+c[5]) /
               ((((d[0]*q+d[1])*q+d[2])*q+d[3])*q+1.0);
    } else if (p <= pHigh) {
        q = p - 0.5; r = q * q;
        return (((((a[0]*r+a[1])*r+a[2])*r+a[3])*r+a[4])*r+a[5])*q /
               (((((b[0]*r+b[1])*r+b[2])*r+b[3])*r+b[4])*r+1.0);
    } else {
        q = std::sqrt(-2.0 * std::log(1.0 - p));
        return -(((((c[0]*q+c[1])*q+c[2])*q+c[3])*q+c[4])*q+c[5]) /
                ((((d[0]*q+d[1])*q+d[2])*q+d[3])*q+1.0);
    }
}

namespace {
void moments(const std::vector<double>& x, double& mean, double& m2, double& m3, double& m4) {
    const double n = static_cast<double>(x.size());
    mean = 0.0;
    for (double v : x) mean += v;
    mean /= n;
    m2 = m3 = m4 = 0.0;
    for (double v : x) {
        const double d = v - mean;
        m2 += d * d;
        m3 += d * d * d;
        m4 += d * d * d * d;
    }
    m2 /= n; m3 /= n; m4 /= n;
}
}

double skewness(const std::vector<double>& x) {
    if (x.size() < 3) return 0.0;
    double mean, m2, m3, m4;
    moments(x, mean, m2, m3, m4);
    return (m2 > 0.0) ? m3 / std::pow(m2, 1.5) : 0.0;
}

double kurtosis(const std::vector<double>& x) {
    if (x.size() < 4) return 3.0;
    double mean, m2, m3, m4;
    moments(x, mean, m2, m3, m4);
    return (m2 > 0.0) ? m4 / (m2 * m2) : 3.0;   // non-excess: Normal == 3
}

double perPeriodSharpe(const std::vector<double>& returns) {
    if (returns.size() < 2) return 0.0;
    const double n = static_cast<double>(returns.size());
    double mean = 0.0;
    for (double r : returns) mean += r;
    mean /= n;
    double var = 0.0;
    for (double r : returns) var += (r - mean) * (r - mean);
    var /= (n - 1.0);
    const double sd = std::sqrt(var);
    return (sd > 0.0) ? mean / sd : 0.0;
}

double probabilisticSharpe(const std::vector<double>& returns, double srBenchmark) {
    if (returns.size() < 3) return 0.0;
    const double sr = perPeriodSharpe(returns);
    const double T  = static_cast<double>(returns.size());
    const double g3 = skewness(returns);
    const double g4 = kurtosis(returns);
    // Variance of the Sharpe estimator, adjusted for skew and kurtosis.
    const double denom = std::sqrt(1.0 - g3 * sr + ((g4 - 1.0) / 4.0) * sr * sr);
    if (denom <= 0.0) return 0.0;
    return normalCdf((sr - srBenchmark) * std::sqrt(T - 1.0) / denom);
}

double deflatedSharpe(const std::vector<double>& returns, const std::vector<double>& trialSharpes) {
    const std::size_t N = trialSharpes.size();
    if (N < 2) return probabilisticSharpe(returns, 0.0);

    double mean = 0.0;
    for (double s : trialSharpes) mean += s;
    mean /= N;
    double var = 0.0;
    for (double s : trialSharpes) var += (s - mean) * (s - mean);
    var /= (N - 1.0);
    const double sdTrials = std::sqrt(var);

    // Expected maximum Sharpe under the null across N independent trials.
    const double z1 = inverseNormalCdf(1.0 - 1.0 / N);
    const double z2 = inverseNormalCdf(1.0 - 1.0 / (N * std::exp(1.0)));
    const double srStar = sdTrials * ((1.0 - kEuler) * z1 + kEuler * z2);

    return probabilisticSharpe(returns, srStar);
}

} // namespace bt
