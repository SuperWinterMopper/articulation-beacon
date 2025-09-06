#pragma once
#include <cmath>
#include <cstring>

template<int TAPS>
class GaussianFIR {
    static_assert(TAPS % 2 == 1, "TAPS must be odd");
public:
    explicit GaussianFIR(float sigma = 1.0f) { setSigma(sigma); reset(); }

    void setSigma(float sigma) {
        if (sigma <= 0.0f) sigma = 1e-6f;
        const int radius = (TAPS - 1) / 2;
        float sum = 0.0f;
        for (int n = -radius; n <= radius; n++) {
            const float g = std::exp(-(n * n) / (2.0f * sigma * sigma));
            h_[n + radius] = g;
            sum += g;
        }
        const float inv = 1.0f / sum;
        for (int i = 0; i < TAPS; ++i) h_[i] *= inv;
    }

    void reset() {
        std::memset(z_, 0, sizeof(z_));
        w_ = 0;
    }

    float process(float x) {
        z_[w_] = x;

        float acc = 0.0f;
        int j = w_;
        for (int k = 0; k < TAPS; ++k) {
            acc += h_[k] * z_[j];
            if (--j < 0) j = TAPS - 1;
        }

        if (++w_ == TAPS) w_ = 0;
        return acc;
    }

    static constexpr int delay() { return (TAPS - 1) / 2; } // frames of latency

private:
    float h_[TAPS]{};  // Gaussian coefficients
    float z_[TAPS]{};  // ring buffer
    int   w_ = 0;      // write index
};
