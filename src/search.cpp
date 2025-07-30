#include "search.hh"

static int evalute(Board& b) {
    static const int value[7] = {0, 100, 320, 330, 500, 900, 20000};
    int score = 0;
    for (int sq = 0; sq < 64; sq++) {
        int pc = b.pieceAt(sq);
        if (!pc) continue;
        auto v = value[pc & 7];
        score += (pc & 8) ? -v : v;
    }
    return score;
}

static int alphabeta(Board& b, int depth, int alpha, int beta) {
    if (depth == 0) return evalute(b);
    auto moves = b.legalMoves();
    if (moves.empty()) return b.inCheck(b.sidesToMove()) ? 
}