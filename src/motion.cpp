#include "motion.h"
#include "math.h"


Motion::Motion(){
    backgroundFrame = NULL;
    currentFrame = NULL;
    differenceThreshold = 15;
    noiseFilterSize = 3;
    minimumBlobArea = 150;
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
Uint32 Motion::showDiffFilter(SDL_Surface *finalImage){
    int width = (int)step2Image->w;
    int height = (int)step2Image->h;

    foreground = SDL_MapRGB(step2Image->format, cBlanco.r,cBlanco.g,cBlanco.b);
    background = SDL_MapRGB(step2Image->format, cNegro.r,cNegro.g,cNegro.b);
    Uint32 moveShape = SDL_MapRGB(finalImage->format, cRojo.r,cRojo.g,cRojo.b);
    Uint32 diferences = 0;
    for (int x = 0; x < width; x++){
        for (int y = 0; y < height; y++){
            if (imGestor.getpixel(step2Image, x, y) == foreground){
                imGestor.putpixel(finalImage, x, y, moveShape);
                diferences++;
            }
        }
    }
    return diferences;
}

Uint32 Motion::showBlobsFilter(SDL_Surface *finalImage){
    return blobAnalysis(finalImage, step2Image);
}

/**
* Detecta el numero de objetos en una imagen
* http://what-when-how.com/introduction-to-video-and-image-processing/blob-analysis-introduction-to-video-and-image-processing-part-1/
*
*/
Uint32 Motion::blobAnalysis(SDL_Surface *finalImage, SDL_Surface *binaryImage){
    int width = (int)binaryImage->w;
    int height = (int)binaryImage->h;
    int arrBlob[width][height];
    memset(arrBlob, 0, width*height*sizeof(int));
    vector<int> lista;
    foreground = SDL_MapRGB(binaryImage->format, cBlanco.r,cBlanco.g,cBlanco.b);
    background = SDL_MapRGB(binaryImage->format, cNegro.r,cNegro.g,cNegro.b);
    int tmpX = 0;
    int tmpY = 0;
    int valXlist = 0;
    int valYlist = 0;
    int posLista = 0;
    int nObj = 0;
    bool found = false;

//    Uint32 moveShape[8];
//
//    moveShape[0] = SDL_MapRGB(finalImage->format, cRojo.r,cRojo.g,cRojo.b);
//    moveShape[1] = SDL_MapRGB(finalImage->format, cVerde.r,cVerde.g,cVerde.b);
//    moveShape[2] = SDL_MapRGB(finalImage->format, cAmarillo.r,cAmarillo.g,cAmarillo.b);
//    moveShape[3] = SDL_MapRGB(finalImage->format, cAzulTotal.r,cAzulTotal.g,cAzulTotal.b);
//    moveShape[4] = SDL_MapRGB(finalImage->format, cNaranja.r,cNaranja.g,cNaranja.b);
//    moveShape[5] = SDL_MapRGB(finalImage->format, cTurquesa.r,cTurquesa.g,cTurquesa.b);
//    moveShape[6] = SDL_MapRGB(finalImage->format, cGrisClaro.r,cGrisClaro.g,cGrisClaro.b);
//    moveShape[7] = SDL_MapRGB(finalImage->format, cAzulOscuro.r,cAzulOscuro.g,cAzulOscuro.b);



    for (int y = 0; y < height; y++){
        for (int x = 0; x < width; x++){
            if (getSafePixel(binaryImage, x, y, background) == foreground){
                nObj++;
                arrBlob[x][y] = nObj;
                imGestor.putpixel(binaryImage, x, y, background);
                found = true;
            } else {
                found = false;
            }

            if (found){
                do{
                    if (lista.size() == 0){
                        valXlist = x;
                        valYlist = y;
                    } else {
                        valXlist = lista.at(0);
                        valYlist = lista.at(1);
                        // erase the first 2 elements:
                        lista.erase (lista.begin(), lista.begin()+2);
                    }

                    tmpX = valXlist+1;
                    tmpY = valYlist;
                    if (getSafePixel(binaryImage, tmpX, tmpY, background) == foreground){
                        lista.push_back(tmpX);
                        lista.push_back(tmpY);
                        arrBlob[tmpX][tmpY] = nObj;
                        imGestor.putpixel(binaryImage, tmpX, tmpY, background);
                    }

                    tmpX = valXlist-1;
                    tmpY = valYlist;
                    if (getSafePixel(binaryImage, tmpX, tmpY, background) == foreground){
                        lista.push_back(tmpX);
                        lista.push_back(tmpY);
                        arrBlob[tmpX][tmpY] = nObj;
                        imGestor.putpixel(binaryImage, tmpX, tmpY, background);
                    }

                    tmpX = valXlist;
                    tmpY = valYlist+1;
                    if (getSafePixel(binaryImage, tmpX, tmpY, background) == foreground){
                        lista.push_back(tmpX);
                        lista.push_back(tmpY);
                        arrBlob[tmpX][tmpY] = nObj;
                        imGestor.putpixel(binaryImage, tmpX, tmpY, background);
                    }

                    tmpX = valXlist;
                    tmpY = valYlist-1;
                    if (getSafePixel(binaryImage, tmpX, tmpY, background) == foreground){
                        lista.push_back(tmpX);
                        lista.push_back(tmpY);
                        arrBlob[tmpX][tmpY] = nObj;
                        imGestor.putpixel(binaryImage, tmpX, tmpY, background);
                    }

                } while (lista.size() > 0);
            }
        }
    }

    tArrBlobPos arrayBlobPos[nObj];

    //Buscamos los limites de cada campo detectado
    for (int y = 0; y < height; y++){
        for (int x = 0; x < width; x++){
            if (arrBlob[x][y] >= 1){
//                if (arrBlob[x][y]-1 < 7)
//                imGestor.putpixel(finalImage, x, y, moveShape[arrBlob[x][y]-1]);
                if (arrayBlobPos[arrBlob[x][y]-1].maxX == -1 || x > arrayBlobPos[arrBlob[x][y]-1].maxX ){
                    arrayBlobPos[arrBlob[x][y]-1].maxX = x;
                } else if (arrayBlobPos[arrBlob[x][y]-1].maxY == -1 || y > arrayBlobPos[arrBlob[x][y]-1].maxY ){
                    arrayBlobPos[arrBlob[x][y]-1].maxY = y;
                } else if (arrayBlobPos[arrBlob[x][y]-1].minX == -1 || x < arrayBlobPos[arrBlob[x][y]-1].minX ){
                    arrayBlobPos[arrBlob[x][y]-1].minX = x;
                } else if (arrayBlobPos[arrBlob[x][y]-1].minY == -1 || y < arrayBlobPos[arrBlob[x][y]-1].minY ){
                    arrayBlobPos[arrBlob[x][y]-1].minY = y;
                }
            }
        }
    }
    int detectedObj = 0;
    //Pintamos rectangulos para encuadrar cada objeto
    for (int i=0; i < nObj; i++){
        if (arrayBlobPos[i].minX >= 0 && arrayBlobPos[i].minY >= 0 &&
            arrayBlobPos[i].maxX >= 0 && arrayBlobPos[i].maxY >= 0){
                detectedObj++;
                drawRectLine(finalImage,arrayBlobPos[i].minX, arrayBlobPos[i].minY,
                    arrayBlobPos[i].maxX,arrayBlobPos[i].maxY, cRojo,1);
            }
    }

    return detectedObj;
}

void Motion::drawRectLine(SDL_Surface *surface, float x0, float y0, float x1, float y1, const t_color color ,int lineWidth)
{
//    if (y0+lineWidth < this->w)
        for (int i=0;i<lineWidth;i++){
            Line(surface, x0,y0+i, x1, y0+i, color); //Superior
            Line(surface, x0,y1-i, x1, y1-i, color); //Inferior
            Line(surface, x0+i,y0, x0+i, y1, color); //Izquierda
            Line(surface, x1-i,y0, x1-i, y1, color);//Derecha
        }
}

void Motion::Line(SDL_Surface *surface, float x1, float y1, float x2, float y2, const t_color color )
{
    // Bresenham's line algorithm
    const bool steep = (fabs(y2 - y1) > fabs(x2 - x1));
    if(steep)
    {
        std::swap(x1, y1);
        std::swap(x2, y2);
    }

    if(x1 > x2)
    {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }

    const float dx = x2 - x1;
    const float dy = fabs(y2 - y1);

    float error = dx / 2.0f;
    const int ystep = (y1 < y2) ? 1 : -1;
    int y = (int)y1;

    const int maxX = (int)x2;

    Uint32 colorLine = SDL_MapRGB(surface->format, color.r,color.g,color.b);

    for(int x=(int)x1; x<maxX; x++)
    {
        if(steep)
        {
            imGestor.putpixel(surface, y,x, colorLine);
        }
        else
        {
            imGestor.putpixel(surface, x,y, colorLine);
        }

        error -= dy;
        if(error < 0)
        {
            y += ystep;
            error += dx;
        }
    }
}

Uint32 Motion::getSafePixel(SDL_Surface *surface, const int x, const int y, Uint32 bckColor){

    if (x >= 0 && y >= 0 && x < surface->w && y < surface->h){
        return imGestor.getpixel(surface, x, y);
    } else {
        return bckColor;
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
