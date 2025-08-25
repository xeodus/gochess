#include <print>
#include <SDL2/SDL.h>
#include <string>

static const int WINDOW_W = 1080;
static const int WINDOW_H = 1920;
static const SDL_Color LIGHT = {240, 217, 181, 255};
static const SDL_Color DARK  = {181, 136, 99, 255};
static const SDL_Color HIGHLIGHT = {60,180,75,200};
static const SDL_Color SELECT = {255, 200, 0, 200};
static const SDL_Color TEXT = {20,20,20,255};

std::string pieceToChar(int p) {
    if (p == -1) return "";
    static const char* tbl = "PNBRQKpnbrqk";
    char c = tbl[p];
    std::string s;
    s.push_back(c);
    return s;
}

void drawRectangle(SDL_Renderer* ren, int x, int y, int w, int h, SDL_Color c) {
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ren, c.r, c.g, c.b, c.a);
    SDL_Rect r{ x, y, w, h };
    SDL_RenderFillRect(ren, &r);
}

void drawTextSimple(SDL_Renderer *ren, int x, int y, const std::string &text, SDL_Color c){
  // Very simple: draw characters using filled rectangles as glyph placeholders.
  // For a nicer UI, integrate SDL_ttf and draw actual fonts. Here we draw small squares for legibility.
  int box = 10;
  drawRectangle(ren, x, y, (int)text.size()*box, box, c);
  // Not using fonts to avoid extra deps; piece chars will be drawn with built-in rectangles + fallback.
}

int main() {
    std::println("Hello world!");
}
