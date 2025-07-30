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
    if (moves.empty()) return b.inCheck(b.sideToMove()) ? -3000 : 0;
    for (auto& m: moves) {
        b.makeMove(m);
        auto score = -alphabeta(b, depth-1, -alpha, -beta);
        b.undoMove();
        if(score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

Search search(Board& b, int depthLimit) {
    Move best;
    int bestScore = -INT_MAX;
    auto moves = b.legalMoves();
    for (auto& move: moves) {
        b.makeMove(move);
        int score = -alphabeta(b, depthLimit-1, -INT_MAX, INT_MAX);
        b.undoMove();
        if (score > bestScore) {
            bestScore = score;
            best = move;
        }
    }
    return {best, bestScore};
}