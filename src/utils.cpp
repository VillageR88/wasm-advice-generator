#include <SDL2/SDL.h>
#include <cmath>
#include <algorithm>
#include <chrono>

void drawCircle(SDL_Renderer *renderer, int cx, int cy, int r, SDL_Color color, bool hover)
{
    static float glowAmount = 0.0f;
    static bool glowActive = false;
    static auto lastTime = std::chrono::steady_clock::now();
    static SDL_Texture *glowTexture = nullptr;
    const float speed = 1.5f; // glow rate per second
    const int glowRadius = r + 40;

    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<float> elapsed = now - lastTime;
    lastTime = now;
    float delta = elapsed.count(); // seconds

    // Update glowAmount only when needed
    if (hover && glowAmount < 1.0f)
    {
        glowActive = true;
        glowAmount = std::min(1.0f, glowAmount + speed * delta);
        if (glowAmount == 1.0f)
            glowActive = false;
    }
    else if (!hover && glowAmount > 0.0f)
    {
        glowActive = true;
        glowAmount = std::max(0.0f, glowAmount - speed * delta);
        if (glowAmount == 0.0f)
            glowActive = false;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Create glow texture if not exists
    if (glowAmount > 0.0f && !glowTexture)
    {
        glowTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA4444, SDL_TEXTUREACCESS_STATIC, glowRadius * 2, glowRadius * 2);
        SDL_SetTextureBlendMode(glowTexture, SDL_BLENDMODE_BLEND);
        Uint16 *pixels = new Uint16[glowRadius * glowRadius * 4];
        SDL_Color glowColor = {83, 255, 170, 255};

        for (int w = 0; w < glowRadius * 2; w++)
        {
            for (int h = 0; h < glowRadius * 2; h++)
            {
                int dx = glowRadius - w;
                int dy = glowRadius - h;
                double dist = sqrt(dx * dx + dy * dy);

                if (dist <= glowRadius)
                {
                    double glowFactor = std::max(0.0, (glowRadius - dist) / 40.0);
                    Uint8 alpha = static_cast<Uint8>(glowFactor * glowFactor * 80);
                    Uint16 pixel = ((glowColor.r >> 4) << 12) | ((glowColor.g >> 4) << 8) | ((glowColor.b >> 4) << 4) | (alpha >> 4);
                    pixels[w + h * glowRadius * 2] = pixel;
                }
                else
                {
                    pixels[w + h * glowRadius * 2] = 0;
                }
            }
        }

        SDL_UpdateTexture(glowTexture, nullptr, pixels, glowRadius * 2 * sizeof(Uint16));
        delete[] pixels;
    }

    // Render glow texture
    if (glowAmount > 0.0f && glowTexture)
    {
        SDL_SetTextureAlphaMod(glowTexture, static_cast<Uint8>(glowAmount * 255));
        SDL_Rect dstRect = {cx - glowRadius, cy - glowRadius, glowRadius * 2, glowRadius * 2};
        SDL_RenderCopy(renderer, glowTexture, nullptr, &dstRect);
    }

    for (int w = 0; w < r * 2; w++)
    {
        for (int h = 0; h < r * 2; h++)
        {
            int dx = r - w;
            int dy = r - h;
            double dist = sqrt(dx * dx + dy * dy);

            if (dist <= r)
            {
                Uint8 alpha = static_cast<Uint8>(255 - (std::max(0.0, dist - r + 1)) * 255);
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha);
                SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
            }
        }
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void drawRoundedRect(SDL_Renderer *renderer, int x, int y, int w, int h, int r, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    SDL_Rect centerRect = {x + r, y + r, w - 2 * r, h - 2 * r};
    SDL_RenderFillRect(renderer, &centerRect);

    SDL_Rect top = {x + r, y, w - 2 * r, r};
    SDL_Rect bottom = {x + r, y + h - r, w - 2 * r, r};
    SDL_Rect left = {x, y + r, r, h - 2 * r};
    SDL_Rect right = {x + w - r, y + r, r, h - 2 * r};
    SDL_RenderFillRect(renderer, &top);
    SDL_RenderFillRect(renderer, &bottom);
    SDL_RenderFillRect(renderer, &left);
    SDL_RenderFillRect(renderer, &right);

    for (int dx = -1; dx <= 1; dx += 2)
    {
        for (int dy = -1; dy <= 1; dy += 2)
        {
            int cx = x + (dx == -1 ? r : w - r);
            int cy = y + (dy == -1 ? r : h - r);
            for (int w = 0; w < r; ++w)
                for (int h = 0; h < r; ++h)
                    if ((w * w + h * h) <= r * r)
                        SDL_RenderDrawPoint(renderer, cx + dx * w, cy + dy * h);
        }
    }
}