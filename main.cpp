#include <iostream>
#include "motion.h"


using namespace std;

//int main(int argc, char *argv[]){
//    cout << "Hello world!" << endl;
//    Motion *motionDetector = new Motion();
//
//    try{
//        motionDetector->iniciarPrueba();
//        motionDetector->diferenceFilter();
//        motionDetector->erosionFilter();
//
//    } catch (Excepcion &e){
//        cout << e.getMessage();
//    }
//
//
//
//
//
//
//    delete motionDetector;
//
//    return 0;
//}

/* libSDL and libVLC sample code
 * Copyright © 2008 Sam Hocevar <sam@zoy.org>
 * license: [http://en.wikipedia.org/wiki/WTFPL WTFPL] */

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#include <SDL/SDL.h>
#include <SDL/SDL_mutex.h>

#include <vlc/vlc.h>

#define WIDTH 640
#define HEIGHT 480

#define VIDEOWIDTH 640
#define VIDEOHEIGHT 480

struct ctx
{
    SDL_Surface *surf;
    SDL_mutex *mutex;
};

static SDL_Surface *backgroundFrame = NULL;
static SDL_Surface *currentFrame = NULL;
static int nFrames = 0;
static Motion *motionDetector;
static UIImageEncoder imEncoder;
static ImagenGestor imGestor;

static void *lock(void *data, void **p_pixels)
{
    struct ctx *cotx = (ctx *)data;

    SDL_LockMutex(cotx->mutex);
    SDL_LockSurface(cotx->surf);
    *p_pixels = cotx->surf->pixels;
    return NULL; /* picture identifier, not needed here */
}

static void unlock(void *data, void *id, void *const *p_pixels)
{
    struct ctx *cotx = (ctx *)data;

    /* VLC just rendered the video, but we can also render stuff */
    uint16_t *pixels = (uint16_t *)*p_pixels;
    int x, y;
//
//    for(y = 10; y < 40; y++)
//        for(x = 10; x < 40; x++)
//            if(x < 13 || y < 13 || x > 36 || y > 36)
//                pixels[y * VIDEOWIDTH + x] = 0xffff;
//            else
//                pixels[y * VIDEOWIDTH + x] = 0x0;
//
    SDL_Rect rect;
    rect.x = 90;
    rect.y = 90;
    rect.w = 20;
    rect.h = 20;

//    SDL_FillRect( cotx->surf, &rect, SDL_MapRGB(cotx->surf->format, cRojo.r, cRojo.g, cRojo.b) );

    if (nFrames > 0){
        //Guardamos la imagen anterior para que sea el fondo sobre el que basar el movimiento
        SDL_BlitSurface(currentFrame, NULL, backgroundFrame, NULL);
    } else if (nFrames == 0){
        if (currentFrame != NULL)
            SDL_FreeSurface(currentFrame);

        if (backgroundFrame != NULL)
            SDL_FreeSurface(backgroundFrame);


        currentFrame = SDL_CreateRGBSurface(SDL_SWSURFACE, cotx->surf->w, cotx->surf->h, 24,
                                            cotx->surf->format->Rmask,
                                            cotx->surf->format->Gmask,
                                            cotx->surf->format->Bmask,
                                            cotx->surf->format->Amask);

        backgroundFrame = SDL_CreateRGBSurface(SDL_SWSURFACE, cotx->surf->w, cotx->surf->h, 24,
                                            cotx->surf->format->Rmask,
                                            cotx->surf->format->Gmask,
                                            cotx->surf->format->Bmask,
                                            cotx->surf->format->Amask);
    }

    if (nFrames > 0){
        uint16_t *dstPixels = (uint16_t *)currentFrame->pixels;

        for(y = 0; y < cotx->surf->h; y++)
            for(x = 0; x < cotx->surf->w; x++)
                imGestor.putpixel(currentFrame, x, y, imGestor.getpixel(cotx->surf, x, y));
                //dstPixels[y * VIDEOWIDTH + x] = pixels[y * VIDEOWIDTH + x]; //Solo si la surface es de 16 bits


        motionDetector->diferenceFilter(backgroundFrame, currentFrame);
        motionDetector->erosionFilter();
        motionDetector->showDiffFilter(cotx->surf);

//        SDL_FillRect( currentFrame, &rect, SDL_MapRGB(currentFrame->format, cRojo.r, cRojo.g, cRojo.b) );
//        imEncoder.IMG_SaveJPG("origen.jpg", currentFrame, 95);
//        currentFrame = SDL_CreateRGBSurface(SDL_SWSURFACE, cotx->surf->w, cotx->surf->h, 24,
//                                            cotx->surf->format->Rmask,
//                                            cotx->surf->format->Gmask,
//                                            cotx->surf->format->Bmask,
//                                            cotx->surf->format->Amask);
    }

    nFrames++;

    SDL_UnlockSurface(cotx->surf);
    SDL_UnlockMutex(cotx->mutex);

    assert(id == NULL); /* picture identifier, not needed here */
}

static void display(void *data, void *id)
{
    /* VLC wants to display the video */
    (void) data;
    assert(id == NULL);
}

