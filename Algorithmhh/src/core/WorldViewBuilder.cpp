#include "WorldViewBuilder.h"
#include "common/SatelliteView.h"

namespace Algorithm_212788293_212497127
{

    WorldView buildWorldView(const ::SatelliteView &sat,
                             std::size_t W, std::size_t H,
                             int my_player_index)
    {
        WorldView wv{W, H};

        for (std::size_t y = 0; y < H; ++y)
        {
            for (std::size_t x = 0; x < W; ++x)
            {
                const char c = sat.getObjectAt(x, y);

                switch (c)
                {
                case '#': // wall
                    wv.setMask(x, y, WorldView::WALL);
                    break;

                case '@': // mine
                    wv.setMask(x, y, WorldView::MINE);
                    break;

                case '*': // shell (if above mine, satellite shows '*', not '@')
                    wv.setMask(x, y, WorldView::SHELL);
                    break;

                case '1': // tank of player 1
                    if (my_player_index == 1)
                        wv.setMask(x, y, WorldView::FRIEND);
                    else
                        wv.setMask(x, y, WorldView::ENEMY);
                    break;

                case '2': // tank of player 2
                    if (my_player_index == 2)
                        wv.setMask(x, y, WorldView::FRIEND);
                    else
                        wv.setMask(x, y, WorldView::ENEMY);
                    break;

                case '%': // the tank which requested the info (always ours)
                    wv.setMask(x, y, WorldView::FRIEND);
                    break;

                case ' ': // empty
                default:  // any other char inside bounds → treat as empty
                    break;
                }
            }
        }
        return wv;
    }

} // namespace Algorithm_212788293_212497127
