#include <SDL2/SDL.h>
#include <random>

void drawCircle(SDL_Renderer *renderer, int cx, int cy, int r, SDL_Color color, bool hover)
{
    static float glowAmount = 0.0f;
    const float speed = 0.05f; // Controls how fast glow fades in/out

    // Animate glowAmount
    if (hover && glowAmount < 1.0f)
    {
        glowAmount = std::min(1.0f, glowAmount + speed);
    }
    else if (!hover && glowAmount > 0.0f)
    {
        glowAmount = std::max(0.0f, glowAmount - speed);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Draw fading glow
    if (glowAmount > 0.0f)
    {
        int glowRadius = r + 40;
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
                    Uint8 alpha = static_cast<Uint8>(glowFactor * glowFactor * 80 * glowAmount);
                    SDL_SetRenderDrawColor(renderer, glowColor.r, glowColor.g, glowColor.b, alpha);
                    SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
                }
            }
        }
    }

    // Draw solid circle
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