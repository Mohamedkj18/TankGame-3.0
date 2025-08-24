#include "AbstractMode.h"



std::string AbstractMode::unique_time_str()
{
    using namespace std::chrono;
    const auto now = system_clock::now();
    const auto ms = duration_cast<milliseconds>(now.time_since_epoch()).count();
    return std::to_string(ms); 
}

static inline void chomp_cr(std::string &s)
{
    if (!s.empty() && s.back() == '\r')
        s.pop_back();
}


static void fail(const std::string &msg, const std::string &filename)
{
    throw std::runtime_error("Invalid map \"" + filename + "\": " + msg);
}


static void reaTheGrid(std::string filename , std::ifstream &file, size_t map_height, size_t map_width,
                        std::set<std::pair<size_t, size_t>> &player1tanks,
                        std::set<std::pair<size_t, size_t>> &player2tanks,
                        std::set<std::pair<size_t, size_t>> &mines,
                        std::set<std::pair<size_t, size_t>> &walls)
{
    std::string line;
    auto is_allowed = [](char c)
    {
        return c == ' ' || c == '#' || c == '@' || c == '1' || c == '2';
    };

    // Lines 6+: map
    for (size_t row = 0; row < map_height; ++row)
    {
        if (!std::getline(file, line))
            line.clear();
        chomp_cr(line);

        // Ignore extra columns beyond map_width, per spec
        if (line.size() < map_width)
            line.append(map_width - line.size(), ' ');

        for (size_t col = 0; col < map_width; ++col)
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
                fail(oss.str(), filename);
            }

            // Non-strict: treat any other char as empty space (spec-compliant)

            switch (c)
            {
            case '#':
                    walls.insert({col, row});
                break;
            case '@':
                    mines.insert({col, row});
                break;
            case '1':
                    player1tanks.insert({col, row});
                break;
            case '2':
                    player2tanks.insert({col, row});
                break;
            default: /* empty */
                break;
            }
        }
    }
}



static size_t getParams(std::ifstream &file, const std::string &prefix, const std::string &filename)
{
    std::string line;
    if (!std::getline(file, line))
        fail("Missing line starting with \"" + prefix + "\".", filename);
    chomp_cr(line);
    {
        std::regex rgx(prefix + R"(\s*=\s*(\d+))");
        std::smatch m;
        if (!std::regex_search(line, m, rgx))
            fail("Bad " + prefix + " format.", filename);
        if (m[1].str().empty())
            fail(prefix + " value missing.", filename);
        return std::stoul(std::regex_replace(line, std::regex(R"(.*=\s*)"), ""));
    }
}



ParsedMap AbstractMode::parseBattlefieldFile(const std::string &filename)
{
    ParsedMap parsed;
    std::ifstream file(filename);
    if (!file)
        throw std::runtime_error("Cannot open map file: " + filename);

    std::string line;
    if (!std::getline(file, line))
        fail("Missing line 1 (map name/description).", filename);
    chomp_cr(line);


    parsed.max_steps = getParams(file, "max_steps", filename);


    parsed.num_shells = getParams(file, "num_shells", filename);


    parsed.map_height = getParams(file, "rows", filename);
    if (parsed.map_height == 0)
        fail("Rows must be >= 1.", filename);


    parsed.map_width = getParams(file, "cols", filename);
    if (parsed.map_width == 0)
        fail("Cols must be >= 1.", filename);

    reaTheGrid(filename, file, parsed.map_height, parsed.map_width, parsed.player1tanks, parsed.player2tanks, parsed.mines, parsed.walls);

    return parsed;
}
