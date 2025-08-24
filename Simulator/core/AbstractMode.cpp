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


static void readTheGrid(std::string filename , std::ifstream &file, size_t map_height, size_t map_width,
                        std::set<std::pair<size_t, size_t>> &walls,
                        std::set<std::pair<size_t, size_t>> &mines,
                        std::set<std::pair<size_t, size_t>> player1tanks,
                        std::set<std::pair<size_t, size_t>> &player2tanks)
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


static size_t getParams(std::ifstream& file, const std::string& expected_key, const std::string& filename)
{
    std::string line;
    if (!std::getline(file, line))
        fail("Missing line for " + expected_key + ".", filename);
    chomp_cr(line);


    const std::string pattern = "^\\s*" + expected_key + "\\s*=\\s*([0-9]+)\\s*$";
    std::regex re(pattern, std::regex::icase); 
    std::smatch m;
    if (!std::regex_match(line, m, re))
        fail("Invalid line for " + expected_key + ": " + line, filename);

    try {
        return static_cast<size_t>(std::stoul(m[1].str()));
    } catch (const std::exception&) {
        fail("Invalid number for " + expected_key + ": " + m[1].str(), filename);
    }
    return 0;
}




ParsedMap AbstractMode::parseBattlefieldFile(const std::string& filename)
{
    ParsedMap parsed;
    std::ifstream file(filename);
    if (!file)
        throw std::runtime_error("Cannot open map file: " + filename);

    std::string line;
    if (!std::getline(file, line))
        fail("Missing line 1 (map name/description).", filename);
    chomp_cr(line);
    // parsed.map_name = line; // if you keep the name

    // Use the spec’d keys (case-insensitive due to getParams above)
    parsed.max_steps  = getParams(file, "MaxSteps",  filename);
    parsed.num_shells = getParams(file, "NumShells", filename);

    parsed.map_height = getParams(file, "Rows", filename);
    if (parsed.map_height == 0) fail("Rows must be >= 1.", filename);

    parsed.map_width  = getParams(file, "Cols", filename);
    if (parsed.map_width == 0)  fail("Cols must be >= 1.", filename);


    readTheGrid( 
        filename, file,
        parsed.map_width, parsed.map_height,  
        parsed.walls, parsed.mines,      
        parsed.player1tanks, parsed.player2tanks
    );

    return parsed;
}
