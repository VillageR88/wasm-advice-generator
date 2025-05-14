#include <SDL2/SDL.h>
#include <emscripten/fetch.h>
#include <string>
#include <regex>
#include <sstream>
std::string returnedAdvice;
std::string returnedAdviceId;

static std::function<void(std::string, std::string)> g_callback;

void on_success(emscripten_fetch_t *fetch)
{
    std::string data(fetch->data, fetch->numBytes);

    std::regex advicePattern(".*\"advice\"\\s*:\\s*\"((?:[^\"\\\\]|\\\\.)*)\".*");
    std::regex idPattern(".*\"id\"\\s*:\\s*(\\d+).*");
    std::smatch match;

    std::string advice, id;

    if (std::regex_match(data, match, advicePattern))
    {
        advice = std::regex_replace(match[1].str(), std::regex("\\\\"), "");
    }

    if (std::regex_match(data, match, idPattern))
    {
        id = match[1].str();
    }

    emscripten_fetch_close(fetch);

    if (g_callback)
    {
        g_callback(advice, id);
    }
}

void on_error(emscripten_fetch_t *fetch)
{
    emscripten_fetch_close(fetch);
}

void perform_fetch(std::function<void(std::string, std::string)> callback)
{
    g_callback = callback;

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.onsuccess = on_success;
    attr.onerror = on_error;
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    emscripten_fetch(&attr, "https://api.adviceslip.com/advice");
}
