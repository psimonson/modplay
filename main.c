#include <stdio.h>
#include <string.h>
#include <dirent.h>
#ifdef _WIN32
#include <io.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "c_logger.h"

#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"

#define MAXFILES 10

static Mix_Music *files[MAXFILES];

int load_files()
{
    struct dirent *dir;
    int res,i;
    DIR *p;
    
    p = opendir(".");
    if(p == NULL) {
        write_log(CLOG0, ERRMSG "Cannot open current directory.\n");
        return -1;
    }
    i = res = 0;
    while((dir = readdir(p)) != NULL)
        if(strcmp(dir->d_name,".") != 0 || strcmp(dir->d_name,"..") != 0) {
            if(strstr(dir->d_name,".mod") != NULL) {
                files[i] = Mix_LoadMUS(dir->d_name);
                if(files[i] == NULL) {
                    write_log(CLOG0, WRNMSG "Cannot load file: %s\n",
                            dir->d_name);
                    res++;
                    continue;
                }
                write_log(CLOG0, DBGMSG "Opened: %s at pos %d.\n",
                        dir->d_name, i);
                i++;
                write_log(CLOG0, WRNMSG "No mods in current directory.\n");
            }
        }
    closedir(p);
    write_log(CLOG0, WRNMSG "Current directory had %d mods in it.\n", i);
    write_log(CLOG0, DBGMSG "Closed current directory.\n");
    return res;
}

int
main()
{
    SDL_Surface *screen;
    int running,i;
    for(i=0; i<MAXFILES; i++)
        files[i] = NULL;
    init_logger();
    open_log(CLOG0, "modplay.log");
    if(SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) != 0) {
        write_log(CLOG0, ERRMSG "SDL_Init() failed!\n");
        close_log(CLOG0);
        return 1;
    }
    screen = SDL_SetVideoMode(320, 240, 24, SDL_SWSURFACE);
    if(screen == NULL) {
        write_log(CLOG0, ERRMSG "SDL_SetVideoMode() failed!\n");
        close_log(CLOG0);
        SDL_Quit();
        return 1;
    }
    write_log(CLOG0, DBGMSG "SDL_Init() successful!\n");
    Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
    if(load_files() != 0) {
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }
    write_log(CLOG0, DBGMSG "Playing mod test.mod\n");
    running = 1;
    while(running) {
        SDL_Event e;
        if(SDL_PollEvent(&e) != 0) {
            switch(e.type) {
            case SDL_QUIT:
                running = 0;
            break;
            case SDL_KEYDOWN:
                switch(e.key.keysym.sym) {
                case SDLK_ESCAPE:
                    running = 0;
                break;
                case 'p':
                    Mix_PlayMusic(files[i], 1);
                break;
                case 's':
                    Mix_HaltMusic();
                break;
                case 'n':
                    Mix_HaltMusic();
                    if(i<MAXFILES)
                        ++i;
                    else
                        i = 0;
                break;
                default:
                break;
                }
            break;
            default:
            break;
            }
        }
    }
    for(i=0; i<MAXFILES; i++)
        Mix_FreeMusic(files[i]);
    Mix_CloseAudio();
    SDL_Quit();
    return 0;
}
