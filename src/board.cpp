#include "board.hh"
#include <cstdint>
<<<<<<< HEAD
#include <functional>
=======
#include <utility>
>>>>>>> d893128 (trivial changes in legal move generation and SDL2 integration)

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

    // Helper
    auto addFrom = [&] (uint64_t bb, std::function<uint64_t(int)> f, bool whitePiece) {
        while (bb) {
            int from = popLsb(bb);
            uint64_t att = f(from);
            uint64_t friendly = whitePiece ? pos.whites : pos.blacks;
            uint64_t targets = att & ~friendly;
            uint64_t t = targets;
            while (t) {
                int to = popLsb(t);
                moves.emplace_back(from, to);
            }
        }
    };

    if (pos.whiteToMove) {
        addFrom(pos.pieceBB[Piece::WN], [&] (int s) { return knightAttack[s]; }, true);
        addFrom(pos.pieceBB[Piece::WB], [&] (int s) { return bishopAttack(s, pos.all); }, true);
        addFrom(pos.pieceBB[Piece::WR], [&] (int s) { return rookAttack(s, pos.all); }, true);
        addFrom(pos.pieceBB[Piece::WQ], [&] (int s) { return queenAttack(s, pos.all); }, true);
        addFrom(pos.pieceBB[Piece::WK], [&] (int s) { return kingAttack[s]; }, true);

        if (pos.pieceBB[Piece::WK]) {
            if (!getBit(pos.all, 5) && !getBit(pos.all, 6)) {
                if (!isSquareAttacked(4, false) && !isSquareAttacked(5, false) && !isSquareAttacked(6, false)) {
                    if (getBit(pos.pieceBB[Piece::WR], 7)) {
                        Move m { 4, 6 };
                        m.isCastled = true;
                        moves.push_back(m);
                    }
                }
            }
        }
	if (pos.pieceBB[Piece::WQ]) {
	    if (!getBit(pos.all, 1) && !getBit(pos.all, 2) && !getBit(pos.all, 3)) {
		if (!isSquareAttacked(4, false) && !isSquareAttacked(3, false) && !isSquareAttacked(2, false)) {
		    if (getBit(pos.pieceBB[Piece::WR], 0)) {
			Move m { 4, 2 };
			m.isCastled = true;
			moves.push_back(m);
		    }
		}
	    }
	}
    }
    else {
	addFrom(pos.pieceBB[Piece::BN], [&] (int s) { return knightAttack[s]; }, true);
	addFrom(pos.pieceBB[Piece::BB], [&] (int s) { return bishopAttack(s, pos.all); }, true);
	addFrom(pos.pieceBB[Piece::BR], [&] (int s) { return rookAttack(s, pos.all); }, true);
	addFrom(pos.pieceBB[Piece::BQ], [&] (int s) { return queenAttack(s, pos.all); }, true);
	addFrom(pos.pieceBB[Piece::BK], [&] (int s) { return kingAttack[s]; }, true);

	if (pos.pieceBB[Piece::BK]) {
	    if (!getBit(pos.all, 61) && !getBit(pos.all, 62)) {
		if (!isSquareAttacked(60, true)  && !isSquareAttacked(61, true) && !isSquareAttacked(62, true)) {
		    if (getBit(pos.pieceBB[Piece::BR], 63)) {
			Move m { 60, 62 };
			m.isCastled = true;
			moves.push_back(m);
		    }
		}
	    }
	}
	if (pos.pieceBB[Piece::BQ]) {
	    if (!getBit(pos.all, 57) && !getBit(pos.all, 58) && !getBit(pos.all, 59)) {
		if (!isSquareAttacked(60, true) && !isSquareAttacked(59, true) && !isSquareAttacked(58, true)) {
		    if (getBit(pos.pieceBB[Piece::BR], 56)) {
			Move m { 60, 58 };
			m.isCastled = true;
			moves.push_back(m);
		    }
		}
	    }
	}
    }
    return moves;
}


