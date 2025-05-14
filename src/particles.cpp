#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <random>
#include <vector>
#include <cmath>
#include <algorithm>

struct Star
{
    float x, y;
    float speed;
    int size;
};

struct Particle
{
    float x, y;
    float velX, velY;
    float size;
    int lifespan;
    SDL_Color color;
};

struct Wormhole
{
    float x, y;
    float baseRadius;
    float pulseOffset;
    float pulseSpeed;
    float velX, velY;
    int layers;
    SDL_Color color;
};

std::vector<Particle> particles;
std::vector<Star> stars;
Wormhole wormhole;
std::vector<SDL_Color> twoColor = {{255, 255, 255, 255}, {83, 255, 170, 255}};
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dis(-1.0, 1.0);
std::uniform_int_distribution<int> colorDis(0, 1);
std::uniform_int_distribution<> sizeDis(2, 5);

void renderParticles(SDL_Renderer *renderer)
{
    for (auto &particle : particles)
    {
        int alpha = (particle.lifespan * 255) / 100;
        SDL_SetRenderDrawColor(renderer, particle.color.r, particle.color.g, particle.color.b, alpha);
        SDL_Rect particleRect = {(int)particle.x, (int)particle.y, (int)particle.size, (int)particle.size};
        SDL_RenderFillRect(renderer, &particleRect);
    }
}

void generateParticles(int x, int y)
{
    for (int i = 0; i < 30; i++)
    {
        Particle p;
        p.x = x;
        p.y = y;
        p.velX = dis(gen) * 2;
        p.velY = dis(gen) * 2;
        p.size = sizeDis(gen);
        p.lifespan = 100;
        p.color = twoColor[colorDis(gen)];
        particles.push_back(p);
    }
}

void updateParticles()
{
    for (auto it = particles.begin(); it != particles.end();)
    {
        it->x += it->velX;
        it->y += it->velY;
        it->velY += 0.1;
        it->lifespan--;
        if (it->lifespan <= 0)
        {
            it = particles.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void initStars(int count, int screenWidth, int screenHeight)
{
    stars.clear();
    for (int i = 0; i < count; ++i)
    {
        Star s;
        s.x = rand() % screenWidth;
        s.y = rand() % screenHeight;
        s.speed = 0.4f + static_cast<float>(rand() % 300) / 100.0f;
        s.size = 1 + rand() % 2;
        stars.push_back(s);
    }
}

void updateStars(int screenWidth, int screenHeight)
{
    for (Star &s : stars)
    {
        s.x += s.speed;
        if (s.x > screenWidth)
        {
            s.x = 0;
            s.y = rand() % screenHeight;
        }
    }
}

void renderStars(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (const Star &s : stars)
    {
        SDL_Rect starRect = {static_cast<int>(s.x), static_cast<int>(s.y), s.size, s.size};
        SDL_RenderFillRect(renderer, &starRect);
    }
}

void initWormhole(int screenWidth, int screenHeight)
{
    std::uniform_real_distribution<> posDisX(screenWidth * 0.2, screenWidth * 0.8);
    std::uniform_real_distribution<> posDisY(screenHeight * 0.2, screenHeight * 0.8);
    std::uniform_real_distribution<> radiusDis(40.0, 60.0);

    wormhole.x = posDisX(gen);
    wormhole.y = posDisY(gen);
    wormhole.baseRadius = radiusDis(gen);
    wormhole.velX = dis(gen) * 0.5;
    wormhole.velY = dis(gen) * 0.5;
}

void updateWormhole(int screenWidth, int screenHeight)
{
    std::uniform_real_distribution<> velChange(-0.2, 0.2);

    wormhole.x += wormhole.velX;
    wormhole.y += wormhole.velY;

    float margin = 120.0f;
    if (wormhole.x < margin)
        wormhole.x = margin;
    if (wormhole.x > screenWidth - margin)
        wormhole.x = screenWidth - margin;
    if (wormhole.y < margin)
        wormhole.y = margin;
    if (wormhole.y > screenHeight - margin)
        wormhole.y = screenHeight - margin;

    wormhole.velX += velChange(gen) * 0.02;
    wormhole.velY += velChange(gen) * 0.02;
    wormhole.velX = std::clamp(wormhole.velX, -0.5f, 0.5f);
    wormhole.velY = std::clamp(wormhole.velY, -0.5f, 0.5f);
}

void renderWormhole(SDL_Renderer *renderer)
{
    float coreRadius = wormhole.baseRadius * 0.9f;
    const int numVeils = 10;
    float maxVeilRadius = coreRadius * 1.4f;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for (int i = 0; i < numVeils; i++)
    {
        float t = static_cast<float>(i) / (numVeils - 1);
        float radius = coreRadius + (maxVeilRadius - coreRadius) * t;
        int alpha = static_cast<int>(178 * (1.0f - t));

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, alpha);
        for (int x = -radius; x <= radius; x++)
        {
            for (int y = -radius; y <= radius; y++)
            {
                float r = sqrt(x * x + y * y);
                if (r <= radius && r > coreRadius)
                {
                    float offset = t * 2.0f;
                    int xDistorted = static_cast<int>(x + offset * (x / r));
                    int yDistorted = static_cast<int>(y + offset * (y / r));
                    SDL_RenderDrawPoint(renderer, wormhole.x + xDistorted, wormhole.y + yDistorted);
                }
            }
        }
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for (int x = -coreRadius; x <= coreRadius; x++)
    {
        for (int y = -coreRadius; y <= coreRadius; y++)
        {
            if (x * x + y * y <= coreRadius * coreRadius)
            {
                SDL_RenderDrawPoint(renderer, wormhole.x + x, wormhole.y + y);
            }
        }
    }
}