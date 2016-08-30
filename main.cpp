#include <iostream>
#include "motion.h"


using namespace std;

//int main(int argc, char *argv[]){
//    cout << "Hello world!" << endl;
//    Motion *motionDetector = new Motion();
//
//    try{
//        SDL_Surface *shapes;
//        SDL_Surface *procesedShapes;
//
//        ImagenGestor imGestor;
//        imGestor.loadImgFromFile("step2invImage.jpg", &shapes);
//        cout << "BitsPerPixel " << (int)shapes->format->BitsPerPixel << endl;
//
//        procesedShapes = SDL_CreateRGBSurface(SDL_SWSURFACE, shapes->w, shapes->h, 24, 0,0,0,0);
//        SDL_BlitSurface(shapes, NULL, procesedShapes, NULL);
//        vector <tArrBlobPos> v;
//        cout << "Detected objects: " << motionDetector->showBlobsFilter(procesedShapes, shapes) << endl;
//
//        UIImageEncoder imEncoder;
//        imEncoder.IMG_SaveJPG("Shapes.jpg", procesedShapes, 95);
//
//    } catch (Excepcion &e){
//        cout << e.getMessage();
//    }
//    delete motionDetector;
//
//    return 0;
//}

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

#define VIDEOWIDTH WIDTH
#define VIDEOHEIGHT HEIGHT



struct ctx
{
    SDL_Surface *surf;
    SDL_Surface *bckSurf;
    SDL_mutex *mutex;
};

static Uint32 nFrames = 0;
static Motion *motionDetector;
static UIImageEncoder imEncoder;
static ImagenGestor imGestor;
static bool saveBackground = true;
bool okSavedBackground = false;

static void *lock(void *data, void **p_pixels)
{
    struct ctx *cotx = (ctx *)data;

    SDL_LockMutex(cotx->mutex);
    SDL_LockSurface(cotx->surf);
    SDL_LockSurface(cotx->bckSurf);
    *p_pixels = cotx->surf->pixels;
    return NULL; /* picture identifier, not needed here */
}

static void unlock(void *data, void *id, void *const *p_pixels)
{
    //unsigned long init = SDL_GetTicks();

    struct ctx *cotx = (ctx *)data;

    /* VLC just rendered the video, but we can also render stuff */
    uint16_t *pixels = (uint16_t *)*p_pixels;
//    int x, y;
//
//    for(y = 10; y < 40; y++)
//        for(x = 10; x < 40; x++)
//            if(x < 13 || y < 13 || x > 36 || y > 36)
//                pixels[y * VIDEOWIDTH + x] = 0xffff;
//            else
//                pixels[y * VIDEOWIDTH + x] = 0x0;
//

    if (nFrames > 0 && saveBackground == true){
        uint16_t *dstPixels = (uint16_t *)cotx->bckSurf->pixels;
        int i=0;
        while (i < cotx->surf->h * cotx->surf->w){
            dstPixels[i] = pixels[i]; //Solo si la surface es de 16 bits
            i++;
        }
        saveBackground = false;
    }

    if (nFrames > 0 && cotx->bckSurf != NULL && cotx->surf != NULL){

        //Doing the diference of two images
        motionDetector->diferenceFilter(cotx->bckSurf, cotx->surf);
        //Joining areas with a minimal number of changes
        motionDetector->erosionFilter();
        //Updating the background dinamically every 10 frames
        if (nFrames % 10 == 0){
            motionDetector->backgroundSubtraction(cotx->bckSurf, cotx->surf);
        }
        //Showing the diferences in the screen
        motionDetector->showDiffFilter(cotx->surf);
        //Showing the objects detected in the screen
        motionDetector->showBlobsFilter(cotx->surf);
    }

    nFrames++;

    SDL_UnlockSurface(cotx->surf);
    SDL_UnlockSurface(cotx->bckSurf);
    SDL_UnlockMutex(cotx->mutex);

    assert(id == NULL); /* picture identifier, not needed here */

    //cout << "time: " << SDL_GetTicks() - init << " ms" << endl;
}

static void display(void *data, void *id)
{
    /* VLC wants to display the video */
    (void) data;
    assert(id == NULL);
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTTHREAD | SDL_INIT_TIMER);

    libvlc_instance_t *libvlc;
    libvlc_media_t *m;
    libvlc_media_player_t *mp;

    string videoRes = "--dshow-size=" + Constant::TipoToStr(VIDEOWIDTH) + "x" + Constant::TipoToStr(VIDEOHEIGHT);

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
        //,"-vvv" /* be much more verbose for debugging */
        ,"--no-osd"
        ,"--dshow-aspect-ratio=16:9"
        ,"--dshow-fps=30"
        ,videoRes.c_str()
        ,"--live-caching=0"
    };
    int vlc_argc = sizeof(vlc_argv) / sizeof(*vlc_argv);
    SDL_Surface *screen;
    SDL_Event event;
    SDL_Rect rect;
    int done = 0, action = 0, pause = 0;
    struct ctx ctx;

    motionDetector = new Motion();
    motionDetector->iniciarSurfaces(VIDEOWIDTH, VIDEOHEIGHT);

    //Creating 16bit RGB565 Surfaces
    ctx.surf    = SDL_CreateRGBSurface(SDL_SWSURFACE, VIDEOWIDTH, VIDEOHEIGHT, 16, 0x001f, 0x07e0, 0xf800, 0);
    ctx.bckSurf = SDL_CreateRGBSurface(SDL_SWSURFACE, VIDEOWIDTH, VIDEOHEIGHT, 16, 0x001f, 0x07e0, 0xf800, 0);

    ctx.mutex = SDL_CreateMutex();
    int options = SDL_ANYFORMAT | SDL_SWSURFACE;

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
    unsigned long inicio = SDL_GetTicks();

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
        case SDLK_s:
            saveBackground = true;
            break;
        }

        rect.x = 0;
        rect.y = 0;

        /* Blitting the surface does not prevent it from being locked and
         * written to by another thread, so we use this additional mutex. */
        SDL_LockMutex(ctx.mutex);
        SDL_BlitSurface(ctx.surf, NULL, screen, &rect);
        //SDL_BlitSurface(ctx.bckSurf, NULL, screen, &rect);
        SDL_UnlockMutex(ctx.mutex);

        if (okSavedBackground)
            SDL_Flip(screen);

        if (SDL_GetTicks() > inicio + 3000 && okSavedBackground == false){
            saveBackground = true;
            okSavedBackground = true;
        }
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
    SDL_FreeSurface(ctx.bckSurf);

    SDL_Quit();

    return 0;
}

