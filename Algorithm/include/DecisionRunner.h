#ifndef ALG_212788293_212497127_DECISIONRUNNER_H
#define ALG_212788293_212497127_DECISIONRUNNER_H

#include "BattleInfoLite.h"
#include "Types.h"
#include "LOS.h"
#include "WorldView.h"
#include "TeamState.h"

// Forward-declare to keep headers clean of the enum definition.
enum class ActionRequest;

namespace Algorithm_212788293_212497127
{

    class DecisionRunner
    {
    public:
        static ::ActionRequest decide(const BattleInfoLite &info,
                                      const WorldView &wv,
                                      const TeamState &tv,
                                      const TankLocal &me);
    };

} // namespace Algorithm_212788293_212497127
#endif
