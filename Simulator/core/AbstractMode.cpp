#include "AbstractMode.h"

static void readTheGrid(std::ifstream &in, size_t height, size_t width,
                        std::set<std::pair<size_t, size_t>> &p1tanks,
                        std::set<std::pair<size_t, size_t>> &p2tanks,
                        std::set<std::pair<size_t, size_t>> &mines,
                        std::set<std::pair<size_t, size_t>> &walls)
{
    std::string line;
    for (size_t row = 0; row < height; ++row)
    {
        if (!std::getline(in, line))
        {
            throw std::runtime_error("Unexpected end of file while reading grid");
        }

        for (size_t col = 0; col < line.size() && col < width; ++col)
        {
            char c = line[col];
            switch (c)
            {
            case '1':
                p1tanks.emplace(row, col);
                break;
            case '2':
                p2tanks.emplace(row, col);
                break;
            case '@':
                mines.emplace(row, col);
                break;
            case '#':
                walls.emplace(row, col);
                break;
            default:
                break; // ignore spaces or other chars
            }
        }
    }
}

std::string AbstractMode::unique_time_str()
{
    using namespace std::chrono;
    const auto now = system_clock::now();
    const auto ms = duration_cast<milliseconds>(now.time_since_epoch()).count();
    return std::to_string(ms); // simple and collision-safe enough for this assignment
}

static inline void chomp_cr(std::string &s)
{
    if (!s.empty() && s.back() == '\r')
        s.pop_back();
}

ParsedMap AbstractMode::parseBattlefieldFile(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file)
        throw std::runtime_error("Cannot open map file: " + filename);

    auto fail = [&](const std::string &msg)
    {
        throw std::runtime_error("Invalid map \"" + filename + "\": " + msg);
    };

    std::string line;

    // Line 1: name/description (ignored)
    if (!std::getline(file, line))
        fail("Missing line 1 (map name/description).");
    chomp_cr(line);

    // Line 2: MaxSteps
    if (!std::getline(file, line))
        fail("Missing line 2 (MaxSteps).");
    chomp_cr(line);
    {
        std::regex rgx(R"(MaxSteps\s*=\s*(\d+))");
        std::smatch m;
        if (!std::regex_search(line, m, rgx))
            fail("Bad MaxSteps format.");
        if (m[1].str().empty())
            fail("MaxSteps value missing.");
        // you may want to check >0 here; the spec doesn’t forbid 0 explicitly
        // if (std::stoul(m[1]) == 0) fail("MaxSteps must be > 0.");
        // else:
    }

    ParsedMap parsed;
    parsed.max_steps = std::stoul(std::regex_replace(line, std::regex(R"(.*=\s*)"), ""));

    // Line 3: NumShells
    if (!std::getline(file, line))
        fail("Missing line 3 (NumShells).");
    chomp_cr(line);
    {
        std::regex rgx(R"(NumShells\s*=\s*(\d+))");
        std::smatch m;
        if (!std::regex_search(line, m, rgx))
            fail("Bad NumShells format.");
    }
    parsed.num_shells = std::stoul(std::regex_replace(line, std::regex(R"(.*=\s*)"), ""));

    // Line 4: Rows
    if (!std::getline(file, line))
        fail("Missing line 4 (Rows).");
    chomp_cr(line);
    {
        std::regex rgx(R"(Rows\s*=\s*(\d+))");
        std::smatch m;
        if (!std::regex_search(line, m, rgx))
            fail("Bad Rows format.");
    }
    parsed.map_height = std::stoul(std::regex_replace(line, std::regex(R"(.*=\s*)"), ""));
    if (parsed.map_height == 0)
        fail("Rows must be >= 1.");

    // Line 5: Cols
    if (!std::getline(file, line))
        fail("Missing line 5 (Cols).");
    chomp_cr(line);
    {
        std::regex rgx(R"(Cols\s*=\s*(\d+))");
        std::smatch m;
        if (!std::regex_search(line, m, rgx))
            fail("Bad Cols format.");
    }
    parsed.map_width = std::stoul(std::regex_replace(line, std::regex(R"(.*=\s*)"), ""));
    if (parsed.map_width == 0)
        fail("Cols must be >= 1.");

    // Allowed symbols when strict
    auto is_allowed = [](char c)
    {
        return c == ' ' || c == '#' || c == '@' || c == '1' || c == '2';
    };

    // Lines 6+: map
    for (size_t row = 0; row < parsed.map_height; ++row)
    {
        if (!std::getline(file, line))
            line.clear();
        chomp_cr(line);

        // Ignore extra columns beyond map_width, per spec
        if (line.size() < parsed.map_width)
            line.append(parsed.map_width - line.size(), ' ');

        for (size_t col = 0; col < parsed.map_width; ++col)
        {
            char c = line[col];

            if (!is_allowed(c))
            {
                std::ostringstream oss;
                oss << "Invalid character '"
                    << (std::isprint((unsigned char)c) ? std::string(1, c) : "\\x" + [&]
                                                                                 {
                        std::ostringstream h; h<<std::hex<<std::uppercase<<int((unsigned char)c); return h.str(); }())
                    << "' at (row " << row << ", col " << col
                    << "). Allowed: space, '#', '@', '1', '2'.";
                fail(oss.str());
            }

            // Non-strict: treat any other char as empty space (spec-compliant)

            switch (c)
            {
            case '#':
                parsed.walls.insert({col, row});
                break;
            case '@':
                parsed.mines.insert({col, row});
                break;
            case '1':
                parsed.player1tanks.insert({col, row});
                break;
            case '2':
                parsed.player2tanks.insert({col, row});
                break;
            default: /* empty */
                break;
            }
        }
    }

    // NOTE: According to the rules, “no tanks for a player” is a VALID map
    // and implies an immediate win/loss/tie at start. So we DON’T fail here.
    // Your simulator can check counts later and short-circuit the game.

    return parsed;
}
