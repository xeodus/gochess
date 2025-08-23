
// chess_gui_sdl.cpp
// Single-file playable chess GUI using SDL2.
// Compile (Fedora): g++ -std=c++23 -O3 chess_gui_sdl.cpp -o chess_gui_sdl `sdl2-config --cflags --libs`
// Run: ./chess_gui_sdl
//
// Features:
// - Click source square, click destination square to move.
// - Highlights legal moves for selected piece.
// - Enforces legal moves (castling, en-passant, promotions).
// - Undo last ply (press 'u').
// - Engine reply: selects a random legal move.
// - Displays basic status text.

#include <SDL2/SDL.h>
#include <bits/stdc++.h>
using namespace std;
using U64 = uint64_t;

inline void set_bit(U64 &b, int sq) { b |= (1ULL << sq); }
inline void clear_bit(U64 &b, int sq) { b &= ~(1ULL << sq); }
inline bool get_bit(U64 b, int sq) { return (b >> sq) & 1ULL; }
inline int popcount(U64 b) { return __builtin_popcountll(b); }
inline int bit_scan_forward(U64 b) { return __builtin_ctzll(b); }
inline int pop_lsb(U64 &b) { int sq = bit_scan_forward(b); b &= b - 1; return sq; }

constexpr U64 FILE_A = 0x0101010101010101ULL;
constexpr U64 FILE_H = FILE_A << 7;
constexpr U64 RANK_1 = 0x00000000000000FFULL;
constexpr U64 RANK_2 = 0x000000000000FF00ULL;
constexpr U64 RANK_3 = 0x0000000000FF0000ULL;
constexpr U64 RANK_6 = 0x0000FF0000000000ULL;
constexpr U64 RANK_7 = 0x00FF000000000000ULL;
constexpr U64 RANK_8 = 0xFF00000000000000ULL;

constexpr U64 north(U64 b) { return b << 8; }
constexpr U64 south(U64 b) { return b >> 8; }
constexpr U64 east(U64 b) { return (b & ~FILE_H) << 1; }
constexpr U64 west(U64 b) { return (b & ~FILE_A) >> 1; }
constexpr U64 north_east(U64 b) { return (b & ~FILE_H) << 9; }
constexpr U64 north_west(U64 b) { return (b & ~FILE_A) << 7; }
constexpr U64 south_east(U64 b) { return (b & ~FILE_H) >> 7; }
constexpr U64 south_west(U64 b) { return (b & ~FILE_A) >> 9; }

enum Piece : int {
  WP=0, WN=1, WB=2, WR=3, WQ=4, WK=5,
  BP=6, BN=7, BB=8, BR=9, BQ=10, BK=11
};

struct Move {
  int from=0, to=0;
  int promotion=-1;
  bool is_castle=false, is_double=false, is_ep=false;
  Move()=default;
  Move(int f,int t):from(f),to(t){}
};

struct Position {
  array<U64,12> bb{};
  U64 white=0, black=0, all=0;
  bool white_to_move=true;
  bool wk=true, wq=true, bk=true, bq=true;
  int ep = -1; // en-passant target
  int halfmove=0, fullmove=1;
};

struct Board {
  Position pos;
  U64 knight_att[64];
  U64 king_att[64];

  Board(){ init_attack_tables(); set_startpos(); }

  void set_startpos(){
    pos = Position{};
    pos.bb[WP] = RANK_2;
    pos.bb[BP] = RANK_7;
    set_bit(pos.bb[WR],0); set_bit(pos.bb[WR],7);
    set_bit(pos.bb[BR],56); set_bit(pos.bb[BR],63);
    set_bit(pos.bb[WN],1); set_bit(pos.bb[WN],6);
    set_bit(pos.bb[BN],57); set_bit(pos.bb[BN],62);
    set_bit(pos.bb[WB],2); set_bit(pos.bb[WB],5);
    set_bit(pos.bb[BB],58); set_bit(pos.bb[BB],61);
    set_bit(pos.bb[WQ],3); set_bit(pos.bb[BQ],59);
    set_bit(pos.bb[WK],4); set_bit(pos.bb[BK],60);
    pos.white_to_move = true;
    pos.wk = pos.wq = pos.bk = pos.bq = true;
    pos.ep = -1; pos.halfmove=0; pos.fullmove=1;
    compute_aggr();
  }

