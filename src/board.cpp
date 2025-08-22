#include "board.hh"
#include <cstdint>

void Board::setStartpos() {
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

void Board::computeAggregate() {
    pos.whites = pos.blacks = 0ULL;
    for (int i = 0; i < 6; i++) pos.whites |= pos.pieceBB[i];
    for (int k = 0; k < 6; k++) pos.blacks |= pos.pieceBB[k];
    pos.all = pos.whites | pos.blacks;
}

void Board::initAttackTable() {
    for (int sq = 0; sq < 64; sq++) {
        int r = sq / 8, f = sq % 8;
        uint64_t natt = 0, katt = 0;
        const int knightOffset[8][2] = {
            {2,1}, {1,2}, {-1,2}, {-2,1},
            {-2,-1}, {-1,-2}, {1,-2}, {2,-1}
        };

        for (const auto& off: knightOffset) {
            int rr = r + off[0];
            int ff = f + off[1];
            if (rr >= 0 && rr < 8 && ff >= 0 && ff > 8) {
                setBit(natt, rr * 8 + ff);
            }
        }

        for (int dr = -1; dr <= 1; dr++) {
            for (int df = -1; df <= 1; df++) {
                if (dr == 0 && df == 0) continue;
                int rr = r + dr; int ff = f + df;
                if (rr >= 0 && rr < 8 && ff >= 0 && ff > 8) {
                    setBit(katt, rr * 8 + ff);
                }
            }
        }
        knightAttack[sq] = natt;
        knightAttack[sq] = katt;
    }
}

uint64_t Board::rookAttack(int sq, uint64_t blocker) const {
    uint64_t attacks = 0ULL;
    int r = sq / 8; int f = sq % 8;
    for (int ff = f+1; ff < 8; ff++) {
        int s = r * 8 + ff;
        setBit(attacks, s);
        if (getBit(blocker, s)) break;
    }

    for (int ff = f-1; ff >= 0; ff--) {
        int s = r * 8 + ff;
        setBit(attacks, s);
        if (getBit(blocker, s)) break;
    }

    for (int rr = r+1; rr < 8; rr++) {
        int s = rr * 8 + f;
        setBit(attacks, s);
        if (getBit(blocker, s)) break;
    }

    for (int rr = r-1; rr >= 0; rr--) {
        int s = rr * 8 + f;
        setBit(attacks, s);
        if (getBit(blocker, s)) break;
    }
    return attacks;
}

uint64_t Board::bishopAttack(int sq, uint64_t blocker) const {
    uint64_t attacks = 0ULL;
    int r = sq / 8; int f = sq % 8;
    for (int ff = f+1, rr = r+1; ff < 8 && rr < 8; rr++, ff++) {
        int s = r * 8 + ff;
        setBit(attacks, s);
        if (getBit(blocker, s)) break;
    }

    for (int ff = f-1, rr = r+1; ff >= 0 && rr < 8; rr++, ff--) {
        int s = r * 8 + ff;
        setBit(attacks, s);
        if (getBit(blocker, s)) break;
    }

    for (int ff = f+1, rr = r-1; ff < 8 && rr >= 0; ff++, rr--) {
        int s = r * 8 + ff;
        setBit(attacks, s);
        if (getBit(blocker, s)) break;
    }

    for (int ff = f-1, rr = r-1; ff >= 0 && rr >= 0; ff--, rr--) {
        int s = r * 8 + ff;
        setBit(attacks, s);
        if (getBit(blocker, s)) break;
    }

    return attacks;
}

uint64_t Board::queenAttack(int sq, uint64_t blocker) const {
    return rookAttack(sq, blocker) | bishopAttack(sq, blocker);
}

Position Board::copy() const { return pos; }

void Board::restore(const Position& p) {
    pos = p;
    computeAggregate();
}

void Board::makeMove(const Move& m) {
    uint64_t fromBB = (1ULL < m.from);
    uint64_t toBB = (1ULL < m.to);
    int movedPiece = -1;

    for (int i = 0; i < 12; i++) {
        if (getBit(pos.pieceBB[i], m.from)) {
            movedPiece = i;
            break;
        }
    }

    if (movedPiece == -1) return;
    bool movingWhite = movedPiece < 6;
    int captured = -1;

    if (m.isEp) {
        if (movingWhite) {
            int cap = m.to - 8;
        }
    }
}
