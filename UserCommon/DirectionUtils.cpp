#include <cmath>
#include "DirectionUtils.h"
#include "common/ActionRequest.h"   // include real enum ONCE, in a .cpp


enum class ActionRequest;


namespace UserCommon_212788293_212497127{

std::unordered_map<Direction, std::array<int, 2>> DirectionsUtils::stringToIntDirection = {
    {U, {0, -1}}, {UR, {1, -1}}, {R, {1, 0}}, {DR, {1, 1}}, {D, {0, 1}}, {DL, {-1, 1}}, {L, {-1, 0}}, {UL, {-1, -1}}};

std::unordered_map<std::string, Direction> DirectionsUtils::stringToDirection = {
    {"U", U}, {"UR", UR}, {"R", R}, {"DR", DR}, {"D", D}, {"DL", DL}, {"L", L}, {"UL", UL}};

std::unordered_map<Direction, Direction> DirectionsUtils::reverseDirection = {
    {U, D}, {UR, DL}, {R, L}, {DR, UL}, {D, U}, {DL, UR}, {L, R}, {UL, DR}};

std::unordered_map<Direction, std::string> DirectionsUtils::directionToString = {
    {U, "U"}, {UR, "UR"}, {R, "R"}, {DR, "DR"}, {D, "D"}, {DL, "DL"}, {L, "L"}, {UL, "UL"}};

std::array<Direction, 8> DirectionsUtils::directions = {U, UR, R, DR, D, DL, L, UL};

std::unordered_map<std::pair<int, int>, Direction, pair_hash> DirectionsUtils::pairToDirections = {
    {{0, -1}, U}, {{1, -1}, UR}, {{1, 0}, R}, {{1, 1}, DR}, {{0, 1}, D}, {{-1, 1}, DL}, {{-1, 0}, L}, {{-1, -1}, UL}};

std::unordered_map<std::string, double> DirectionsUtils::stringToAngle = {{"a", -0.25}, {"d", 0.25}, {"q", -0.125}, {"e", 0.125}, {"x", 0}};

Direction &operator+=(Direction &dir, double angle)
{
    std::array<Direction, 8> directions = {U, UR, R, DR, D, DL, L, UL};
    int shift = static_cast<int>(angle * 8);
    int dIdx = 0;
    for (int i = 0; i < 8; ++i)
    {
        if (dir == directions[i])
        {
            dIdx = i;
            break;
        }
    }
    dIdx = (dIdx + shift + 8) % 8;
    dir = directions[dIdx];
    return dir;
}




// 

int bijection(int x, int y)
{
    return ((int)((x + y) * (x + y + 1)) * 0.5) + y;
}

std::pair<int, int> inverseBijection(int z)
{
    int w = static_cast<int>(std::floor((std::sqrt(8 * z + 1) - 1) / 2));
    int t = (w * w + w) / 2;
    int y = z - t;
    int x = w - y;
    return {x, y};
}
}

inline std::string to_string(ActionRequest action)
{
    switch (action)
    {
    case ActionRequest::MoveForward:
        return "MoveForward";
    case ActionRequest::MoveBackward:
        return "MoveBackward";
    case ActionRequest::RotateLeft90:
        return "RotateLeft90";
    case ActionRequest::RotateRight90:
        return "RotateRight90";
    case ActionRequest::RotateLeft45:
        return "RotateLeft45";
    case ActionRequest::RotateRight45:
        return "RotateRight45";
    case ActionRequest::Shoot:
        return "Shoot";
    case ActionRequest::GetBattleInfo:
        return "GetBattleInfo";
    case ActionRequest::DoNothing:
        return "DoNothing";
    default:
        return "UnknownAction";
    }
}