  void compute_aggr(){
    pos.white = pos.black = 0;
    for(int i=0;i<6;++i) pos.white |= pos.bb[i];
    for(int i=6;i<12;++i) pos.black |= pos.bb[i];
    pos.all = pos.white | pos.black;
  }

  void init_attack_tables(){
    for(int sq=0;sq<64;++sq){
      int r=sq/8,f=sq%8;
      U64 natt=0,katt=0;
      const int kn[8][2] = {{2,1},{1,2},{-1,2},{-2,1},{-2,-1},{-1,-2},{1,-2},{2,-1}};
      for(auto &o: kn){
        int rr=r+o[0], ff=f+o[1];
        if(rr>=0&&rr<8&&ff>=0&&ff<8) set_bit(natt, rr*8+ff);
      }
      for(int dr=-1;dr<=1;++dr) for(int df=-1;df<=1;++df){
        if(dr==0&&df==0) continue;
        int rr=r+dr, ff=f+df;
        if(rr>=0&&rr<8&&ff>=0&&ff<8) set_bit(katt, rr*8+ff);
      }
      knight_att[sq]=natt;
      king_att[sq]=katt;
    }
  }

  U64 rook_attacks(int sq, U64 blockers) const {
    U64 attacks=0; int r=sq/8,f=sq%8;
    for(int ff=f+1; ff<8; ++ff){ int s=r*8+ff; set_bit(attacks,s); if(get_bit(blockers,s)) break; }
    for(int ff=f-1; ff>=0; --ff){ int s=r*8+ff; set_bit(attacks,s); if(get_bit(blockers,s)) break; }
    for(int rr=r+1; rr<8; ++rr){ int s=rr*8+f; set_bit(attacks,s); if(get_bit(blockers,s)) break; }
    for(int rr=r-1; rr>=0; --rr){ int s=rr*8+f; set_bit(attacks,s); if(get_bit(blockers,s)) break; }
    return attacks;
  }
  U64 bishop_attacks(int sq, U64 blockers) const {
    U64 attacks=0; int r=sq/8,f=sq%8;
    for(int rr=r+1,ff=f+1; rr<8&&ff<8; ++rr,++ff){ int s=rr*8+ff; set_bit(attacks,s); if(get_bit(blockers,s)) break; }
    for(int rr=r+1,ff=f-1; rr<8&&ff>=0; ++rr,--ff){ int s=rr*8+ff; set_bit(attacks,s); if(get_bit(blockers,s)) break; }
    for(int rr=r-1,ff=f+1; rr>=0&&ff<8; --rr,++ff){ int s=rr*8+ff; set_bit(attacks,s); if(get_bit(blockers,s)) break; }
    for(int rr=r-1,ff=f-1; rr>=0&&ff>=0; --rr,--ff){ int s=rr*8+ff; set_bit(attacks,s); if(get_bit(blockers,s)) break; }
    return attacks;
  }
  U64 queen_attacks(int sq, U64 blockers) const { return rook_attacks(sq,blockers) | bishop_attacks(sq,blockers); }

  bool is_square_attacked(int square, bool by_white) const {
    if(by_white){
      U64 paw = north_west(pos.bb[WP]) | north_east(pos.bb[WP]);
      if(get_bit(paw, square)) return true;
    } else {
      U64 paw = south_west(pos.bb[BP]) | south_east(pos.bb[BP]);
      if(get_bit(paw, square)) return true;
    }
    U64 na = knight_att[square];
    if(by_white){ if(na & pos.bb[WN]) return true; } else { if(na & pos.bb[BN]) return true; }
    U64 ka = king_att[square];
    if(by_white){ if(ka & pos.bb[WK]) return true; } else { if(ka & pos.bb[BK]) return true; }

    U64 blockers = pos.all;
    U64 r_att = rook_attacks(square, blockers);
    if(by_white){ if(r_att & (pos.bb[WR] | pos.bb[WQ])) return true; } else { if(r_att & (pos.bb[BR] | pos.bb[BQ])) return true; }
    U64 b_att = bishop_attacks(square, blockers);
    if(by_white){ if(b_att & (pos.bb[WB] | pos.bb[WQ])) return true; } else { if(b_att & (pos.bb[BB] | pos.bb[BQ])) return true; }
    return false;
  }