std::vector<Move> Board::generateLegal() {
    auto pseudo = generatePseudo();
    std::vector<Move> legal;

    for (const auto& m: pseudo) {
        Position snap = pos;
        makeMove(m);
        int kingSq = -1;
        if (!pos.whiteToMove) {
            if (pos.pieceBB[Piece::WK]) kingSq = bitScanForward(pos.pieceBB[Piece::WK]);
        }
        else {
            if (pos.pieceBB[Piece::BK]) kingSq = bitScanForward(pos.pieceBB[Piece::BK]);
        }

        bool inCheck = isSquareAttacked(kingSq, !pos.whiteToMove);
        computeAggregate();
        if (!inCheck) legal.push_back(m);
    }
    return legal;
}

int Board::findPieceAt(int s) const {
    for (int i = 0; i < 12; i++) {
        if (getBit(pos.pieceBB[i], s)) return i;
    }
    return -1;
}

void Board::makeMove(const Move& m) {
    int moved = -1;
    for (int i = 0; i < 12; i++) {
        if (getBit(pos.pieceBB[i], m.from)) { moved = i; break; }

    }

    if (moved == -1) return;
    bool movingWhite = moved < 6;
    int capture = -1;

    if (m.isEp) {
        if (movingWhite) {
            int cap = m.to - 8;
            for (int i = 6; i < 12; i++) {
                if (getBit(pos.pieceBB[i], cap)) {
                    capture = i;
                    clearBit(pos.pieceBB[i], cap);
                    break;
                }
            }
        }
        else {
            int cap = m.to + 8;
            for (int i = 0; i < 6; i++) {
                if (getBit(pos.pieceBB[i], cap)) {
                    capture = i;
                    clearBit(pos.pieceBB[i], cap);
                    break;
                }
            }
        }
    }
    else {
        for (int i = 0; i < 12; i++) {
            if (getBit(pos.pieceBB[i],m.to)) {
                capture = i;
                clearBit(pos.pieceBB[i], m.to);
                break;
            }
        }
    }

    clearBit(pos.pieceBB[moved], m.from);

    if (m.promotions != 1) {
        setBit(pos.pieceBB[m.promotions], m.to);
    }
    else {
        setBit(pos.pieceBB[moved], m.to);
    }

    if (m.isCastled) {
        if (movingWhite) {
            if (m.to == 6) {
                clearBit(pos.pieceBB[Piece::WR], 7);
                setBit(pos.pieceBB[Piece::WR], 5);
            }
            else if (m.to == 2) {
                clearBit(pos.pieceBB[Piece::WR], 0);
                setBit(pos.pieceBB[Piece::WR], 3);
            }
        }
        else {
            if (m.to == 62) {
                clearBit(pos.pieceBB[Piece::BR], 63);
                setBit(pos.pieceBB[Piece::BR], 61);
            }
            else if (m.to == 58) {
                clearBit(pos.pieceBB[Piece::BR], 56);
                setBit(pos.pieceBB[Piece::BR], 59);
            }
        }
    }

    if (movingWhite) {
        pos.pieceBB[Piece::WK] = pos.pieceBB[Piece::WQ] = false;
    }
    else {
        pos.pieceBB[Piece::BK] = pos.pieceBB[Piece::BQ] = false;
    }

    if (m.from == 0 || m.to == 0) pos.pieceBB[Piece::WQ] = false;
    if (m.from == 7 || m.to == 7) pos.pieceBB[Piece::WK] = false;
    if (m.from == 56 || m.to == 56) pos.pieceBB[Piece::BQ] = false;
    if (m.from == 63 || m.to == 63) pos.pieceBB[Piece::BK] = false;

    if (m.isDouble) {
        if (movingWhite) pos.enPassant = m.from + 8;
        else pos.enPassant = m.from - 8;
    }
    else {
        pos.enPassant = -1;
    }

    if (moved%6 == 0 && capture != -1) pos.halfmove = 0;
    else pos.halfmove++;
    if (!pos.whiteToMove) pos.fullmove++;
    computeAggregate();
}

void Board::restore(const Position& p) {
    pos = p;
    computeAggregate();
}