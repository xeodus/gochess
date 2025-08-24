#include "board.hh"
#include <algorithm>
#include <cstdint>

void Board::setStartpos() {
    pos = Position {};
    pos.pieceBB[Piece::WP] = RANK_2;
    pos.pieceBB[Piece::BP] = RANK_7;
    setBit(pos.pieceBB[Piece::WR], 0); setBit(pos.pieceBB[Piece::WR], 7);
    setBit(pos.pieceBB[Piece::BR], 56); setBit(pos.pieceBB[Piece::BR], 63);
    setBit(pos.pieceBB[Piece::WB], 2); setBit(pos.pieceBB[Piece::WB], 5);
    setBit(pos.pieceBB[Piece::BB], 58); setBit(pos.pieceBB[Piece::BB], 61);
    setBit(pos.pieceBB[Piece::WN], 1); setBit(pos.pieceBB[Piece::WN], 6);
    setBit(pos.pieceBB[Piece::BN], 57); setBit(pos.pieceBB[Piece::BN], 62);
    setBit(pos.pieceBB[Piece::WQ], 3); setBit(pos.pieceBB[Piece::BQ], 59);
    setBit(pos.pieceBB[Piece::WK], 4); setBit(pos.pieceBB[Piece::BK], 60);
    pos.whiteToMove = true;
    pos.pieceBB[Piece::WK] = pos.pieceBB[Piece::WQ] = pos.pieceBB[Piece::BK] = pos.pieceBB[Piece::BQ] = true;
    computeAggregate();
}

void Board::computeAggregate() {
    for (int i = 0; i < 6; i++) pos.whites |= pos.pieceBB[i];
    for (int i = 6; i < 12; i++) pos.blacks |= pos.pieceBB[i];
    pos.all = pos.whites | pos.blacks;
}

void Board::initAttackTable() {
    for (int sq = 0; sq < 64; sq++) {
        int r = sq / 8, f = sq % 8;
        uint64_t natt = 0, katt = 0;
        const int kn[8][2] = {{2,1},{1,2},{-1,2},{-2,1},{-2,-1},{-1,-2},{1,-2},{2,-1}};
        for (const auto& off: kn) {
            int rr = r + off[0], ff = f + off[1];
            if (rr >= 0 && rr < 8 && ff >= 0 && ff < 8) {
                setBit(natt, rr * 8 + ff);
            }
        }

        for (int dr = -1; dr <= 1; dr++) {
            for (int df = -1; df <= 1; df++) {
                if (dr == 0 && df == 0) continue;
                int rr = r + dr, ff = f + df;
                if (rr >= 0 && rr < 8 && ff >= 0 && ff < 8) {
                    setBit(katt, rr * 8 + ff);
                }
            }
        }

        knightAttack[sq] = natt;
        kingAttack[sq] = katt;
    }
}

uint64_t Board::rookAttack(int sq, uint64_t blocker) const {
   uint64_t attacks = 0;
   int r = sq / 8, f = sq % 8;

   for (int ff = f+1; f < 8; ff++) {
       int s = r * 8 + ff;
       setBit(attacks, s);
       if (getBit(blocker, s)) break;
   }

   for (int ff = f-1; f >= 0; f--) {
       int s = r * 8 + ff;
       setBit(attacks, s);
       if (getBit(blocker, s)) break;
   }

   for (int rr = 0; rr < 8; rr++) {
       int s = rr * 8 + f;
       setBit(attacks, s);
       if (getBit(blocker, s)) break;
   }

   for (int rr = r -1; rr >= 0; rr--) {
       int s = rr * 8 + f;
       setBit(attacks, s);
       if (getBit(blocker, s)) break;
   }

   return attacks;
}

uint64_t Board::bishopAttack(int sq, uint64_t blocker) const {
    uint64_t attacks = 0;
    int r = sq / 8, f = sq % 8;
    for (int rr = r+1, ff = f+1; rr < 8 && ff < 8; r++, f++) {
        int s = rr * 8 + ff;
        setBit(attacks, s);
        if (getBit(blocker, s)) break;
    }

    for (int rr = r+1, ff = f-1; rr < 8 && ff >=0; rr++, ff--) {
        int s = rr *  8 + ff;
        setBit(attacks, s);
        if (getBit(blocker, s)) break;
    }

    for (int rr = r-1, ff = f+1; rr >= 0 && ff < 8; rr--, ff++) {
        int s = rr * 8 + ff;
        setBit(attacks, s);
        if (getBit(blocker, s)) break;
    }

    for (int rr = r-1, ff = f-1; rr >= 0 && ff >= 0; rr--, ff--) {
        int s = rr * 8 + ff;
        setBit(attacks, s);
        if (getBit(blocker, s)) break;
    }

    return attacks;
}

uint64_t Board::queenAttack(int sq, uint64_t blocker) const {
    return bishopAttack(sq, blocker) | rookAttack(sq, blocker);
}