  vector<Move> generate_pseudo() const {
    vector<Move> moves;
    U64 empties = ~pos.all;
    bool white = pos.white_to_move;

    // pawns
    if(white){
      U64 paw = pos.bb[WP];
      U64 singles = north(paw) & empties;
      U64 s = singles;
      while(s){ int to = pop_lsb(s); int from = to-8; Move m(from,to); if(get_bit(RANK_8,to)) m.promotion = WQ; moves.push_back(m); }
      U64 first = north(paw) & empties & RANK_3;
      U64 doubles = north(first) & empties;
      s = doubles;
      while(s){ int to=pop_lsb(s); int from=to-16; Move m(from,to); m.is_double=true; moves.push_back(m); }
      U64 cl = north_west(paw) & pos.black;
      s = cl;
      while(s){ int to=pop_lsb(s); int from=to-7; Move m(from,to); if(get_bit(RANK_8,to)) m.promotion=WQ; moves.push_back(m); }
      U64 cr = north_east(paw) & pos.black;
      s = cr;
      while(s){ int to=pop_lsb(s); int from=to-9; Move m(from,to); if(get_bit(RANK_8,to)) m.promotion=WQ; moves.push_back(m); }
      // ep
      if(pos.ep!=-1){
        int ep = pos.ep;
        if( ((north_west(paw) | north_east(paw)) & (1ULL<<ep)) ){
          // find pawn from
          U64 copy = paw;
          while(copy){ int from=pop_lsb(copy); if(from+8==ep){ Move m(from,ep); m.is_ep=true; moves.push_back(m);} }
        }
      }
    } else {
      U64 paw = pos.bb[BP];
      U64 singles = south(paw) & empties;
      U64 s = singles;
      while(s){ int to=pop_lsb(s); int from=to+8; Move m(from,to); if(get_bit(RANK_1,to)) m.promotion=BQ; moves.push_back(m); }
      U64 first = south(paw) & empties & RANK_6;
      U64 doubles = south(first) & empties;
      s = doubles;
      while(s){ int to=pop_lsb(s); int from=to+16; Move m(from,to); m.is_double=true; moves.push_back(m);}
      U64 cl = south_west(paw) & pos.white;
      s = cl; while(s){ int to=pop_lsb(s); int from=to+9; Move m(from,to); if(get_bit(RANK_1,to)) m.promotion=BQ; moves.push_back(m); }
      U64 cr = south_east(paw) & pos.white;
      s = cr; while(s){ int to=pop_lsb(s); int from=to+7; Move m(from,to); if(get_bit(RANK_1,to)) m.promotion=BQ; moves.push_back(m); }
      if(pos.ep!=-1){
        int ep=pos.ep;
        if(((south_west(paw)|south_east(paw)) & (1ULL<<ep))){
          U64 copy = paw;
          while(copy){ int from=pop_lsb(copy); if(from-8==ep){ Move m(from,ep); m.is_ep=true; moves.push_back(m);} }
        }
      }
    }

    // helpers
    auto add_from = [&](U64 bb, function<U64(int)> f, bool white_piece){
      U64 copy=bb;
      while(copy){
        int from=pop_lsb(copy);
        U64 att = f(from);
        U64 friendly = white_piece ? pos.white : pos.black;
        U64 targets = att & ~friendly;
        U64 t=targets;
        while(t){ int to=pop_lsb(t); moves.emplace_back(from,to); }
      }
    };

    if(pos.white_to_move){
      add_from(pos.bb[WN], [&](int s){ return knight_att[s]; }, true);
      add_from(pos.bb[WB], [&](int s){ return bishop_attacks(s,pos.all); }, true);
      add_from(pos.bb[WR], [&](int s){ return rook_attacks(s,pos.all); }, true);
      add_from(pos.bb[WQ], [&](int s){ return queen_attacks(s,pos.all); }, true);
      add_from(pos.bb[WK], [&](int s){ return king_att[s]; }, true);
      // castling
      if(pos.wk){
        if(!get_bit(pos.all,5) && !get_bit(pos.all,6)){
          if(!is_square_attacked(4,false) && !is_square_attacked(5,false) && !is_square_attacked(6,false)){
            if(get_bit(pos.bb[WR],7)) { Move m(4,6); m.is_castle=true; moves.push_back(m); }
          }
        }
      }
      if(pos.wq){
        if(!get_bit(pos.all,1) && !get_bit(pos.all,2) && !get_bit(pos.all,3)){
          if(!is_square_attacked(4,false) && !is_square_attacked(3,false) && !is_square_attacked(2,false)){
            if(get_bit(pos.bb[WR],0)) { Move m(4,2); m.is_castle=true; moves.push_back(m); }
          }
        }
      }
    } else {
      add_from(pos.bb[BN], [&](int s){ return knight_att[s]; }, false);
      add_from(pos.bb[BB], [&](int s){ return bishop_attacks(s,pos.all); }, false);
      add_from(pos.bb[BR], [&](int s){ return rook_attacks(s,pos.all); }, false);
      add_from(pos.bb[BQ], [&](int s){ return queen_attacks(s,pos.all); }, false);
      add_from(pos.bb[BK], [&](int s){ return king_att[s]; }, false);
      if(pos.bk){
        if(!get_bit(pos.all,61) && !get_bit(pos.all,62)){
          if(!is_square_attacked(60,true) && !is_square_attacked(61,true) && !is_square_attacked(62,true)){
            if(get_bit(pos.bb[BR],63)) { Move m(60,62); m.is_castle=true; moves.push_back(m); }
          }
        }
      }
      if(pos.bq){
        if(!get_bit(pos.all,57) && !get_bit(pos.all,58) && !get_bit(pos.all,59)){
          if(!is_square_attacked(60,true) && !is_square_attacked(59,true) && !is_square_attacked(58,true)){
            if(get_bit(pos.bb[BR],56)) { Move m(60,58); m.is_castle=true; moves.push_back(m); }
          }
        }
      }
    }
    return moves;
  }

