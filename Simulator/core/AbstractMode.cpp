#include "AbstractMode.h"


static void readTheGrid(std::ifstream& in, size_t height, size_t width,
    std::set<std::pair<size_t, size_t>>& p1tanks,
    std::set<std::pair<size_t, size_t>>& p2tanks,
    std::set<std::pair<size_t, size_t>>& mines,
    std::set<std::pair<size_t, size_t>>& walls) 
{
    std::string line;
    for (size_t row = 0; row < height; ++row) {
        if (!std::getline(in, line)) {
            throw std::runtime_error("Unexpected end of file while reading grid");
        }

        for (size_t col = 0; col < line.size() && col < width; ++col) {
            char c = line[col];
            switch (c) {
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


ParsedMap AbstractMode::parseBattlefieldFile(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) {
        throw std::runtime_error("Cannot open map file: " + filename);
    }
    ParsedMap result;
    std::string line;
    // 1. Battlefield name (skip or store if needed)
    std::getline(in, line); // e.g. "1-Small battlefield"
    // 2. MaxSteps
    std::getline(in, line);
    {
        auto pos = line.find('=');
        result.max_steps = std::stoul(line.substr(pos + 1));
    }
    // 3. NumShells
    std::getline(in, line);
    {
        auto pos = line.find('=');
        result.num_shells = std::stoul(line.substr(pos + 1));
    }
    // 4. Rows
    std::getline(in, line);
    {
        auto pos = line.find('=');
        result.map_height = std::stoul(line.substr(pos + 1));
    }
    // 5. Cols
    std::getline(in, line);
    {
        auto pos = line.find('=');
        result.map_width = std::stoul(line.substr(pos + 1));
    }
    // 6. Read the grid
    readTheGrid(in, result.map_height, result.map_width,
        result.player1tanks, result.player2tanks, result.mines, result.walls);
    return result;
}


std::string AbstractMode::unique_time_str() {
    using namespace std::chrono;
    const auto now = system_clock::now();
    const auto ms  = duration_cast<milliseconds>(now.time_since_epoch()).count();
    return std::to_string(ms); // simple and collision-safe enough for this assignment
}





