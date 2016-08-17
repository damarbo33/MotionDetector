#include <iostream>
#include "motion.h"

using namespace std;

int main(int argc, char *argv[]){
    cout << "Hello world!" << endl;
    Motion *motionDetector = new Motion();

    try{
        motionDetector->iniciarPrueba();
        motionDetector->diferenceFilter();
        motionDetector->erosionFilter();

    } catch (Excepcion &e){
        cout << e.getMessage();
    }


    delete motionDetector;

    return 0;
}
