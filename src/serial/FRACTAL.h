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

            //srand(time(0));

            this->fractal = new bool* [WIDTH];
            for (int i = 0; i < WIDTH; ++i)
                this->fractal[i] = new bool[HEIGHT];

            this->fractalTemp = new bool* [WIDTH];
            for (int i = 0; i < WIDTH; ++i)
                this->fractalTemp[i] = new bool[HEIGHT];

            for (auto i = 0; i < WIDTH; ++i)
                for (auto j = 0; j < HEIGHT; ++j)
                    this->fractal[i][j] =  false;

            for (auto i = 0; i < WIDTH; ++i)
                for (auto j = 0; j < HEIGHT; ++j)
                    this->fractalTemp[i][j] =  false;


            this->fractal[WIDTH/2][HEIGHT/2] = true;
            this->fractal[WIDTH/2+1][HEIGHT/2+1]= true;
            this->fractal[WIDTH/2-1][HEIGHT/2-1]= true;


        }

        void future() {

            for (auto i = 0; i < WIDTH; ++i)
                for (auto j = 0; j < HEIGHT; ++j)
                    fractalTemp[i][j] = fractal[i][j];


            for (auto i = 0; i < WIDTH; ++i)
                for (auto j = 0; j < HEIGHT; ++j) {

                    auto Neighborhood = getNeighborhood(i, j, fractalTemp);

                    auto oldSelf = fractalTemp[i][j];
                    auto newSelf = (2 + (parity(Neighborhood) - oldSelf)) % 2;

                    fractal[i][j] = newSelf;
                }

            if(swap) {

                for (auto i = 0; i < WIDTH; ++i)
                    for (auto j = 0; j < HEIGHT; ++j) {

                        auto Neighborhood = getNeighborhood(i, j, fractal);

                        auto newSelf = fractal[i][j];
                        auto oldSelf = (2 + (parity(Neighborhood) - newSelf)) % 2;

                        fractalTemp[i][j] = oldSelf;

                        cout<<fractal[i][j]<<endl;
                    }

                for (auto i = 0; i < WIDTH; ++i)
                    for (auto j = 0; j < HEIGHT; ++j)
                        fractal[i][j] = fractalTemp[i][j];

                    swap=false;
            }

        }

        inline void changeRule(){ swap = !swap; }

        inline bool getCell(int i, int j) {
            return fractal[i][j];
        }

        inline bool getSwap() const {
            return swap;
        }

    private:

        bool** fractal; bool swap; bool** fractalTemp;

        static int getNeighborhood(int i, int j, bool** fractal) {

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
