#ifndef MOTION_H
#define MOTION_H

#include "SDL/SDL.h"
#include "ImagenGestor.h"
#include "image/uiimgencoder.h"


static Uint32 background;
static Uint32 foreground;

class tArrBlobPos{
    public:

    tArrBlobPos(){
        minX = -1;
        minY = -1;
        maxX = -1;
        maxY = -1;
    }

    int minX, minY, maxX, maxY;
};

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
        Uint32 showDiffFilter(SDL_Surface *finalImage);
        Uint32 showBlobsFilter(SDL_Surface *finalImage);


        void setDifferenceThreshold(int var){
            differenceThreshold = var;
        }
        void setNoiseFilterSize(int var){
            noiseFilterSize = var;
        }

        Uint32 blobAnalysis(SDL_Surface *finalImage, SDL_Surface *binaryImage);


    protected:

    private:
        SDL_Surface *backgroundFrame;
        SDL_Surface *currentFrame;

        SDL_Surface *grayBackgroundFrame;
        SDL_Surface *grayCurrentFrame;
        int differenceThreshold, noiseFilterSize, minimumBlobArea;

        Uint32 getGrayScale(Uint32 source_color);
        Uint32 getSafePixel(SDL_Surface *surface, const int x, const int y, Uint32 bckColor);
        void Line(SDL_Surface *surface, float x1, float y1, float x2, float y2, const t_color color);
        void drawRectLine(SDL_Surface *surface, float x0, float y0, float x1, float y1, const t_color color ,int lineWidth);

        SDL_Surface * step1Image;
        SDL_Surface * step2Image;
        ImagenGestor imGestor;
};

#endif // MOTION_H
