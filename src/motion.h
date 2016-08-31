#ifndef MOTION_H
#define MOTION_H

#include "SDL/SDL.h"
#include "ImagenGestor.h"
#include "image/uiimgencoder.h"

static const WORD red_mask_vlcsurface = 0x001F;
static const WORD green_mask_vlcsurface = 0x07E0;
static const WORD blue_mask_vlcsurface = 0xF800;

static const WORD red_mask_b = 0xF800;
static const WORD green_mask_b = 0x07E0;
static const WORD blue_mask_b = 0x001F;

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
        void iniciarPrueba(SDL_Surface *backgroundFrame, SDL_Surface *currentFrame);

        void iniciarSurfaces(int w, int h);

        void diferenceFilter();
        void diferenceFilter(SDL_Surface *varBackground, SDL_Surface *varCurrent);
        void erosionFilter();
        Uint32 showDiffFilter(SDL_Surface *finalImage);
        Uint32 showBlobsFilter(SDL_Surface *finalImage);
        Uint32 showBlobsFilter(SDL_Surface *finalImage, SDL_Surface *binaryImage);
        void backgroundSubtraction(SDL_Surface *varBackground, SDL_Surface *varCurrent);
        void showStepImage(SDL_Surface *finalImage);


        void setDifferenceThreshold(int var){
            differenceThreshold = var;
        }
        void setNoiseFilterSize(int var){
            noiseFilterSize = var;
        }

        Uint32 blobAnalysis(SDL_Surface *binaryImage, vector <tArrBlobPos> *v);


    protected:

    private:
        int differenceThreshold; //Diference in the substraction to consider a object
        int noiseFilterSize;     //number of diferences to consider an cuadratic area of size "noiseFilterSize" as an object
        int minimumBlobArea;     //Minimal area to consider as an object
        double factorBackground; //Factor expressed in % that makes to change the background

        Uint32 background;       //The color to represent nothing
        Uint32 foreground;       //The color to represent an object
        Uint32 difColour;

        Uint32 getGrayScale(Uint32 source_color);
        Uint32 getSafePixel(SDL_Surface *surface, const int x, const int y, Uint32 bckColor);
        void Line(SDL_Surface *surface, float x1, float y1, float x2, float y2, const t_color color);
        void drawRectLine(SDL_Surface *surface, float x0, float y0, float x1, float y1, const t_color color ,int lineWidth);


        SDL_Surface * stepsImage;
        //SDL_Surface * tmpImage;
        ImagenGestor imGestor;



};

#endif // MOTION_H
