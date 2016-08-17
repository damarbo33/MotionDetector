#include "motion.h"
#include "math.h"
#include "image/uiimgencoder.h"

Motion::Motion(){
    backgroundFrame = NULL;
    currentFrame = NULL;
    differenceThreshold = 600;
    noiseFilterSize = 11;
    step1Image = NULL;
    step2Image = NULL;
}

Motion::~Motion()
{
    //dtor
}

/**
*
*/
void Motion::iniciarPrueba(){
    ImagenGestor imGestor;

    imGestor.loadImgFromFile("ims\\actualFrame.jpg", &currentFrame);
    imGestor.loadImgFromFile("ims\\background.jpg", &backgroundFrame);

    if (currentFrame != NULL && backgroundFrame != NULL){
        cout << "Images Loaded" << endl;
        int bpp = currentFrame->format->BytesPerPixel;
        int w = currentFrame->w;
        int h = currentFrame->h;

        cout << "bpp: " << bpp << " w: " << w << " h: " << h << endl;


    } else {
        cout << "Error al cargar imagenes" << endl;
    }
}

/**
*
*/
void Motion::diferenceFilter(){
    int width = (int)backgroundFrame->w;
    int height = (int)backgroundFrame->h;
    ImagenGestor imGestor;
    Uint8 r, g, b;

    if (step1Image != NULL){
        SDL_FreeSurface(step1Image);
    }
    step1Image = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 24, rmask, gmask, bmask, amask);

    foreground = SDL_MapRGB(step1Image->format, cBlanco.r,cBlanco.g,cBlanco.b);
    background = SDL_MapRGB(step1Image->format, cNegro.r,cNegro.g,cNegro.b);

    Uint32 pixelBack, pixelcurrent;

    //first-pass: difference and threshold filter (mark the pixels that are changed between two frames)
    for (int x = 0; x < width; x++)
    {
         for (int y = 0; y < height; y++)
         {
             SDL_GetRGB(imGestor.getpixel(backgroundFrame, x, y), backgroundFrame->format, &r, &g, &b);
             pixelBack = (r+g+b)/3;
             SDL_GetRGB(imGestor.getpixel(currentFrame, x, y), currentFrame->format, &r, &g, &b);
             pixelcurrent = (r+g+b)/3;

             Uint32 diff = abs(pixelBack - pixelcurrent);
             imGestor.putpixel(step1Image, x, y, diff >= differenceThreshold ? foreground : background);
         }
    }

    UIImageEncoder imEncoder;
    imEncoder.IMG_SaveJPG("step1Image.jpg", step1Image, 80);
}

/**
*
*/
void Motion::erosionFilter(){
    ImagenGestor imGestor;

    if (step2Image != NULL){
        SDL_FreeSurface(step2Image);
    }

    int width = (int)step1Image->w;
    int height = (int)step1Image->h;
    step2Image = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 24, 0,0,0,0);

    foreground = SDL_MapRGB(step2Image->format, cBlanco.r,cBlanco.g,cBlanco.b);
    background = SDL_MapRGB(step2Image->format, cNegro.r,cNegro.g,cNegro.b);

    SDL_BlitSurface(step1Image, NULL, step2Image, NULL);

    //second-pass: erosion filter (remove noise i.e. ignore minor differences between two frames)
    int m = noiseFilterSize;
    int n = (m - 1) / 2; //'m' will be an odd number always

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            //count the number of marked pixels in current window
            int marked = 0;
            for (int i = x - n; i < x + n; i++)
                for (int j = y - n; j < y + n; j++)
                    //cout << i << "," << j << endl;
                    if (i < width && j < height && i >= 0 && j >= 0)
                    marked += imGestor.getpixel(step1Image, i, j) == foreground ? 1 : 0;

            if (marked >= m) //if atleast half the number of pixels are marked, then mark the full window
            {
                for (int i = x - n; i < x + n; i++)
                    for (int j = y - n; j < y + n; j++)
                        if (i < width && j < height && i >= 0 && j >= 0)
                        imGestor.putpixel(step2Image, i, j, foreground);
            }
        }
    }

    UIImageEncoder imEncoder;
    imEncoder.IMG_SaveJPG("step2Image.jpg", step2Image, 80);
}