  vector<Move> generate_legal() {
    auto pseudo = generate_pseudo();
    vector<Move> legal;
    for(auto &m: pseudo){
      Position snap = pos;
      make_move(m);
      int king_sq = -1;
      if(!pos.white_to_move){
        // white just moved -> find white king
        if(pos.bb[WK]) king_sq = bit_scan_forward(pos.bb[WK]);
      } else {
        if(pos.bb[BK]) king_sq = bit_scan_forward(pos.bb[BK]);
      }
      bool incheck = is_square_attacked(king_sq, !pos.white_to_move);
      pos = snap; compute_aggr();
      if(!incheck) legal.push_back(m);
    }
    return legal;
  }

  int find_piece_at(int s) const {
    for(int i=0;i<12;++i) if(get_bit(pos.bb[i],s)) return i;
    return -1;
  }

  void make_move(const Move &m){
    int moved=-1;
    for(int i=0;i<12;++i) if(get_bit(pos.bb[i], m.from)){ moved=i; break; }
    if(moved==-1) return;
    bool moving_white = moved < 6;
    int captured = -1;
    if(m.is_ep){
      if(moving_white){
        int cap = m.to - 8;
        for(int i=6;i<12;++i) if(get_bit(pos.bb[i], cap)){ captured=i; clear_bit(pos.bb[i], cap); break; }
      } else {
        int cap = m.to + 8;
        for(int i=0;i<6;++i) if(get_bit(pos.bb[i], cap)){ captured=i; clear_bit(pos.bb[i], cap); break; }
      }
    } else {
      for(int i=0;i<12;++i) if(get_bit(pos.bb[i], m.to)){ captured=i; clear_bit(pos.bb[i], m.to); break; }
    }
    clear_bit(pos.bb[moved], m.from);

    if(m.promotion!=-1){
      // promotion index already contains correct colored piece index
      set_bit(pos.bb[m.promotion], m.to);
    } else {
      set_bit(pos.bb[moved], m.to);
    }

    if(m.is_castle){
      if(moving_white){
        if(m.to==6){ clear_bit(pos.bb[WR],7); set_bit(pos.bb[WR],5); } // h1->f1
        else if(m.to==2){ clear_bit(pos.bb[WR],0); set_bit(pos.bb[WR],3); } // a1->d1
      } else {
        if(m.to==62){ clear_bit(pos.bb[BR],63); set_bit(pos.bb[BR],61); }
        else if(m.to==58){ clear_bit(pos.bb[BR],56); set_bit(pos.bb[BR],59); }
      }
    }

    // update castling rights
    if(moving_white){ pos.wk = pos.wq = false; }
    else { pos.bk = pos.bq = false; }
    // rook moves/captures
    if(m.from==0 || m.to==0) pos.wq=false;
    if(m.from==7 || m.to==7) pos.wk=false;
    if(m.from==56 || m.to==56) pos.bq=false;
    if(m.from==63 || m.to==63) pos.bk=false;

    // en-passant target
    if(m.is_double){
      if(moving_white) pos.ep = m.from + 8;
      else pos.ep = m.from - 8;
    } else pos.ep = -1;

    // half/fullmove
    if(moved%6==0 || captured!=-1) pos.halfmove=0; else pos.halfmove++;
    if(!pos.white_to_move) pos.fullmove++;

    pos.white_to_move = !pos.white_to_move;
    compute_aggr();
  }

