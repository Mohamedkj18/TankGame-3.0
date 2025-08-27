#ifndef ALG_212788293_212497127_PLAYER_H
#define ALG_212788293_212497127_PLAYER_H

#include <cstddef>
#include <unordered_map>

#include "Types.h"          // Cell, TankLocal
#include "TeamState.h"      // reservations
#include "BattleInfoLite.h" // RoleTag
#include "common/Player.h"
#include "common/TankAlgorithm.h"
#include "common/SatelliteView.h"

namespace Algorithm_212788293_212497127
{

    class Player_212788293_212497127 : public Player
    {
    public:
        Player_212788293_212497127(int player_index,
                                   std::size_t width,
                                   std::size_t height,
                                   std::size_t max_steps,
                                   std::size_t num_shells) noexcept;

        void updateTankWithBattleInfo(TankAlgorithm &tank, SatelliteView &satellite_view) override;

        Player_212788293_212497127(const Player_212788293_212497127 &) = delete;
        Player_212788293_212497127 &operator=(const Player_212788293_212497127 &) = delete;
        ~Player_212788293_212497127() override = default;
        Player_212788293_212497127(Player_212788293_212497127 &&) noexcept = default;
        Player_212788293_212497127 &operator=(Player_212788293_212497127 &&) noexcept = default;

    private:
        int getOrAssignTankIndex(const ::TankAlgorithm *tank);

    private:
        // ---- Immutable per-match params ----
        int player_idx_{0};
        std::size_t w_{0}, h_{0};
        std::size_t max_steps_{0};
        std::size_t num_shells_{0};

        // ---- Shared per-tick state (reservations, shot lanes) ----
        TeamState team_;

        // ---- Deterministic tank indexing ----
        int next_tank_id_{0};
        std::unordered_map<const ::TankAlgorithm *, int> tank_ids_;

        // ---- Sticky role assignment per tank id (deterministic) ----
        std::unordered_map<int, RoleTag> role_by_tid_;
    };

} // namespace Algorithm_212788293_212497127

#endif
