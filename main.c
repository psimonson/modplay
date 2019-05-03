#include <stdio.h>
#include <stdlib.h>
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
#define MAXNAME  64

struct file {
    char name[MAXNAME];
    Mix_Music *music;
} files[MAXFILES];

int load_files(const char *dirname)
{
    struct dirent *dir;
    int res,i;
    DIR *p;
    
    if(dirname == NULL)
        p=opendir(".");
    else
        p=opendir(dirname);
    if(p == NULL) {
        write_log(CLOG0,ERRMSG "Cannot open current directory.\n");
        return -1;
    }
    i = res = 0;
    while((dir=readdir(p)) != NULL)
        if(strcmp(dir->d_name,".") != 0 || strcmp(dir->d_name,"..") != 0) {
            if(strstr(dir->d_name,".mod") != NULL) {
                char name[260];
                memset(name,0,260);
#ifdef _WIN32
                sprintf(name,"%s\\%s",dirname,dir->d_name);
#else
                sprintf(name,"%s/%s",dirname,dir->d_name);
#endif
                files[i].music = Mix_LoadMUS(name);
                if(files[i].music == NULL) {
                    write_log(CLOG0,WRNMSG "Cannot load file: %s\n",
                            dir->d_name);
                    res++;
                    continue;
                }
                strncpy(files[i].name,dir->d_name,strlen(dir->d_name));
                write_log(CLOG0,DBGMSG "Opened: %s at pos %d.\n",
                        dir->d_name,i);
                if(i>=MAXFILES)
                    break;
                i++;
            }
        }
    closedir(p);
    write_log(CLOG0,WRNMSG "MOD files loaded: %d\n", i);
    return res;
}

int
init_text (TTF_Font **font,const char *name,int size)
{
    char fname[260];
    if(name == NULL) {
        memset(fname,0,260);
        sprintf(fname,"fonts/LiberationMono-Regular.ttf");
        *font = TTF_OpenFont(fname,(size<0 ? 10 : size));
    } else {
        *font = TTF_OpenFont(name,(size<0 ? 10 : size));
    }
    if(*font == NULL) {
        write_log(CLOG0,ERRMSG "Cannot open %s font.\n",
            (name == NULL ? fname : name));
        close_log(CLOG0);
        return 1;
    }
    write_log(CLOG0,DBGMSG "File %s opened with success.\n",
            (name == NULL ? fname : name));
    return 0;
}

void
quit_text(TTF_Font **font)
{
    write_log(CLOG0,DBGMSG "Destroying font...\n");
    TTF_CloseFont(*font);
    *font = NULL;
}

SDL_Surface*
put_text (TTF_Font *font,const char *text,int x,int y,
        SDL_Rect *rect,SDL_Color color)
{
    SDL_Surface *surface;
    surface = TTF_RenderText_Solid(font, text, color);
    if(surface == NULL) {
        write_log(CLOG0,ERRMSG "Cannot create text surface.\n");
        return NULL;
    }
    if(rect == NULL)
        return surface;
    rect->x = x;
    rect->y = y;
    rect->w = surface->w;
    rect->h = surface->h;
    return surface;
}

int
main ()
{
    SDL_Surface *screen;
    SDL_Surface *modname;
    SDL_Rect modrect;
    TTF_Font *font;
    int running,i;
    char stopped;
    for(i=0; i<MAXFILES; i++)
        files[i].music=NULL;
    init_logger();
    open_log(CLOG0,"modplay.log");
    if(SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) != 0) {
        write_log(CLOG0,ERRMSG "SDL_Init() failed!\n");
        close_log(CLOG0);
        return 1;
    }
    if(TTF_Init() != 0) {
        write_log(CLOG0,ERRMSG "TTF_Init() failed!\n");
        close_log(CLOG0);
        SDL_Quit();
        return 1;
    }
    screen = SDL_SetVideoMode(320,240,24,SDL_SWSURFACE);
    if(screen == NULL) {
        write_log(CLOG0,ERRMSG "SDL_SetVideoMode() failed!\n");
        close_log(CLOG0);
        SDL_Quit();
        return 1;
    }
    write_log(CLOG0,DBGMSG "SDL_Init() successful!\n");
    if(init_text(&font, NULL, 10) != 0)
        return 1;
    Mix_OpenAudio(22050,MIX_DEFAULT_FORMAT,2,4096);
    if(load_files("music") != 0) {
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }
    i=0;
    stopped=running=1;
    Mix_PlayMusic(files[i].music, 1);
    while(running) {
        SDL_Color color = {0,170,200,0};
        SDL_Event e;
        unsigned int start = SDL_GetTicks();
        if(SDL_PollEvent(&e) != 0) {
            switch(e.type) {
            case SDL_QUIT:
                running=0;
            break;
            case SDL_KEYDOWN:
                switch(e.key.keysym.sym) {
                case SDLK_ESCAPE:
                    running=0;
                break;
                case 'p':
                    Mix_PlayMusic(files[i].music,1);
                break;
                case 'o':
                    stopped = !stopped;
                    if(stopped == 1)
                        Mix_ResumeMusic();
                    else
                        Mix_PauseMusic();
                break;
                case 's':
                    Mix_HaltMusic();
                break;
                case 'n':
                    Mix_HaltMusic();
                    if(i>=MAXFILES) i=0;
                    else i++;
                    if(files[i].music != NULL)
                        Mix_PlayMusic(files[i].music,1);
                break;
                default:
                break;
                }
            break;
            default:
            break;
            }
        }
        SDL_FillRect(screen,NULL,SDL_MapRGB(screen->format,0,0,180));
        
        /* display current file name */
        {
            char name[256];
            memset(name,0,256);
            if(files[i].music != NULL)
                sprintf(name,"MOD [%d]: %s",i,files[i].name);
            else
                sprintf(name,"MOD [%d]: <null>",i);
            modname = put_text(font,name,0,0,&modrect,color);
        }
        SDL_BlitSurface(modname,NULL,screen,&modrect);
        SDL_FreeSurface(modname);
        
        SDL_UpdateRect(screen,0,0,0,0);
        
        if(1000/60 > SDL_GetTicks()-start)
            SDL_Delay(1000/60-(SDL_GetTicks()-start));
    }
    for(i=0; i<MAXFILES; i++)
        Mix_FreeMusic(files[i].music);
    quit_text(&font);
    Mix_CloseAudio();
    SDL_Quit();
    return 0;
}
