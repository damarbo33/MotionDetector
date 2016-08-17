#ifndef MOTION_H
#define MOTION_H

#include "SDL/SDL.h"
#include "ImagenGestor.h"


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
        void diferenceFilter();
        void erosionFilter();


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



        SDL_Surface * step1Image;
        SDL_Surface * step2Image;
};

#endif // MOTION_H
