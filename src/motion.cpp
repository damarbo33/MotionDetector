#include "motion.h"
#include "math.h"

static bool debug = false;

Motion::Motion(){
    differenceThreshold = 80;
    noiseFilterSize = 19;
    minimumBlobArea = 20;
    factorBackground = 0.80;
    stepsImage = NULL;
}

Motion::~Motion(){
    //dtor
}

void Motion::iniciarSurfaces(int w, int h){

    if (stepsImage != NULL){
        SDL_FreeSurface(stepsImage);
    }

    stepsImage = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 16, red_mask_vlcsurface,
                                      green_mask_vlcsurface, blue_mask_vlcsurface,0);

    foreground = SDL_MapRGB(stepsImage->format, cBlanco.r,cBlanco.g,cBlanco.b);
    background = SDL_MapRGB(stepsImage->format, cNegro.r,cNegro.g,cNegro.b);
    difColour = SDL_MapRGB(stepsImage->format, cRojo.r,cRojo.g,cRojo.b);

//    if (tmpImage != NULL){
//        SDL_FreeSurface(tmpImage);
//    }
//    tmpImage = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 16, 0,0,0,0);
}

/**
*
*/
void Motion::iniciarPrueba(SDL_Surface *backgroundFrame, SDL_Surface *currentFrame){
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



/**
* http://codeding.com/articles/motion-detection-algorithm
*/
void Motion::diferenceFilter(SDL_Surface *varBackground, SDL_Surface *varCurrent){
    if (debug) cout << "diferenceFilter" << endl;
    int width = (int)varBackground->w;
    int height = (int)varBackground->h;

    Uint8 r = 0, g = 0, b = 0;
//    Uint8 r2 = 0, g2 = 0, b2 = 0;
//    Uint8 r3 = 0, g3 = 0, b3 = 0;
    Uint8 pixBack = 0, pixcurrent = 0;
    uint16_t *dstPixels = (uint16_t *)stepsImage->pixels;
    uint16_t *backPixels = (uint16_t *)varBackground->pixels;
    uint16_t *currPixels = (uint16_t *)varCurrent->pixels;

    //first-pass: difference and threshold filter (mark the pixels that are changed between two frames)
    int i = 0;
    uint16_t diff = 0;
    while (i < width*height){
        r = ((backPixels[i] & red_mask_b) >> 11) << 3;
        g = ((backPixels[i] & green_mask_b) >> 5) << 2;
        b = (backPixels[i] & blue_mask_b) << 3;

//        SDL_GetRGB(backPixels[i], varBackground->format, &r,&g,&b);
        //Obtenemos la media de los 3 valores para obtener un pixel gris
        //pixBack = (r+g+b)/3;
         pixBack = (0.3 * r) + (0.59 * g) + (0.11 * b);

        //El desplazamiento a derechas 11,5 es para obtener los valores
        //en formato rgb565 directamente del pixel en formato 16bits.
        //El desplazamiento a izquierda 3,2,3 es para convertir al formato
        //rgb de 0 bits por color
        r = ((currPixels[i] & red_mask_b) >> 11) << 3;
        g = ((currPixels[i] & green_mask_b) >> 5) << 2;
        b = (currPixels[i] & blue_mask_b) << 3;

//        SDL_GetRGB(currPixels[i], varCurrent->format, &r3,&g3,&b3);
        //Obtenemos la media de los 3 valores para obtener un pixel gris
        //pixcurrent = (r+g+b)/3;
        pixcurrent = (0.3 * r) + (0.59 * g) + (0.11 * b);

        diff = pixBack > pixcurrent ? pixBack - pixcurrent : pixcurrent - pixBack;
		//Creating a binary image with a threshold
        dstPixels[i] = diff >= differenceThreshold ? foreground : background;
        /*
        //Para obtener una imagen en gris, primero devolvemos al formato rgb565
        r = pixcurrent >> 3;
        g = pixcurrent >> 2;
        b = pixcurrent >> 3;
        //Y desplazamos los bits correspondiendo al formato 16bits RGB565
        dstPixels[i] = (r << 11) | (g << 5) | b;
        */
        i++;
    }
//    UIImageEncoder imEncoder;
//    imEncoder.IMG_SaveJPG("stepsImage.jpg", stepsImage, 95);
}

/**
* http://codeding.com/articles/motion-detection-algorithm
*/
void Motion::erosionFilter(){
    if (debug) cout << "erosionFilter" << endl;
    int width = (int)stepsImage->w;
    int height = (int)stepsImage->h;
    //SDL_FillRect(tmpImage, NULL, background);
    //SDL_BlitSurface(stepsImage, NULL, tmpImage, NULL);

    uint16_t *dstPixels = (uint16_t *)stepsImage->pixels;
    //uint16_t *srcPixels = (uint16_t *)tmpImage->pixels;

    //second-pass: erosion filter (remove noise i.e. ignore minor differences between two frames)
    int m = noiseFilterSize;
    int n = (m - 1) / 2; //'m' will be an odd number always

    for (int x = 0; x < width; x+=m-1)
    {
        for (int y = 0; y < height; y+=m-1)
        {
            //count the number of marked pixels in current window
            int marked = 0;
            for (int i = x - n; i < x + n && marked < n; i++)
                for (int j = y - n; j < y + n && marked < n; j++)
                    if (i < width && j < height && i >= 0 && j >= 0)
                    marked += dstPixels[i+j*width] == foreground ? 1 : 0;

            //if atleast half the number of pixels are marked, then mark the full window
            if (marked >= n)
            {
                for (int i = x - n; i < x + n; i++)
                    for (int j = y - n; j < y + n; j++)
                        if (i < width && j < height && i >= 0 && j >= 0)
                            dstPixels[i+j*width] = foreground;
            }
        }
    }

    //UIImageEncoder imEncoder;
    //imEncoder.IMG_SaveJPG("tmpImageV2.jpg", tmpImage, 95);
}

/**
*
* UPDATING THE BACKGROUND DINAMICALLY
* http://what-when-how.com/introduction-to-video-and-image-processing/segmentation-in-video-data-introduction-to-video-and-image-processing-part-2/
*/
void Motion::backgroundSubtraction(SDL_Surface *varBackground, SDL_Surface *varCurrent){
    if (debug) cout << "backgroundSubtraction" << endl;
    uint8_t r=0,g=0,b=0;
    uint8_t r1=0,g1=0,b1=0;
    uint8_t r2=0,g2=0,b2=0;

    uint16_t *dstPixels = (uint16_t *)varBackground->pixels;
    uint16_t *pixels = (uint16_t *)varCurrent->pixels;

    int i=0;
    while (i < varCurrent->h * varCurrent->w){
//        SDL_GetRGB(dstPixels[i], varBackground->format, &r1,&g1,&b1);
//        SDL_GetRGB(pixels[i], varCurrent->format, &r2,&g2,&b2);

        r1 = ((dstPixels[i] & red_mask_b) >> 11) << 3;
        g1 = ((dstPixels[i] & green_mask_b) >> 5) << 2;
        b1 = (dstPixels[i] & blue_mask_b) << 3;

        r2 = ((pixels[i] & red_mask_b) >> 11) << 3;
        g2 = ((pixels[i] & green_mask_b) >> 5) << 2;
        b2 = (pixels[i] & blue_mask_b) << 3;

        r = ((r1 * factorBackground) + (double)(1.0 - factorBackground) * r2);
        g = ((g1 * factorBackground) + (double)(1.0 - factorBackground) * g2);
        b = ((b1 * factorBackground) + (double)(1.0 - factorBackground) * b2);

//        dstPixels[i] = SDL_MapRGB(varBackground->format, r,g,b);
        dstPixels[i] = ((r >> 3) << 11) | ((g >> 2) << 5) | b >> 3;

        i++;
    }
}

/**
* Despues de aplicar diferenceFilter y erosionFilter, tenemos en stepsImage
* las diferencias del movimiento de la imagen en el color blanco.
* Recorremos stepsImage y modificamos el valor de los pixeles en la surface
* pasada por parametro
*/
Uint32 Motion::showDiffFilter(SDL_Surface *finalImage){
    if (debug) cout << "showDiffFilter" << endl;
    int width = (int)stepsImage->w;
    int height = (int)stepsImage->h;

    Uint32 diferences = 0;

    uint16_t *dstPixels = (uint16_t *)finalImage->pixels;
    uint16_t *srcPixels = (uint16_t *)stepsImage->pixels;

    int i = 0;
    while (i < width*height){
        if (srcPixels[i] == foreground){
            dstPixels[i] = difColour;
            diferences++;
        }
        i++;
    }
    return diferences;
}

/**
*
*/
void Motion::showStepImage(SDL_Surface *finalImage){
    if (debug) cout << "showStepImage" << endl;
    int width = (int)stepsImage->w;
    int height = (int)stepsImage->h;

    uint16_t *dstPixels = (uint16_t *)finalImage->pixels;
    uint16_t *srcPixels = (uint16_t *)stepsImage->pixels;

    int i = 0;
    while (i < width*height){
        dstPixels[i] = srcPixels[i];
        i++;
    }
}

Uint32 Motion::showBlobsFilter(SDL_Surface *finalImage){
    return showBlobsFilter(finalImage, stepsImage);
}



/**
* Despues de aplicar diferenceFilter y erosionFilter, tenemos en stepsImage
* las diferencias del movimiento de la imagen en el color blanco.
* Recorremos stepsImage y modificamos el valor de los pixeles en la surface
* pasada por parametro
*/
Uint32 Motion::showBlobsFilter(SDL_Surface *finalImage, SDL_Surface *binaryImage){
    if (debug) cout << "showBlobsFilter" << endl;
    vector <tArrBlobPos> v;
    Uint32 nObjs = blobAnalysis(finalImage, binaryImage, &v);
    if (finalImage != NULL){
    t_color color;

        for (Uint32 i=0; i < nObjs; i++){

//            color.r = v.at(i).meanR;
//            color.g = v.at(i).meanG;
//            color.b = v.at(i).meanB;

            drawRectLine(finalImage,v.at(i).minX, v.at(i).minY,
            v.at(i).maxX,v.at(i).maxY, cVerde,1);

//            cout << "blob" << i << " r " << v.at(i).meanR
//            << " g " << v.at(i).meanG
//            << " b " << v.at(i).meanB
//            << endl;

            //drawText(finalImage, "aliron", v.at(i).minX, v.at(i).minY, cVerde);
        }
    }
    return nObjs;
}

/**
* Detecta el numero de objetos en una imagen mediante "The Sequential Grass-Fire Algorithm"
* http://what-when-how.com/introduction-to-video-and-image-processing/blob-analysis-introduction-to-video-and-image-processing-part-1/
*
*/
Uint32 Motion::blobAnalysis(SDL_Surface *finalImage, SDL_Surface *binaryImage, vector <tArrBlobPos> *v){
    int detectedObj = 0;
    if (debug) cout << "blobAnalysis" << endl;

    Uint8 r,g,b;
    if (binaryImage != NULL){
        int width = (int)binaryImage->w;
        int height = (int)binaryImage->h;
        //En arrBlob almacenamos el numero de objeto que le corresponde a cada pixel
        Uint16 arrBlob[width][height];
        memset(arrBlob, 0, width*height*sizeof(Uint16));

        vector<int> lista;
        //foreground = SDL_MapRGB(binaryImage->format, cBlanco.r,cBlanco.g,cBlanco.b);
        //background = SDL_MapRGB(binaryImage->format, cNegro.r,cNegro.g,cNegro.b);
        int tmpX = 0;
        int tmpY = 0;
        int valXlist = 0;
        int valYlist = 0;
        int nObj = 0;
        bool found = false;

    //    Uint32 moveShape[8];
    //    moveShape[0] = SDL_MapRGB(finalImage->format, cRojo.r,cRojo.g,cRojo.b);
    //    moveShape[1] = SDL_MapRGB(finalImage->format, cVerde.r,cVerde.g,cVerde.b);
    //    moveShape[2] = SDL_MapRGB(finalImage->format, cAmarillo.r,cAmarillo.g,cAmarillo.b);
    //    moveShape[3] = SDL_MapRGB(finalImage->format, cAzulTotal.r,cAzulTotal.g,cAzulTotal.b);
    //    moveShape[4] = SDL_MapRGB(finalImage->format, cNaranja.r,cNaranja.g,cNaranja.b);
    //    moveShape[5] = SDL_MapRGB(finalImage->format, cTurquesa.r,cTurquesa.g,cTurquesa.b);
    //    moveShape[6] = SDL_MapRGB(finalImage->format, cGrisClaro.r,cGrisClaro.g,cGrisClaro.b);
    //    moveShape[7] = SDL_MapRGB(finalImage->format, cAzulOscuro.r,cAzulOscuro.g,cAzulOscuro.b);
        int maxShapes = pow(2, sizeof(arrBlob) / sizeof(arrBlob[0]));

        //uint16_t *dstPixels = (uint16_t *)finalImage->pixels;
        uint16_t *srcPixels = (uint16_t *)binaryImage->pixels;

        int i = 0;
        int x = 0, y = 0;
        while (i < width*height && nObj < maxShapes-1){
            x = i % width;
            y = i / width;

            if (srcPixels[i] == foreground){
                nObj++;
                arrBlob[x][y] = nObj;
                srcPixels[i] = background;
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
            i++;
        }
//        }

        tArrBlobPos arrayBlobPos[nObj];
        uint16_t *dstPixels = (uint16_t *)finalImage->pixels;
        //Buscamos los limites de cada campo detectado
        for (int y = 0; y < height; y++){
            for (int x = 0; x < width; x++){
                if (arrBlob[x][y] >= 1){

                    r = ((dstPixels[y*width+x] & red_mask_b) >> 11) << 3;
                    g = ((dstPixels[y*width+x] & green_mask_b) >> 5) << 2;
                    b = (dstPixels[y*width+x] & blue_mask_b) << 3;

                    arrayBlobPos[arrBlob[x][y]-1].meanR += r;
                    arrayBlobPos[arrBlob[x][y]-1].meanG += g;
                    arrayBlobPos[arrBlob[x][y]-1].meanB += b;
    //                if (arrBlob[x][y]-1 < 7)
    //                imGestor.putpixel(finalImage, x, y, moveShape[arrBlob[x][y]-1]);
                    if (arrayBlobPos[arrBlob[x][y]-1].maxX == -1 || x > arrayBlobPos[arrBlob[x][y]-1].maxX ){
                        arrayBlobPos[arrBlob[x][y]-1].maxX = x;
                    }
                    if (arrayBlobPos[arrBlob[x][y]-1].minX == -1 || x < arrayBlobPos[arrBlob[x][y]-1].minX ){
                        arrayBlobPos[arrBlob[x][y]-1].minX = x;
                    }
                    if (arrayBlobPos[arrBlob[x][y]-1].maxY == -1 || y > arrayBlobPos[arrBlob[x][y]-1].maxY ){
                        arrayBlobPos[arrBlob[x][y]-1].maxY = y;
                    }
                    if (arrayBlobPos[arrBlob[x][y]-1].minY == -1 || y < arrayBlobPos[arrBlob[x][y]-1].minY ){
                        arrayBlobPos[arrBlob[x][y]-1].minY = y;
                    }
                }
            }
        }
        //Pintamos rectangulos para encuadrar cada objeto
        Uint32 nPixelsBlob = 0;
        for (int i=0; i < nObj; i++){
            if (arrayBlobPos[i].minX >= 0 && arrayBlobPos[i].minY >= 0 &&
                arrayBlobPos[i].maxX >= 0 && arrayBlobPos[i].maxY >= 0 &&
                (abs(arrayBlobPos[i].minX - arrayBlobPos[i].maxX)
                  * abs(arrayBlobPos[i].minY - arrayBlobPos[i].maxY)) > minimumBlobArea){



                    nPixelsBlob = (arrayBlobPos[i].maxX - arrayBlobPos[i].minX + 1)
                                * (arrayBlobPos[i].maxY - arrayBlobPos[i].minY + 1);

                    arrayBlobPos[i].meanR = nPixelsBlob > 0 ? arrayBlobPos[i].meanR / nPixelsBlob : 0;
                    arrayBlobPos[i].meanG = nPixelsBlob > 0 ? arrayBlobPos[i].meanG / nPixelsBlob : 0;
                    arrayBlobPos[i].meanB = nPixelsBlob > 0 ? arrayBlobPos[i].meanB / nPixelsBlob : 0;

                    //if (arrayBlobPos[i].meanG < 120){
                        detectedObj++;
                        v->push_back(arrayBlobPos[i]);
                    //}

    //                drawRectLine(finalImage,arrayBlobPos[i].minX, arrayBlobPos[i].minY,
    //                    arrayBlobPos[i].maxX,arrayBlobPos[i].maxY, cRojo,1);
                }
        }
    }
    return detectedObj;
}

void Motion::drawRectLine(SDL_Surface *surface, float x0, float y0, float x1, float y1, const t_color color ,int lineWidth){
    if (y0+lineWidth < surface->w)
        for (int i=0;i<lineWidth;i++){
            Line(surface, x0,y0+i, x1, y0+i, color); //Superior
            Line(surface, x0,y1-i, x1, y1-i, color); //Inferior
            Line(surface, x0+i,y0, x0+i, y1, color); //Izquierda
            Line(surface, x1-i,y0, x1-i, y1, color);//Derecha
        }
}

void Motion::Line(SDL_Surface *surface, float x1, float y1, float x2, float y2, const t_color color ){
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

