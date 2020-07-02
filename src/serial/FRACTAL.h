//
// Created by not_tyche on 02/07/20.
//

#ifndef FRACTAL_FRACTAL_H
#define FRACTAL_FRACTAL_H

#define GAME_FRAME_PER_SECOND 8.0
#define WIDTH 120
#define HEIGHT 120
#define SCALE 5

#include <iostream>
using namespace std;

class FRACTAL {

    public:

        FRACTAL(): swap(false) {

            srand(time(0));

            this->fractal = new bool* [WIDTH];
            for (int i = 0; i < WIDTH; ++i)
                this->fractal[i] = new bool[HEIGHT];

//            for (auto i = 0; i < WIDTH; ++i)
//                for (auto j = 0; j < HEIGHT; ++j)
//                    this->fractal[i][j] =  rand()%2;

            this->fractal[10][11];
            this->fractal[11][12];
            this->fractal[13][14];
            this->fractal[20][21];
            this->fractal[22][32];
            this->fractal[12][12];
            this->fractal[13][17];
            this->fractal[13][11];
            this->fractal[15][18];

        }

        void future() {

            for (auto i = 0; i < WIDTH; ++i)
                for (auto j = 0; j < HEIGHT; ++j) {

                    auto Neighborhood = getNeighborhood(i,j);

                    auto oldSelf = 0;
                    auto newSelf = 0;

                    if(!swap){

                        oldSelf = fractal[i][j];
                        newSelf = ( 2 + (parity(Neighborhood) - oldSelf)) % 2;

                    }
                    else{

                        oldSelf = fractal[i][j];
                        newSelf = ( 2 + (parity(Neighborhood) - oldSelf)) % 2;

                        oldSelf = ( 2 + (parity(Neighborhood) - newSelf)) % 2;
                        newSelf = ( 2 + (parity(Neighborhood) - oldSelf)) % 2;
                    }

                    fractal[i][j] = newSelf;

                }

        }

        void changeRule(){ swap = !swap; }

        inline bool getCell(int i, int j) {
            return fractal[i][j];
        }

    private:

        bool** fractal; bool swap;

        static int getNeighborhood(int i, int j) {

            auto Neighborhood = 0;

            if( j - 1 >= 0 && i + 1 < WIDTH ) Neighborhood++;
            if( j + 1 < HEIGHT && i + 1 < WIDTH ) Neighborhood++;
            if( j - 1 >= 0 && i - 1 >= 0 ) Neighborhood++;
            if( j + 1 < HEIGHT && i - 1 >= 0 ) Neighborhood++;

            return Neighborhood;

        }

        static inline bool parity(int neighborhood) { return neighborhood % 2 == 0; }

};

#endif //FRACTAL_FRACTAL_H
