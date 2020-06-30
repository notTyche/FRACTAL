//
// Created by not_tyche on 30/06/20.
//

#ifndef FRACTAL_GRAPHIC_H
#define FRACTAL_GRAPHIC_H

#include <iostream>
#include <string>
#include <ctime>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

using namespace std;

int x=37;
int y=24;
int pixel=16;


class Graphic{

    private:
        bool** matrix;
        ALLEGRO_BITMAP* buffer;
        ALLEGRO_DISPLAY *display;
        int scale_w; int scale_h; int scale_x; int scale_y;
        bool swap;
        int x; int n;

    public:

    Graphic(const int &scale_w, const int &scale_h, const int &scale_x, const int &scale_y, ALLEGRO_BITMAP *buffer, ALLEGRO_DISPLAY *display, int& x) :
            scale_w(scale_w),scale_h(scale_h), scale_x(scale_x), scale_y(scale_y), buffer(buffer), display(display), x(x), n(pixel*x),swap(false) {

        al_init_primitives_addon();
        srand(time(nullptr));

        auto count = 0;
        matrix = new bool*[n];

        for(int i=0; i<n; i++)
            matrix[i]=new bool[n];

        #pragma omp parallel for schedule(dynamic)
        for (auto i=0; i<n; i++)
            for (auto j=0; j<n; j++)
            {
                matrix[i][j] = count != (x * y) / 2;

                if(matrix[i][j])
                        count++;
            }

    }


    static inline bool parity(int neighborhood) { return neighborhood % 2 == 0; }

    void nextGeneration()
    {
        bool** future = new bool*[n];
        for(int i=0; i<100 ;i++)
            future[i]=new bool[n];

        for (auto i = 0; i < n; i++)
            for (auto j = 0; j < n; j++)
            {
                auto Neighborhood = 0;
                auto oldSelf = future[i][j];

                if(j - 1 >= 0 && i + 1 < n ) Neighborhood++;
                if(j + 1 < n && i + 1 < n) Neighborhood++;
                if(j - 1 >= 0 && i - 1 >= 0) Neighborhood++;
                if(j + 1 < n && i - 1 >= 0) Neighborhood++;

                auto newSelf = ( 2 + (parity(Neighborhood) - oldSelf)) % 2;

                future[i][j] = newSelf;

            }


        for(int i=0; i<n; i++)
            delete[] matrix[i];
        delete[] matrix;

        matrix=future;
    }


    void drawMap ()
    {
        al_set_target_bitmap(buffer);
        al_clear_to_color(al_map_rgb(0, 0, 0));
        nextGeneration();
        for(int i=0;i<n;i++)
            for(int j=0;j<n;j++){
                if(matrix[i][j]==1)
                    al_draw_filled_rectangle(j*pixel-8,i*pixel-8,j*pixel+8,i*pixel+8,al_map_rgb(rand()%256,rand()%256,rand()%256));
                else if(matrix[i][j]==0)
                    al_draw_filled_rectangle(j*pixel-8,i*pixel-8,j*pixel+8,i*pixel+8,al_map_rgb(0,0,0));
            }
        al_set_target_backbuffer(this->display);
        al_clear_to_color(al_map_rgb(0, 0, 0));
        al_draw_scaled_bitmap(buffer, 0, 0,(x*pixel) ,(y*pixel), scale_x, scale_y, scale_w, scale_h, 0);
    }

    ~Graphic()= default;
};
#endif //FRACTAL_GRAPHIC_H
