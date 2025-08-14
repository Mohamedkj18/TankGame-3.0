#pragma once

#include <unordered_map>
#include <array>
#include <string>
#include <utility>
#include <functional>
#include <set>
#include <iostream>
#include <fstream>
#include "common/ActionRequest.h"

// ========================= ENUMS & STRUCTS =========================

enum Direction
{
    U,
    UR,
    R,
    DR,
    D,
    DL,
    L,
    UL
};

struct Wall
{
    int x, y;
    short health;
};

struct pair_hash
{
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2> &p) const
    {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ (h2 << 1);
    }
};

Direction &operator+=(Direction &dir, double angle);

// =========================== UTILS =============================
int bijection(int x, int y);
std::pair<int, int> inverseBijection(int z);
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

class DirectionsUtils
{
public:
    static std::unordered_map<Direction, std::array<int, 2>> stringToIntDirection;
    static std::unordered_map<std::string, Direction> stringToDirection;
    static std::unordered_map<Direction, Direction> reverseDirection;
    static std::unordered_map<std::string, double> stringToAngle;
    static std::unordered_map<std::pair<int, int>, Direction, pair_hash> pairToDirections;
    static std::unordered_map<Direction, std::string> directionToString;
    static std::array<Direction, 8> directions;
};
