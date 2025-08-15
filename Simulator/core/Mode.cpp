#include "include/Mode.h"


ParsedMap Mode::parseBattlefieldFile(const std::string& filename) {
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
    for (size_t row = 0; row < result.map_height; ++row) {
        if (!std::getline(in, line)) {
            throw std::runtime_error("Unexpected end of file while reading grid");
        }

        for (size_t col = 0; col < line.size() && col < result.map_width; ++col) {
            char c = line[col];
            switch (c) {
                case '1':
                    result.player1tanks.emplace(row, col);
                    break;
                case '2':
                    result.player2tanks.emplace(row, col);
                    break;
                case '@':
                    result.mines.emplace(row, col);
                    break;
                case '#':
                    result.walls.emplace(row, col);
                    break;
                default:
                    break; // ignore spaces or other chars
            }
        }
    }

    return result;
}


