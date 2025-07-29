#pragma once
#include "board.hh"

struct Search {
    Move best;
    int score;
};

Search result(Board& b, int depthLimit);