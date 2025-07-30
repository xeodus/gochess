#pragma once
#include "board.hh"

struct Search {
    Move best;
    int score;
};

Search search(Board& b, int depthLimit);