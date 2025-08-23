#pragma once
#include <cstdint>

namespace Algorithm_212788293_212497127
{
    struct WorldView;

    // Enemy LOS seeding (8 directions). If decay==0 → flat; else decays by `decay` per step.
    // max_range caps how far the ray paints (default 10 cells); walls still block.
    void seedEnemyLOSDanger(WorldView &wv,
                            std::uint32_t base = 8,
                            std::uint32_t decay = 0,
                            int max_range = 10,
                            bool include_diagonals = true);

    // Immediate shell cell penalty (kept for compatibility).
    void seedShellDanger(WorldView &wv, std::uint32_t shellBase = 20);

    // Predictive (direction-unknown) shell danger: paints ahead at 2 cells/step along all 8 dirs.
    // Adds `base - decay*(t-1)` at future positions t=1..horizon. If mark_intermediate=true,
    // also marks the “passing” cells at 2*t-1 distance with the same decayed value.
    void seedShellDangerPredictiveUnknown(WorldView &wv,
                                          std::uint32_t base = 20,
                                          std::uint32_t decay = 6,
                                          int horizon = 2,
                                          bool mark_intermediate = true);

    // Optional: small soft penalty near walls (see earlier version).
    void seedWallProximityDanger(WorldView &wv, std::uint32_t k = 1, int radius = 1);

} // namespace Algorithm_212788293_212497127
