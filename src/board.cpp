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
        }
    }
}
