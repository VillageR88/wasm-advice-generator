#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <emscripten/emscripten.h>
#include <emscripten/fetch.h>
#include <emscripten/html5.h>
#include <string>
#include <regex>
#include <sstream>
#include "particles.cpp"
#include "fetch.cpp"
#include "utils.cpp"

const SDL_Color titleColor = {83, 255, 170, 255};
const SDL_Color quoteColor = {206, 227, 233, 255};
const SDL_Color buttonColor = {255, 255, 255, 255};

SDL_Window *win;
SDL_Renderer *renderer;
TTF_Font *fontTitle, *fontQuote, *fontButton;
SDL_Texture *textTitle, *buttonTexture, *patternTexture;
std::vector<SDL_Texture *> quoteLines;
std::vector<SDL_Rect> quoteLineRects;
SDL_Cursor *handCursor, *defaultCursor;
SDL_Surface *surfButton;
std::string surfTitleText;
std::string surfQuoteText;
Uint32 lastTicks = 0;

int titleW, titleH, buttonD, lineCount;
float buttonX, buttonY;
double prevWidth = -1, prevHeight = -1;

std::vector<std::string> wrapText(const std::string &text, TTF_Font *font, int maxWidth)
{
    std::vector<std::string> lines;
    std::istringstream words(text);
    std::string word, line;

    while (words >> word)
    {
        std::string testLine = line.empty() ? word : line + " " + word;
        int w;
        TTF_SizeText(font, testLine.c_str(), &w, nullptr);
        if (w > maxWidth && !line.empty())
        {
            lines.push_back(line);
            line = word;
        }
        else
        {
            line = testLine;
        }
    }
    if (!line.empty())
        lines.push_back(line);

    return lines;
}

void renderCenteredWrappedText(const std::string &text, TTF_Font *font, SDL_Color color, int maxWidth, int centerX, int startY)
{
    for (SDL_Texture *tex : quoteLines)
        SDL_DestroyTexture(tex);
    quoteLines.clear();
    quoteLineRects.clear();

    auto lines = wrapText(text, font, maxWidth);
    lineCount = lines.size();
    int y = startY;

    for (const std::string &line : lines)
    {
        SDL_Surface *surf = TTF_RenderText_Blended(font, line.c_str(), color);
        SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_Rect rect = {(int)(centerX - surf->w / 2), y, surf->w, surf->h};
        quoteLines.push_back(tex);
        quoteLineRects.push_back(rect);
        y += surf->h + 0;
        SDL_FreeSurface(surf);
    }
}

void renderLetterSpacedText(const std::string &text, TTF_Font *font, SDL_Color color, int centerX, int y, int spacing)
{
    int totalWidth = 0;
    std::vector<SDL_Texture *> textures;
    std::vector<int> widths;

    for (char c : text)
    {
        std::string s(1, c);
        SDL_Surface *surf = TTF_RenderText_Blended(font, s.c_str(), color);
        SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
        totalWidth += surf->w + spacing;
        textures.push_back(tex);
        widths.push_back(surf->w);
        SDL_FreeSurface(surf);
    }

    int x = centerX - (totalWidth - spacing) / 2;

    for (size_t i = 0; i < textures.size(); ++i)
    {
        SDL_Rect dst = {x, y, widths[i], TTF_FontHeight(font)};
        SDL_RenderCopy(renderer, textures[i], nullptr, &dst);
        SDL_DestroyTexture(textures[i]);
        x += widths[i] + spacing;
    }
}

void updateViewWithFetchedData(std::string advice, std::string adviceId)
{
    SDL_DestroyTexture(textTitle);
    surfTitleText = "ADVICE #" + adviceId;
    surfQuoteText = advice;
}

