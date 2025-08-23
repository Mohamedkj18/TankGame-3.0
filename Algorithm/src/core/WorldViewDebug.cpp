#include "WorldViewDebug.h"

namespace Algorithm_212788293_212497127
{

    // paint order = background → terrain → hazards → units → debug tags
    // Higher priority overwrites lower (e.g., a shell '*' masks a mine '@' like your sim).
    static void paintBase(const WorldView &wv,
                          std::vector<std::string> &board,
                          char friendCh, char enemyCh)
    {
        const std::size_t W = wv.w, H = wv.h;

        // 1) background
        board.assign(H, std::string(W, '.'));

        for (std::size_t y = 0; y < H; ++y)
        {
            for (std::size_t x = 0; x < W; ++x)
            {
                // 2) terrain (walls)
                if (wv.isWall(x, y))
                    board[y][x] = '#';

                // 3) hazards (mines)
                if (wv.isMine(x, y))
                    board[y][x] = '@';

                // 4) dynamic hazards (shells) – overrides mine (matches assignment behavior)
                if (wv.hasShell(x, y))
                    board[y][x] = '*';

                // 5) units
                if (wv.hasEnemy(x, y))
                    board[y][x] = enemyCh;
                if (wv.hasFriend(x, y))
                    board[y][x] = friendCh;

                // 6) debug tags (optional)
                if (wv.getMask(x, y) & WorldView::GOAL)
                    board[y][x] = 'G';
                else if (wv.getMask(x, y) & WorldView::VISITED)
                {
                    // don't overwrite shells/units/goals; only mark empty-ish cells
                    char c = board[y][x];
                    if (c == '.' || c == '@' || c == '#')
                        board[y][x] = '+';
                }
            }
        }
    }

    void printWorldView(const WorldView &wv,
                        std::ostream &out,
                        char friendCh,
                        char enemyCh)
    {
        if (wv.w == 0 || wv.h == 0)
        {
            out << "(empty world)\n";
            return;
        }

        std::vector<std::string> board;
        paintBase(wv, board, friendCh, enemyCh);

        // dump
        for (std::size_t y = 0; y < wv.h; ++y)
        {
            out << board[y] << '\n';
        }
        out << std::flush;
    }

    void printWorldViewWithPath(const WorldView &wv,
                                const std::vector<Cell> &path,
                                std::ostream &out,
                                char friendCh,
                                char enemyCh)
    {
        if (wv.w == 0 || wv.h == 0)
        {
            out << "(empty world)\n";
            return;
        }

        std::vector<std::string> board;
        paintBase(wv, board, friendCh, enemyCh);

        // overlay path: start 'S', goal 'G', mid 'o' (don’t overwrite shells/units/goals)
        if (!path.empty())
        {
            const auto start = path.front();
            const auto goal = path.back();

            auto safePaint = [&](std::size_t x, std::size_t y, char glyph)
            {
                char &c = board[y][x];
                if (c == '#' || c == '*' || c == friendCh || c == enemyCh || c == 'G')
                    return; // keep strong markers
                c = glyph;
            };

            safePaint(start.x, start.y, 'S');
            for (std::size_t i = 1; i + 1 < path.size(); ++i)
            {
                safePaint(path[i].x, path[i].y, 'o');
            }
            safePaint(goal.x, goal.y, 'G');
        }

        // dump
        for (std::size_t y = 0; y < wv.h; ++y)
        {
            out << board[y] << '\n';
        }
        out << std::flush;
    }

} // namespace Algorithm_212788293_212497127
