#include <cstdio>

#include <SDL.h>

int
main(int argc, char *args[]) {
    SDL_version v;
    SDL_GetVersion(&v);
    std::printf("SDL version %d.%d.%d\n", v.major, v.minor, v.patch);
    return 0;
}
