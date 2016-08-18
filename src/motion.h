#ifndef MOTION_H
#define MOTION_H

#include "SDL/SDL.h"
#include "ImagenGestor.h"
#include "image/uiimgencoder.h"


static Uint32 background;
static Uint32 foreground;

class Motion
{
    public:
        /** Default constructor */
        Motion();
        /** Default destructor */
        virtual ~Motion();
        void iniciarPrueba();

        void iniciarSurfaces(int w, int h);

        void diferenceFilter();
        void diferenceFilter(SDL_Surface *varBackground, SDL_Surface *varCurrent);
        void erosionFilter();
        void showDiffFilter(SDL_Surface *finalImage);


        void setDifferenceThreshold(int var){
            differenceThreshold = var;
        }
        void setNoiseFilterSize(int var){
            noiseFilterSize = var;
        }



    protected:

    private:
        SDL_Surface *backgroundFrame;
        SDL_Surface *currentFrame;

        SDL_Surface *grayBackgroundFrame;
        SDL_Surface *grayCurrentFrame;
        int differenceThreshold, noiseFilterSize;

        Uint32 getGrayScale(Uint32 source_color);


        SDL_Surface * step1Image;
        SDL_Surface * step2Image;
        ImagenGestor imGestor;
};

#endif // MOTION_H