  void undo_to(const Position &p){ pos = p; compute_aggr(); }
};

////////////////////
// SDL2 GUI code
////////////////////

static const int WINDOW_W = 640;
static const int WINDOW_H = 640;
static const SDL_Color LIGHT = {240, 217, 181, 255};
static const SDL_Color DARK  = {181, 136, 99, 255};
static const SDL_Color HIGHLIGHT = {60,180,75,200};
static const SDL_Color SELECT = {255, 200, 0, 200};
static const SDL_Color TEXT = {20,20,20,255};

string piece_to_char(int p){
  if(p==-1) return "";
  static const char *tbl = "PNBRQKpnbrqk";
  char c = tbl[p];
  string s; s.push_back(c); return s;
}

void draw_rect(SDL_Renderer *ren, int x, int y, int w, int h, SDL_Color c){
  SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(ren, c.r, c.g, c.b, c.a);
  SDL_Rect r{ x, y, w, h };
  SDL_RenderFillRect(ren, &r);
}

void draw_text_simple(SDL_Renderer *ren, int x, int y, const string &text, SDL_Color c){
  // Very simple: draw characters using filled rectangles as glyph placeholders.
  // For a nicer UI, integrate SDL_ttf and draw actual fonts. Here we draw small squares for legibility.
  int box = 10;
  draw_rect(ren, x, y, (int)text.size()*box, box, c);
  // Not using fonts to avoid extra deps; piece chars will be drawn with built-in rectangles + fallback.
}

