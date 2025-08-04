#include "uci.hh"
#include "board.hh"
#include "search.hh"

int main() {
    Board board;
    Search search;
    UCI uci(board, search); 
    uci.run();
    return 0;
}
