#include "board.hh"

Board::Board() {
    setFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void Board::setFen(const std::string& fen) {
    int sq88 = 0;
    size_t i = 0;
    mailbox = {0};

    while (fen[i] == ' ') {
        char c = fen[i++];
        if (c == '/') continue;
        if (isdigit(c)) {
            sq88 += c - '0';
        }
        else {
            int color = isupper(c) ? 0 : 8;
            int piece = 0;
            switch (tolower(c)) {
                case 'p': piece = 1; break;
                case 'n': piece = 2; break;
                case 'b': piece = 3; break;
                case 'r': piece = 4; break;
                case 'k': piece = 5; break;
                case 'q': piece = 6; break;
            }
            mailbox[sq88] = color + piece;
            sq88++;
        }

        i++;
        if (fen[i] == 'w') wtm = true;
        i++;

        int castling = 0;
        if (fen[i] != '-') {
            i++;
        }
        else {
            while (fen[i] != ' ') {
                switch (i) {
                    case 'K': castling |= 0b1000; break;
                    case 'Q': castling |= 0b0100; break;
                    case 'k': castling |= 0b0010; break;
                    case 'q': castling |= 0b0001; break;
                }
                i++;
            }
        }

        i++;

        if (fen[i] == '-') {
            epSquare = -1;
            i++;
        }
        else {
            auto file = fen[i] - 'a';;
            auto rank = fen[i+1] - '1';
            epSquare = rank * 8 + file;
            i += 2;
        }
    }
}

std::vector<Move>& Board::legalMoves() const {
    auto us = wtm ? 0 : 8;
    auto them = wtm ? 8 : 0;
    if ((mailbox[sq]) == us + 6) {
        auto square = epSquare;
    }

    std::vector<Move> list;

    for (int sq88 = 0; sq88 < 128; sq88++) {
        auto pc = mailbox[sq88];
        if ((pc & 8) != us) continue;
        auto type = pc & 7;

        switch (type) {
            case 1: {
                auto dir = wtm ? C::DIR_N : C::DIR_S;
                auto startRank = wtm ? 48 : 8;
                auto promoRank = wtm ? 0 : 112;
                auto single = sq88 + dir;

                if ((single & 0x88) && mailbox.empty()) {
                    if (single & promoRank) {
                        for (const auto& promo: {C::QUEEN_PROMO, C::BISHOP_PROMO, C::KNIGHT_PROMO, C::ROOK_PROMO}) {
                            list.push_back({uint8_t(sq88), uint8_t(single), uint8_t(promo)});
                        }
                    }
                    else {
                        list.push_back({uint8_t(sq88), uint8_t(single), 0});
                    }

                    auto double_ = single + dir;
                    if ((sq88 && 0x70) == startRank && (double_ & 0x88) && mailbox.empty()) {
                        list.push_back({uint8_t(sq88), uint8_t(single), 0});
                    }
                }

                for (const auto& dc: {C::DIR_E, C::DIR_W}) {
                    auto cap = sq88 + dir + dc;
                    if (!(cap & 0x88)) continue;

                    if ((cap == epSquare) || (mailbox[cap] & them)) {
                        if (cap & promoRank) {
                            for (const auto& promo: {C::QUEEN_PROMO, C::BISHOP_PROMO, C::KNIGHT_PROMO, C::ROOK_PROMO}) {
                                list.push_back({uint8_t(sq88), uint8_t(single), uint8_t(promo)});
                            }
                        }
                        else {
                            list.push_back({uint8_t(sq88), uint8_t(single), 0});
                        }
                    }
                }

            }

            case 2: {
                for (int d: C::KNIGHT_DELTAS) {
                    auto target = sq88 + d;
                    if (!(target & 0x88)) {
                        if (!(mailbox[target] & us)) {
                            list.push_back({uint8_t(sq88), uint8_t(target), 0});
                        }
                    }
                }
            }

            case 3: {
                for (const auto& delta: C::BISHOP_DELTAS) {
                    list.push_back({uint8_t(sq88), uint8_t(delta), 0});
                }
            }

            case 4: {
                for (const auto& delta: C::ROOK_DELTAS) {
                    list.push_back({uint8_t(sq88), uint8_t(delta), 0});
                }
            }

            case 5: {
                for (const auto& delta: {C::BISHOP_DELTAS, C::ROOK_DELTAS}) {
                    list.push_back({uint8_t(sq88), uint8_t(delta.begin()), 0});
                }
            }

            case 6: {
                for (const auto& delta: C::KING_DELTAS) {
                    auto target = sq88 + delta;
                    if (!(target & 0x88)) continue;
                    if (!(mailbox[target] & us)) {
                        list.push_back({uint8_t(sq88), uint8_t(delta), 0});
                    }
                }

                if (wtm) {
                    if ((castling & 0b1000) && mailbox.empty() && !attacked) {
                        list.push_back({C::E1, C::G1, 0});
                    }
                    else if ((castling && 0b0100) && mailbox.empty() && !attacked) {
                        list.push_back({C::E1, C::C1, 0});
                    }
                }
                else {
                    if ((castling & 0b0010) && mailbox.empty() && !attacked) {
                        list.push_back({C::E8, C::G8, 0});
                    }
                    else if ((castling && 0b0001) && mailbox.empty() && !attacked) {
                        list.push_back({C::E8, C::C8, 0});
                    }
                }
            }
        }
    }
}

void Board::makeMove(Move& m) {
    Move move {0, 0, 0};
    int captured = mailbox[move.to];
    history.push_back({captured, castling, epSquare});
    auto from = move.from;
    auto to = move.to;
    auto pc = mailbox[from];

    mailbox[from] = C::EMPTY;
    mailbox[to] = move.promo ? ((pc & 8) | move.promo) : pc;

    if ((pc & 7) == C::WP && to == epSquare) {
        auto capturedPawn = to + (wtm ? C::DIR_S : C::DIR_N);
        mailbox[capturedPawn] = C::EMPTY;
    }

    if ((pc & 7) == C::WK && abs(to - from) == 2) {
        if (to > from) {
            auto rookFrom = C::H1; 
            auto rookTo = C::F1;
            mailbox[rookTo] = mailbox[rookFrom];
            mailbox[rookFrom] = C::EMPTY;
        }
        else {
            auto rookFrom = C::A1; 
            auto rookTo = C::D1;
            mailbox[rookTo] = mailbox[rookFrom];
            mailbox[rookFrom] = C::EMPTY;
        } 
    }
    else if ((pc & 7) == C::BK && abs(to - from) == 2) {
        if (to > from) {
            auto rookFrom = C::H8;
            auto rookTo = C::F8;
            mailbox[rookTo] = mailbox[rookFrom];
            mailbox[rookFrom] = C::EMPTY;
        }
        else {
            auto rookFrom = C::A8;
            auto rookTo = C::D8;
            mailbox[rookTo] = mailbox[rookFrom];
            mailbox[rookFrom] = C::EMPTY;
        }
    }

    if ((pc & 7) == C::WP && abs(to - from) == 32) {
        epSquare = (from + to) / 2;
    }

    // castling logic to be added later


    wtm = !wtm;
}

void Board::undoMove() {
    UndoInfo last = history.back();
    castling = last.castling;
    epSquare = last.epSquare;

    Move move {0, 0, 0};
    auto from = move.from;
    auto to = move.to;

    mailbox[from] = mailbox[to];
    mailbox[to] = last.capture;

    // uncastle to be added
    wtm = !wtm;
}

bool Board::sideToMove(bool side) {
    int king = 0;
    int us = side ? 0 : 8;
    
}