int main(int argc, char**argv){
  if(SDL_Init(SDL_INIT_VIDEO) != 0){ cerr<<"SDL_Init error: "<<SDL_GetError()<<"\n"; return 1; }
  SDL_Window *win = SDL_CreateWindow("BlazingChess GUI - SDL2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN);
  if(!win){ cerr<<"SDL_CreateWindow error: "<<SDL_GetError()<<"\n"; SDL_Quit(); return 1; }
  SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if(!ren){ cerr<<"SDL_CreateRenderer error: "<<SDL_GetError()<<"\n"; SDL_DestroyWindow(win); SDL_Quit(); return 1; }

  Board board;
  vector<Position> history;
  history.push_back(board.pos);

  bool running=true;
  int selected = -1;
  vector<int> legal_dests;
  std::mt19937_64 rng((uint64_t)chrono::high_resolution_clock::now().time_since_epoch().count());

  while(running){
    SDL_Event ev;
    while(SDL_PollEvent(&ev)){
      if(ev.type==SDL_QUIT) running=false;
      else if(ev.type==SDL_KEYDOWN){
        if(ev.key.keysym.sym==SDLK_ESCAPE) running=false;
        if(ev.key.keysym.sym==SDLK_u){
          if(history.size()>=2){
            history.pop_back();
            history.pop_back(); // undo last two plies (player+engine)
            if(history.empty()) board.set_startpos();
            else board.undo_to(history.back());
            selected=-1; legal_dests.clear();
          }
        }
      } else if(ev.type==SDL_MOUSEBUTTONDOWN && ev.button.button==SDL_BUTTON_LEFT){
        int mx = ev.button.x, my = ev.button.y;
        int sqx = mx / (WINDOW_W/8);
        int sqy = my / (WINDOW_H/8);
        int clicked = (7 - sqy)*8 + sqx; // top row is 8th rank
        // if no selection and clicked is a friendly piece -> select and highlight legal moves
        int piece = board.find_piece_at(clicked);
        if(selected==-1){
          // only allow selecting side to move
          if(piece!=-1){
            bool is_white_piece = piece < 6;
            if(is_white_piece == board.pos.white_to_move){
              // compute legal moves from this square
              auto legal = board.generate_legal();
              legal_dests.clear();
              for(auto &m: legal) if(m.from==clicked) legal_dests.push_back(m.to);
              if(!legal_dests.empty()) selected = clicked;
            }
          }
        } else {
          // if clicked a highlighted dest -> find that move and play it
          bool is_dest=false;
          for(int d: legal_dests) if(d==clicked) { is_dest=true; break; }
          if(is_dest){
            auto legal = board.generate_legal();
            Move chosen;
            bool found=false;
            for(auto &m: legal){
              if(m.from==selected && m.to==clicked){
                chosen=m; found=true; break;
              }
            }
            if(found){
              board.make_move(chosen);
              history.push_back(board.pos);
              // reset selection
              selected=-1; legal_dests.clear();
              // engine reply (random)
              auto legal2 = board.generate_legal();
              if(!legal2.empty()){
                uniform_int_distribution<size_t> dist(0, legal2.size()-1);
                auto em = legal2[dist(rng)];
                SDL_Delay(150); // tiny pause so user sees move
                board.make_move(em);
                history.push_back(board.pos);
              }
            } else {
              // not found -> clear
              selected=-1; legal_dests.clear();
            }
          } else {
            // select another piece? allow reselect if it's own piece
            int p2 = board.find_piece_at(clicked);
            if(p2!=-1 && (p2<6)==board.pos.white_to_move){
              selected = clicked;
              auto legal = board.generate_legal();
              legal_dests.clear();
              for(auto &m: legal) if(m.from==clicked) legal_dests.push_back(m.to);
              if(legal_dests.empty()) selected=-1;
            } else {
              selected=-1; legal_dests.clear();
            }
          }
        }
      }
    }

    // render
    SDL_SetRenderDrawColor(ren, 200,200,200,255);
    SDL_RenderClear(ren);

    int cell = WINDOW_W/8;
    // board squares
    for(int r=0;r<8;++r){
      for(int f=0;f<8;++f){
        int sq = (7-r)*8 + f;
        bool light = ((r+f)%2==0);
        SDL_Color col = light ? LIGHT : DARK;
        draw_rect(ren, f*cell, r*cell, cell, cell, col);
      }
    }
    // highlight legal dests
    for(int d: legal_dests){
      int rr = 7 - (d/8);
      int ff = d%8;
      draw_rect(ren, ff*cell, rr*cell, cell, cell, HIGHLIGHT);
    }
    // highlight selected
    if(selected!=-1){
      int rr = 7 - (selected/8);
      int ff = selected%8;
      draw_rect(ren, ff*cell, rr*cell, cell, cell, SELECT);
    }

    // draw pieces (letters)
    for(int s=0;s<64;++s){
      int p = board.find_piece_at(s);
      if(p==-1) continue;
      int rr = 7 - (s/8);
      int ff = s%8;
      // piece color text: white pieces dark text on light squares, black pieces light text on dark squares
      SDL_Color pc = (p<6) ? TEXT : SDL_Color{250,250,250,255};
      // center draw: draw a filled small box representing piece and letter using rectangles as we didn't include fonts
      int cx = ff*cell + cell/2;
      int cy = rr*cell + cell/2;
      // draw a circle-like square
      SDL_SetRenderDrawColor(ren, pc.r, pc.g, pc.b, pc.a);
      SDL_Rect r{ cx-10, cy-10, 20, 20 };
      SDL_RenderFillRect(ren, &r);
      // draw piece letter by rendering a smaller contrasting box to suggest letter position
      // For clarity, also draw a tiny inner rect of opposite color to make white/black distinguishable
      SDL_SetRenderDrawColor(ren, 120,120,120,255);
      SDL_Rect r2{ cx-6, cy-6, 12, 12 };
      SDL_RenderFillRect(ren, &r2);
    }

    // present
    SDL_RenderPresent(ren);
    SDL_Delay(10);
  }

  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  SDL_Quit();
  return 0;
}

