#include "motion.h"
#include "math.h"


Motion::Motion(){
    backgroundFrame = NULL;
    currentFrame = NULL;
    differenceThreshold = 15;
    noiseFilterSize = 3;
    step1Image = NULL;
    step2Image = NULL;


}

Motion::~Motion()
{
    //dtor
}

void Motion::iniciarSurfaces(int w, int h){

    if (step1Image != NULL){
        SDL_FreeSurface(step1Image);
    }
    step1Image = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 24, 0,0,0,0);

    foreground = SDL_MapRGB(step1Image->format, cBlanco.r,cBlanco.g,cBlanco.b);
    background = SDL_MapRGB(step1Image->format, cNegro.r,cNegro.g,cNegro.b);

    if (step2Image != NULL){
        SDL_FreeSurface(step2Image);
    }
    step2Image = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 24, 0,0,0,0);
}

/**
*
*/
void Motion::iniciarPrueba(){
    ImagenGestor imGestor;

    imGestor.loadImgFromFile("ims\\1.bmp", &currentFrame);
    imGestor.loadImgFromFile("ims\\2.bmp", &backgroundFrame);

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

void Motion::diferenceFilter(){
    diferenceFilter(currentFrame, backgroundFrame);
}

/**
*
*/
void Motion::diferenceFilter(SDL_Surface *varBackground, SDL_Surface *varCurrent){
    int width = (int)varBackground->w;
    int height = (int)varBackground->h;

    Uint8 r, g, b, a;
    Uint32 pixelBack, pixelcurrent;
//    SDL_Surface *grey1Image = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 24, rmask, gmask, bmask, amask);
//    SDL_Surface *grey2Image = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 24, rmask, gmask, bmask, amask);

    //first-pass: difference and threshold filter (mark the pixels that are changed between two frames)
    for (int x = 0; x < width; x++)
    {
         for (int y = 0; y < height; y++)
         {
             SDL_GetRGB(imGestor.getpixel(varBackground, x, y), varBackground->format, &r, &g, &b);
             pixelBack = (r+g+b)/3;
             SDL_GetRGB(imGestor.getpixel(varCurrent, x, y), varCurrent->format, &r, &g, &b);
             pixelcurrent = (r+g+b)/3;

//             imGestor.putpixel(grey1Image, x, y, getGrayScale(imGestor.getpixel(varBackground, x, y)));
//             imGestor.putpixel(grey2Image, x, y, getGrayScale(imGestor.getpixel(varCurrent, x, y)));

             int diff = abs(pixelBack - pixelcurrent);
             if (diff >= differenceThreshold){
                //cout << diff << endl;
                int x = 0;
             }
             imGestor.putpixel(step1Image, x, y, diff >= differenceThreshold ? foreground : background);
         }
    }

//    UIImageEncoder imEncoder;
//    imEncoder.IMG_SaveJPG("step1Image.jpg", step1Image, 95);
//    imEncoder.IMG_SaveJPG("grey1Image.jpg", grey1Image, 95);
//    imEncoder.IMG_SaveJPG("grey2Image.jpg", grey2Image, 95);
}

/**
*
*/
void Motion::erosionFilter(){
    int width = (int)step1Image->w;
    int height = (int)step1Image->h;
    SDL_FillRect(step2Image, NULL, background);
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

    //UIImageEncoder imEncoder;
    //imEncoder.IMG_SaveJPG("step2ImageV2.jpg", step2Image, 95);
}

/**
* Despues de aplicar diferenceFilter y erosionFilter, tenemos en step2Image
* las diferencias del movimiento de la imagen en el color blanco.
* Recorremos step2Image y modificamos el valor de los pixeles en la surface
* pasada por parametro
*/
void Motion::showDiffFilter(SDL_Surface *finalImage){
    int width = (int)step2Image->w;
    int height = (int)step2Image->h;

    foreground = SDL_MapRGB(step2Image->format, cBlanco.r,cBlanco.g,cBlanco.b);
    background = SDL_MapRGB(step2Image->format, cNegro.r,cNegro.g,cNegro.b);
    Uint32 moveShape = SDL_MapRGB(finalImage->format, cRojo.r,cRojo.g,cRojo.b);

    for (int x = 0; x < width; x++){
        for (int y = 0; y < height; y++){
            if (imGestor.getpixel(step2Image, x, y) == foreground){
                imGestor.putpixel(finalImage, x, y, moveShape);
            }
        }
    }
}

Uint32 Motion::getGrayScale(Uint32 source_color){

    const Uint8 component1 = source_color & 0xff; // red or blue
    const Uint8 component2 = (source_color >> 8) & 0xff; // green
    const Uint8 component3 = (source_color >> 16) & 0xff; // blue or red

    // calculating the average component/color (i.e. grayscale)
    const Uint32 gray = (component1 + component2 + component3) / 3;

    // back to SDL's RGB color (optional)
    const Uint32 gray_rgb = 0xff000000 | (gray << 16) | (gray << 8) | gray;

    /*
    // There are multiple ways to get the monochrom image, based on the format you'd like (e.g. just a boolean value or just a RGB value).
    // First version (actually not recommended, but might save you code e.g. having one fixed conversion function):
    // calculating the monochromatic color with a given threshold (127)
    const unsigned int mono = gray > 127 ? 255 : 0;
    // once again back to SDL's RGB model (optional)
    const int mono_rgb = 0xff000000 | (mono << 16) | (mono << 8) | mono;
    */
    // Second version:
    // just return the final color in one step
    // this returns pure white or pure black
    //const int mono_rgb = gray > 127 ? 0xffffffff : 0xff000000;
    return gray_rgb;
}
