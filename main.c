/*
 * main.c - main source file for modplay, a simple MOD file player.
 *
 * Author: Philip R. Simonson
 * Date  : 2019/05/03
 **********************************************************************
 */

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
#include "config.h"

#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"

#define APPVER        "v0.1"
#define MAXNAME       64
#define WINDOW_WIDTH  320
#define WINDOW_HEIGHT 64

struct file {
    char name[MAXNAME];
    Mix_Music *music;
} files[MAXFILES];

int
load_files (const char *dirname)
{
    struct dirent *dir;
    int res,i;
    DIR *p;
    
    if(dirname == NULL)
        p=opendir(".");
    else
        p=opendir(dirname);
    if(p == NULL) {
        if(dirname != NULL)
            write_log(CLOG0,ERRMSG "Cannot open %s directory.\n",dirname);
        else
            write_log(CLOG0,ERRMSG "Cannot open current directory.\n");
        return -1;
    }
    i = res = 0;
    while(i<MAXFILES && (dir=readdir(p)) != NULL)
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
quit_text (TTF_Font **font)
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
main (int argc, char **argv)
{
    SDL_Surface *screen;
    SDL_Surface *modname;
    SDL_Rect modrect;
    TTF_Font *font;
    int running,i;
    char startup;
    for(i=0; i<MAXFILES; i++)
        files[i].music=NULL;
    init_logger();
    open_log(CLOG0,"/tmp/modplay.log");
    if(argc>3) {
        write_log(CLOG0,ERRMSG "%s [-d dirname]\n",argv[0]);
        close_log(CLOG0);
        return 1;
    }
    for(i=argc-1;i>0;--i) {
        if(argv[i][0] == '-') {
            while(*++argv[i] != 0) {
                switch(*argv[i]) {
                case 'd':
                    dirname = argv[i+1];
                    if(dirname[0] == '\0') {
                        write_log(CLOG0,ERRMSG "No directory given.\n");
                        close_log(CLOG0);
                        return 1;
                    }
                break;
                default:
                    write_log(CLOG0,ERRMSG "Unknown option '%c'.\n",*argv[i]);
                    close_log(CLOG0);
                    return 1;
                break;
                }
            }
        }
    }
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
    screen = SDL_SetVideoMode(WINDOW_WIDTH,WINDOW_HEIGHT,24,SDL_SWSURFACE);
    if(screen == NULL) {
        write_log(CLOG0,ERRMSG "SDL_SetVideoMode() failed!\n");
        close_log(CLOG0);
        SDL_Quit();
        return 1;
    }
    write_log(CLOG0,DBGMSG "SDL_Init() successful!\n");
    SDL_WM_SetCaption("MODPlay " APPVER,NULL);
    if(init_text(&font,fontname,10) != 0)
        return 1;
    Mix_OpenAudio(22050,MIX_DEFAULT_FORMAT,2,4096);
    if(load_files(dirname) != 0) {
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }
    i=startup=0;
    running=1;
    while(running) {
        SDL_Color color = {200,200,0,0};
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
                    if(Mix_PausedMusic() == 1)
                        Mix_ResumeMusic();
                    else
                        Mix_PauseMusic();
                break;
                case 's':
                    Mix_HaltMusic();
                break;
                case 'b':
                    i--;
                    if(i<=0) i=0;
                    else if(i>=MAXFILES) i=MAXFILES;
                    Mix_HaltMusic();
                    startup=0;
                break;
                case 'n':
                    i++;
                    if(i<0) i=0;
                    else if(i>=MAXFILES) i=MAXFILES;
                    Mix_HaltMusic();
                    startup=0;
                break;
                default:
                break;
                }
            break;
            default:
            break;
            }
        }
        
        /* auto play music */
        if(Mix_PausedMusic() == 0) {
            if(Mix_PlayingMusic() == 0) {
                if(startup == 0) {
                    Mix_PlayMusic(files[i].music,1);
                    startup = 1;
                } else {
                    if(i<0 || i>=MAXFILES) i=0;
                    else {
                        if(files[i].music != NULL) ++i;
                        else i=0;
                        Mix_PlayMusic(files[i].music,1);
                    }
                }
            }
        }
        
        /* clear display to blue */
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

        /* put help message */
        modname = put_text(font,"[Play:p | Pause/Resume:o | Stop:s]",0,
                            modrect.h*2,&modrect,color);
        SDL_BlitSurface(modname,NULL,screen,&modrect);
        SDL_FreeSurface(modname);

        /* put rest of help */
        modname = put_text(font,"[Next Song:n | Previous Song:b]",0,
                            modrect.h*3,&modrect,color);
        SDL_BlitSurface(modname,NULL,screen,&modrect);
        SDL_FreeSurface(modname);

        /* print escape quits */
        modname = put_text(font,"ESC: to quit this mod player...",0,
                            modrect.h*4,&modrect,color);
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
