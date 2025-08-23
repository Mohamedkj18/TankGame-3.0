#pragma once
#include <vector>
#include <cstddef>
#include <cassert>
#include <algorithm>
#include "common/SatelliteView.h"

// A minimal, deterministic SatelliteView for tests.
// Populate grid with '.', '#', '@', '*', '1', '2'.
// For the tank that is currently requesting BI, set its cell via setRequestingTank(x,y)
// to be returned as '%'.
struct FakeSatelliteView : public ::SatelliteView
{
    std::size_t W, H;
    std::vector<char> g;
    std::size_t req_x{0}, req_y{0};
    bool has_req{false};

    explicit FakeSatelliteView(std::size_t w, std::size_t h)
        : W(w), H(h), g(w * h, '.') {}

    void clear(char c = '.') { std::fill(g.begin(), g.end(), c); }

    void set(std::size_t x, std::size_t y, char c)
    {
        assert(x < W && y < H);
        g[y * W + x] = c;
    }

    char baseAt(std::size_t x, std::size_t y) const
    {
        assert(x < W && y < H);
        return g[y * W + x];
    }

    void setRequestingTank(std::size_t x, std::size_t y)
    {
        req_x = x;
        req_y = y;
        has_req = true;
    }

    // ::SatelliteView API
    char getObjectAt(std::size_t x, std::size_t y) const override
    {
        assert(x < W && y < H);
        if (has_req && x == req_x && y == req_y)
            return '%'; // the requesting tank
        return baseAt(x, y);
    }
};
