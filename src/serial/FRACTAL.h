//
// Created by not_tyche on 02/07/20.
//

#ifndef FRACTAL_FRACTAL_H
#define FRACTAL_FRACTAL_H

#define GAME_FRAME_PER_SECOND 2.0
#define WIDTH 120
#define HEIGHT 120
#define SCALE 5

#include <iostream>
#include <bitset>
using namespace std;

class FRACTAL {

    public:

        FRACTAL(): swap(false) {


            this->fractal = new int* [WIDTH];
            for (int i = 0; i < WIDTH; ++i)
                this->fractal[i] = new int[HEIGHT];

            this->fractalTemp = new int* [WIDTH];
            for (int i = 0; i < WIDTH; ++i)
                this->fractalTemp[i] = new int[HEIGHT];

            for (auto i = 0; i < WIDTH; ++i)
                for (auto j = 0; j < HEIGHT; ++j)
                    this->fractal[i][j] =  0;

            for (auto i = 0; i < WIDTH; ++i)
                for (auto j = 0; j < HEIGHT; ++j)
                    this->fractalTemp[i][j] =  0;


            this->fractal[WIDTH/2][HEIGHT/2] = 1;


        }

        void future() {

            if(swap) {

                for (auto i = 0; i < WIDTH; ++i)
                    for (auto j = 0; j < HEIGHT; ++j) {

                        auto b1 = fractal[i][j] & 1;
                        auto b2 = (fractal[i][j] & 2) >> 1;

                        fractal[i][j] = b2 | (b1 << 1);

                    }

                changeRule();

            }

            for (auto i = 0; i < WIDTH; ++i)
                for (auto j = 0; j < HEIGHT; ++j) {

                    auto mem = (fractal[i][j] >> 1) & 1;
                    auto Neighborhood = getNeighborhood(i, j, fractal) + mem;

                    auto newSelf = parity(Neighborhood);

                    fractalTemp[i][j] = ( (fractal[i][j] & 1) << 1 ) | newSelf;

                }

            for (auto i = 0; i < WIDTH; ++i)
                for (auto j = 0; j < HEIGHT; ++j)
                    fractal[i][j] = fractalTemp[i][j];
        }

        inline void changeRule(){ swap = !swap; }

        inline int getCell(int i, int j) {
            return (fractal[i][j] & 1);
        }

        inline bool getSwap() const {
            return swap;
        }

    private:

        int** fractal; bool swap; int** fractalTemp;

        static int getNeighborhood(int i, int j, int** fractal) {

            auto Neighborhood = 0;

            if( j - 1 >= 0 && i + 1 < WIDTH )
                if(fractal[j-1][i+1])
                    Neighborhood++;

            if( j + 1 < HEIGHT && i + 1 < WIDTH )
                if(fractal[j+1][i+1])
                    Neighborhood++;

            if( j - 1 >= 0 && i - 1 >= 0 )
                if(fractal[j-1][i-1])
                    Neighborhood++;

            if( j + 1 < HEIGHT && i - 1 >= 0 )
                if(fractal[j+1][i-1])
                    Neighborhood++;

            return Neighborhood;



        }

        static inline bool parity(int neighborhood) { return neighborhood % 2 != 0; }

};

#endif //FRACTAL_FRACTAL_H
