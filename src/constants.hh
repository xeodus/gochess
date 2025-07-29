#pragma once
#include <cstdint>
#include <array>

namespace C {
    constexpr uint8_t EMPTY = 0;

    // piece codes (color bit = 8)
    constexpr uint8_t WP = 1,  WN = 2,  WB = 3,  WR = 4,  WQ = 5,  WK = 6;
    constexpr uint8_t BP = 9,  BN = 10, BB = 11, BR = 12, BQ = 13, BK = 14;

    // promotion codes
    constexpr uint8_t KNIGHT_PROMO = 2;
    constexpr uint8_t BISHOP_PROMO = 3;
    constexpr uint8_t ROOK_PROMO   = 4;
    constexpr uint8_t QUEEN_PROMO  = 5;

    // board helpers
    constexpr int BOARD_SIZE = 128;
    constexpr int REAL_SIZE  = 64;
    constexpr int DIR_N   = 16;
    constexpr int DIR_S   = -16;
    constexpr int DIR_E   = 1;
    constexpr int DIR_W   = -1;
    constexpr int DIR_NE  = 17;
    constexpr int DIR_NW  = 15;
    constexpr int DIR_SE  = -15;
    constexpr int DIR_SW  = -17;

    constexpr std::array<int,8> KNIGHT_DELTAS{33,31,18,14,-33,-31,-18,-14};
    constexpr std::array<int,4> BISHOP_DELTAS{15,17,-15,-17};
    constexpr std::array<int,4> ROOK_DELTAS{1,-1,16,-16};
    constexpr std::array<int,8> KING_DELTAS{1,-1,15,16,17,-15,-16,-17};
}