void loop()
{
    Uint32 currentTicks = SDL_GetTicks();
    double dpr = emscripten_get_device_pixel_ratio();

    float deltaTime = (currentTicks - lastTicks) / 1000.0f;
    lastTicks = currentTicks;
    double outerWidth, outerHeight;
    emscripten_get_element_css_size("body", &outerWidth, &outerHeight);

    outerWidth = outerWidth * dpr;
    outerHeight = outerHeight * dpr;
    if (outerWidth != prevWidth || outerHeight != prevHeight)
    {
        prevWidth = outerWidth;
        prevHeight = outerHeight;
        initStars(120, outerWidth, outerHeight);
        initWormhole(outerWidth, outerHeight);
    }

    SDL_SetWindowSize(win, outerWidth, outerHeight);
    SDL_SetRenderDrawColor(renderer, 32, 39, 51, 255);
    SDL_RenderClear(renderer);

    updateStars(outerWidth, outerHeight);
    updateWormhole(outerWidth, outerHeight);
    renderStars(renderer);
    renderWormhole(renderer);
    renderParticles(renderer);
    updateParticles();
    int innerWidth = outerWidth <= 600 ? outerWidth - 40 : 540;
    int innerHeight = 220 + lineCount * 40;
    int contentX = (outerWidth - innerWidth) / 2;
    int contentY = (outerHeight - innerHeight) / 2;

    SDL_SetRenderDrawColor(renderer, 49, 58, 72, 255);
    SDL_Rect innerWindow = {(int)(outerWidth / 2) - innerWidth / 2, (int)(outerHeight / 2) - innerHeight / 2, innerWidth, innerHeight};
    drawRoundedRect(renderer, innerWindow.x, innerWindow.y, innerWindow.w, innerWindow.h, 20, {49, 58, 72, 255});

    renderLetterSpacedText(surfTitleText, fontTitle, titleColor, outerWidth / 2, innerWindow.y + 50, 4);
    renderCenteredWrappedText(surfQuoteText.c_str(), fontQuote, quoteColor, innerWidth - 80, contentX + innerWidth / 2, contentY + 90);
    for (size_t i = 0; i < quoteLines.size(); i++)
        SDL_RenderCopy(renderer, quoteLines[i], NULL, &quoteLineRects[i]);

    buttonD = 64;
    buttonX = (innerWidth - buttonD) / 2 + contentX;
    buttonY = innerHeight + contentY - buttonD / 2;

    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
            emscripten_cancel_main_loop();

        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
        {
            int mx = e.button.x, my = e.button.y;
            bool overBtn = mx >= buttonX && mx <= buttonX + buttonD && my >= buttonY && my <= buttonY + buttonD;
            if (overBtn)
            {

                perform_fetch(updateViewWithFetchedData);
                generateParticles(mx, my);
            }
        }
    }

    int mx, my;
    SDL_GetMouseState(&mx, &my);
    bool overBtn = mx >= buttonX && mx <= buttonX + buttonD && my >= buttonY && my <= buttonY + buttonD;
    SDL_SetCursor(overBtn ? handCursor : defaultCursor);

    drawCircle(renderer, outerWidth / 2, buttonY + buttonD / 2, buttonD / 2, {83, 255, 170, 25}, overBtn);
    SDL_Rect buttonRect = {(int)buttonX + 21, (int)buttonY + 21, 24, 24};
    SDL_RenderCopy(renderer, buttonTexture, NULL, &buttonRect);

    SDL_Rect patternRect = {innerWindow.x + 50, innerWindow.y - 90 + innerWindow.h, innerWidth - 100, 18};
    SDL_RenderCopy(renderer, patternTexture, NULL, &patternRect);

    SDL_RenderPresent(renderer);
}

int main()
{
    SDL_AudioSpec wav_spec_click, wav_spec_hover;
    double dpr = emscripten_get_device_pixel_ratio();

    surfTitleText = "ADVICE #186";
    surfQuoteText = "One of the single best things about being an adult, is being able to buy as much LEGO as you want.";
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    TTF_Init();
    IMG_Init(IMG_INIT_PNG);
    fontTitle = TTF_OpenFont("assets/fonts/Manrope/Manrope-ExtraBold.ttf", 13);
    fontQuote = TTF_OpenFont("assets/fonts/Manrope/Manrope-ExtraBold.ttf", 28);
    fontButton = TTF_OpenFont("assets/fonts/Manrope/Manrope-ExtraBold.ttf", 32);
    handCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    defaultCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    double outerWidth;
    double outerHeight;
    emscripten_get_element_css_size("body", &outerWidth, &outerHeight);
    win = SDL_CreateWindow("Advice App Container", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, outerWidth, outerHeight, 0);
    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    buttonTexture = IMG_LoadTexture(renderer, "assets/images/icon-dice.png");
    patternTexture = IMG_LoadTexture(renderer, "assets/images/pattern-divider-desktop.png");

    EM_ASM({
        function resizeCanvas()
        {
            var dpr = window.devicePixelRatio || 1;
            var canvas = document.getElementById('canvas');
            canvas.width = window.innerWidth * dpr;
            canvas.height = window.innerHeight * dpr;
        }
        window.addEventListener('resize', resizeCanvas);
        resizeCanvas();
    });

    emscripten_set_main_loop(loop, 0, 1);
    return 0;
}