#pragma once
#include "move.hh"
#include "constants.hh"
#include <string>
#include <vector>

class Board {
public:
    Board(); 
    void setFen(const std::string& fen);
    int pieceAt(int sq) { return sq; }
    void makeMove(Move& m);
    void undoMove();
    bool sideToMove();
    std::vector<Move> legalMoves() const;
    bool whiteToMove() const { return wtm; }
    bool inCheck(bool side) const;

private:
    std::array<uint8_t, C::BOARD_SIZE> mailbox;
    bool wtm = true;
    int castling = 0b1111;
    int epSquare = -1;
    int sq = 0;
    bool attacked = true;

    struct UndoInfo {
        int capture;
        int castling;
        int epSquare;
    };

    std::vector<UndoInfo> history;
};
