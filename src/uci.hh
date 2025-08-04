#include "board.hh"
#include "search.hh"
#include <iostream>
#include <string>

class UCI {
public:
    Board& board;
    Search search;
    int depthLimit = 10;

    UCI(Board& board_, Search search_) : board{board_}, search{search_} {}

    void run() {
	std::string command;
	while (std::getline(std::cin, command)) {
	    if (command == "uci") {
		std::cout << "id name GoChess\n";
		std::cout << "id author You\n";
		std::cout << "uciok\n";
	    }
	    else if (command == "go") {
		Move m = search.best;
		std::cout << "best move" << char('a' + (m.from % 8)) << (m.from / 8 + 1)
		    << char('a' + (m.to % 8)) << (m.to / 8 + 1) << "\n";
	    }
	    else if (command == "quit") {
		break;
	    }
	}
    }
};