int main(int argc, char *argv[])
{
    libvlc_instance_t *libvlc;
    libvlc_media_t *m;
    libvlc_media_player_t *mp;
    char const *vlc_argv[] =
    {
        //"--no-audio", /* skip any audio track */
        //"--no-xlib", /* tell VLC to not use Xlib */
        "--intf", "dummy",          // no interface
        "--vout", "dummy",          // we don't want video (output)
        "--no-audio",               // we don't want audio (decoding)
        "--no-video-title-show",    // nor the filename displayed
        "--no-stats",               // no stats
        "--no-sub-autodetect-file", // we don't want subtitles
        //"--no-inhibit",             // we don't want interfaces
        "--no-disable-screensaver", // we don't want interfaces
        "--no-snapshot-preview"    // no blending in dummy vout
        ,"-vvv" /* be much more verbose for debugging */
        ,"--no-osd"
        ,"--dshow-aspect-ratio=4:3"
        ,"--dshow-fps=30"
        ,"--dshow-size=640x480"
        ,"--live-caching=250"
    };
    int vlc_argc = sizeof(vlc_argv) / sizeof(*vlc_argv);

    SDL_Surface *screen, *empty;
    SDL_Event event;
    SDL_Rect rect;
    int done = 0, action = 0, pause = 0, n = 0;

    struct ctx ctx;

//    if(argc < 2)
//    {
//        printf("Usage: %s <filename>\n", argv[0]);
//        return EXIT_FAILURE;
//    }

    /*
     *  Initialise libSDL
     */
//    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTTHREAD) == -1){
//        printf("cannot initialize SDL\n");
//        return EXIT_FAILURE;
//    }

    motionDetector = new Motion();
    motionDetector->iniciarSurfaces(VIDEOWIDTH, VIDEOHEIGHT);


    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTTHREAD);

    empty = SDL_CreateRGBSurface(SDL_SWSURFACE, VIDEOWIDTH, VIDEOHEIGHT,
                                 32, 0, 0, 0, 0);
    ctx.surf = SDL_CreateRGBSurface(SDL_SWSURFACE, VIDEOWIDTH, VIDEOHEIGHT,
                                    16, 0x001f, 0x07e0, 0xf800, 0);

    ctx.mutex = SDL_CreateMutex();

    int options = SDL_ANYFORMAT | SDL_HWSURFACE | SDL_DOUBLEBUF;

    screen = SDL_SetVideoMode(WIDTH, HEIGHT, 0, options);
    if(!screen)
    {
        printf("cannot set video mode\n");
        return EXIT_FAILURE;
    }

    /*
     *  Initialise libVLC
     */
    libvlc = libvlc_new(vlc_argc, vlc_argv);
    m = libvlc_media_new_location(libvlc,"dshow://");
    libvlc_media_add_option(m," :dshow-vdev=Default");

    mp = libvlc_media_player_new_from_media(m);
    libvlc_media_release(m);

    libvlc_video_set_callbacks(mp, lock, unlock, display, &ctx);
    libvlc_video_set_format(mp, "RV16", VIDEOWIDTH, VIDEOHEIGHT, VIDEOWIDTH*2);
    libvlc_media_player_play(mp);



    /*
     *  Main loop
     */
    rect.w = 0;
    rect.h = 0;

    while(!done)
    {
        action = 0;

        /* Keys: enter (fullscreen), space (pause), escape (quit) */
        while( SDL_PollEvent( &event ) )
        {
            switch(event.type)
            {
            case SDL_QUIT:
                done = 1;
                break;
            case SDL_KEYDOWN:
                action = event.key.keysym.sym;
                break;
            }
        }

        switch(action)
        {
        case SDLK_ESCAPE:
            done = 1;
            break;
        case SDLK_RETURN:
            options ^= SDL_FULLSCREEN;
            screen = SDL_SetVideoMode(WIDTH, HEIGHT, 0, options);
            break;
        case ' ':
            pause = !pause;
            break;
        }

        //rect.x = (int)((1. + .5 * sin(0.03 * n)) * (WIDTH - VIDEOWIDTH) / 2);
        //rect.y = (int)((1. + .5 * cos(0.03 * n)) * (HEIGHT - VIDEOHEIGHT) / 2);
        rect.x = 0;
        rect.y = 0;

        //if(!pause)
         //   n++;

        /* Blitting the surface does not prevent it from being locked and
         * written to by another thread, so we use this additional mutex. */
        SDL_LockMutex(ctx.mutex);
        SDL_BlitSurface(ctx.surf, NULL, screen, &rect);
        SDL_UnlockMutex(ctx.mutex);

        SDL_Flip(screen);
        //SDL_Delay(10);

        SDL_BlitSurface(empty, NULL, screen, &rect);
    }

    /*
     * Stop stream and clean up libVLC
     */
    libvlc_media_player_stop(mp);
    libvlc_media_player_release(mp);
    libvlc_release(libvlc);

    /*
     * Close window and clean up libSDL
     */
    SDL_DestroyMutex(ctx.mutex);
    SDL_FreeSurface(ctx.surf);
    SDL_FreeSurface(empty);

    SDL_Quit();

    return 0;
}