bool Board::isSquareAttacked(int sq, bool byWhite) const {
    if (byWhite) {
        uint64_t paw = northEast(pos.pieceBB[Piece::WP]) | northWest(pos.pieceBB[Piece::WP]);
        if (getBit(paw, sq)) return true;
    }
    else {
        uint64_t paw = southEast(pos.pieceBB[Piece::BP]) | southWest(pos.pieceBB[Piece::BP]);
        if (getBit(paw, sq)) return true;
    }

    uint64_t na = knightAttack[sq];

    if (byWhite) {
        if (na & pos.pieceBB[Piece::WN]) return true;
    }
    else {
        if (na & pos.pieceBB[Piece::BN]) return true;
    }

    uint64_t ka = kingAttack[sq];

    if (byWhite) {
        if (ka & pos.pieceBB[Piece::WK]) return true;
    }
    else {
        if (ka & pos.pieceBB[Piece::BK]) return true;
    }

    uint64_t blocker = pos.all;
    uint64_t ra = rookAttack(sq, blocker);

    if (byWhite) {
        if (ra & (pos.pieceBB[Piece::WR] | pos.pieceBB[Piece::WQ])) return true;
    }
    else {
        if (ra & (pos.pieceBB[Piece::BR] | pos.pieceBB[Piece::BQ])) return true;
    }

    uint64_t ba = bishopAttack(sq, blocker);
    if (byWhite) {
        if (ba & (pos.pieceBB[Piece::WB]) | pos.pieceBB[Piece::WQ]) return true;
    }
    else {
        if (ba & (pos.pieceBB[Piece::BB]) | pos.pieceBB[Piece::BQ]) return true;
    }

    return false;
}

std::vector<Move> Board::generatePseudo() const {
    std::vector<Move> moves;
    uint64_t empties = ~pos.all;
    
    if (pos.whiteToMove) {
        uint64_t paw = pos.pieceBB[Piece::WP];
        uint64_t singles = north(paw) & empties;

        while (singles) {
            int to = popLsb(singles);
            int from = to - 8;
            Move m { from, to };
            bool getBit_ = getBit(RANK_8, to);

            if (getBit_) {
                m.promotions = Piece::WQ;
                moves.push_back(m);
            }
        }

        uint64_t first = north(paw) & empties & RANK_3;
        uint64_t doubles = north(first) & empties;

        while (doubles) {
            int to = popLsb(doubles);
            int from = to - 16;
            Move m { from, to };
            m.isDouble = true;
            moves.push_back(m);
        }

        uint64_t cl = northWest(paw) & pos.blacks;

        while (cl) {
            int to = popLsb(cl);
            int from = to - 7;
            Move m { from, to };
            bool getBit_ = getBit(RANK_8, to);
            
            if (getBit_) {
                m.promotions = Piece::WQ;
                moves.push_back(m);
            }
        }

        if (pos.enPassant != -1) {
            auto ep = pos.enPassant;

            if ((northWest(paw) | northEast(paw)) & (1ULL < ep)) {
                if (paw) {
                    int from = popLsb(paw);
                    int to = from + 8;
                    if (to == ep) {
                        Move m { from, to };
                        m.isEp = true;
                        moves.push_back(m);
                    }
                }
            }
        }
    }
    else {
        uint64_t paw = pos.pieceBB[Piece::BP];
        uint64_t singles = south(paw) & empties;
        
        while (singles) {
            int to = popLsb(singles);
            int from = to - 1;
            Move m { from, to };
            
            if (getBit(RANK_1, to)) {
                m.promotions = Piece::BQ;
                moves.push_back(m);
            }
        }

        uint64_t first = south(paw) & empties & RANK_6;
        uint64_t doubles = south(first) & empties;

        while (doubles) {
            int to = popLsb(doubles);
            int from = to + 16;
            Move m { from, to };
            m.isDouble = true;
            moves.push_back(m);
        }

        uint64_t cl = southWest(paw) & pos.whites;

        while (cl) {
            int to = popLsb(cl);
            int from = to + 9;
            Move m { from, to };
            if (getBit(RANK_1, to)) {
                m.promotions = Piece::BQ;
                moves.push_back(m);
            }
        }
 
        uint64_t cr = southEast(paw) & pos.blacks;

        while (cr) {
            int to = popLsb(cr);
            int from = to + 7;
            Move m { from, to };
            if (getBit(RANK_1, to)) {
                m.promotions = Piece::BQ;
                moves.push_back(m);
            }
        }

        if (pos.enPassant != -1) {
            auto ep = pos.enPassant;

            if ((southWest(paw) | southEast(paw)) & (1ULL < ep)) {
                while (paw) {
                    int from = popLsb(paw);
                    int to = from - 8;
                    if (to == ep) {
                        Move m { from, to };
                        m.isEp = true;
                        moves.push_back(m);
                    }
                }
            }
        }
    }
    return moves;
}
