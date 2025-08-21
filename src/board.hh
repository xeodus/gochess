#pragma once
#include <cstdint>
#include <optional>
#include <array>

inline constexpr uint64_t FILE_A = 0x0101010101010101ULL;
inline constexpr uint64_t FILE_B = FILE_A << 1;
inline constexpr uint64_t FILE_G = FILE_A << 6;
inline constexpr uint64_t FILE_H = FILE_A << 7;

inline constexpr uint64_t RANK_1 = 0x00000000000000FFULL;
inline constexpr uint64_t RANK_2 = 0x000000000000FF00ULL;
inline constexpr uint64_t RANK_3 = 0x0000000000FF0000ULL;
inline constexpr uint64_t RANK_4 = 0x00000000FF000000ULL;
inline constexpr uint64_t RANK_5 = 0x000000FF00000000ULL;
inline constexpr uint64_t RANK_6 = 0x0000FF0000000000ULL;
inline constexpr uint64_t RANK_7 = 0x00FF000000000000ULL;
inline constexpr uint64_t RANK_8 = 0xFF00000000000000ULL;

inline void setBit(uint64_t& b, int sq) { b |= (1ULL << sq); }
inline bool getBit(uint64_t& b, int sq) { return b & (1ULL << sq); }
inline void clearBit(uint64_t& b, int sq) { b &= ~(1ULL << sq); }
inline int bitScanForward(uint64_t b) { return __builtin_ctzll(b); }
inline int popcount(uint64_t b) { return __builtin_popcountll(b); }
inline int popLsb(uint64_t& b) {
    int sq = bitScanForward(b);
    b &= b - 1;
    return sq;
}

constexpr uint64_t north(uint64_t b) { return b << 8; }
constexpr uint64_t south(uint64_t b) { return b >> 8; }
constexpr uint64_t east(uint64_t b) { return (b & ~FILE_H) << 1; }
constexpr uint64_t west(uint64_t b) { return (b & ~FILE_A) >> 1; }
constexpr uint64_t northEast(uint64_t b) { return (b & ~FILE_H) << 9; }
constexpr uint64_t northWest(uint64_t b) { return (b & ~FILE_A) << 7; }
constexpr uint64_t southEast(uint64_t b) { return (b & ~FILE_H) >> 7; }
constexpr uint64_t southWest(uint64_t b) { return (b & ~FILE_A) >> 9; }

enum Piece : int {
    WP=0, WN=1, WB=2, WR=3, WQ=4, WK=5,
    BP=6, BN=7, BB=8, BR=9, BQ=10, BK=11
};

struct Move {
    int from;
    int to;
    std::optional<int> promotions;
    Move (int f=0, int t=0, std::optional<int> p = std::nullopt)
        : from{f}, to{t}, promotions{p} {}
};

struct Position {
    std::array<uint64_t, 12> pieceBB{};
    uint64_t whites=0, blacks=0, all=0;
    bool whiteToMove = true;
    std::optional<int> enPassant = std::nullopt;
};

class Board {
public:
    Position pos;
    uint64_t knightAttack[64];
    uint64_t kingAttack[64];

    Board() {
        setStartpos();
        computeAggregate();
        initAttackTables();
    }

    void setStartpos() {
        pos.pieceBB[Piece::WP] = RANK_2;
        pos.pieceBB[Piece::BP] = RANK_7;
        setBit(pos.pieceBB[Piece::WR], 0); setBit(pos.pieceBB[Piece::WR], 7);
        setBit(pos.pieceBB[Piece::BR], 56); setBit(pos.pieceBB[Piece::BR], 63);
        setBit(pos.pieceBB[Piece::WB], 2); setBit(pos.pieceBB[Piece::WB], 5);
        setBit(pos.pieceBB[Piece::BB], 58); setBit(pos.pieceBB[Piece::BB], 61);
        setBit(pos.pieceBB[Piece::WN], 1); setBit(pos.pieceBB[Piece::WN], 6);
        setBit(pos.pieceBB[Piece::BN], 57); setBit(pos.pieceBB[Piece::BN], 62);
        setBit(pos.pieceBB[Piece::WQ], 3);
        setBit(pos.pieceBB[Piece::BQ], 59);
        setBit(pos.pieceBB[Piece::WK], 4);
        setBit(pos.pieceBB[Piece::BK], 60);
        pos.whiteToMove = true;
    }

    void computeAggregate() {
        pos.whites = pos.blacks = 0ULL;
        for (int i=0; i<6; i++) pos.whites |= pos.pieceBB[i];
        for (int k=0; k<6; k++) pos.blacks |= pos.pieceBB[k];
        pos.all = pos.whites | pos.blacks;
    }

    void initAttackTables() {

    }
};
