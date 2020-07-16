//
// Created by not_tyche on 02/07/20.
//

#ifndef FRACTAL_APPLICATION_H
#define FRACTAL_APPLICATION_H

#include <allegro5/events.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include "FRACTAL.h"

#include <iostream>
using namespace std;

class application {

    public:

    application(): running(true) {}

    void draw(ALLEGRO_EVENT* e, int& generation){

        al_clear_to_color(al_map_rgb(0, 0, 0));

        for (auto i = 0; i < WIDTH; ++i)
            for (auto j = 0; j < HEIGHT; ++j){


                if(fractal.getCell(i,j))
                    continue;

                else if(fractal.getNeighborhood(i, j) == 1) {

                    al_draw_filled_rectangle(j * SCALE, i * SCALE, j * SCALE + SCALE, i * SCALE + SCALE,al_map_rgb(0, 255, 0));
                    continue;

                }

                else if(fractal.getNeighborhood(i, j) == 2) {

                    al_draw_filled_rectangle(j * SCALE, i * SCALE, j * SCALE + SCALE, i * SCALE + SCALE,al_map_rgb(255, 0, 0));
                    continue;
                }

                al_draw_filled_rectangle(j * SCALE, i * SCALE, j * SCALE + SCALE, i * SCALE + SCALE, al_map_rgb(0, 0, 255));

            }


        al_draw_textf(al_create_builtin_font(), al_map_rgb(255,255,255) , 730 , 780 , ALLEGRO_ALIGN_CENTRE, "Generation: %d" , generation);
        al_draw_textf(al_create_builtin_font(), al_map_rgb(255,255,255) , 682 , 10 , ALLEGRO_ALIGN_CENTRE, "Press X to clear the screen" );
        al_draw_textf(al_create_builtin_font(), al_map_rgb(255,255,255) , 690 , 30 , ALLEGRO_ALIGN_CENTRE, "Press C to change pattern" );
        al_draw_textf(al_create_builtin_font(), al_map_rgb(255,255,255) , 702 , 50 , ALLEGRO_ALIGN_CENTRE, "Press R to change rule" );
        al_draw_textf(al_create_builtin_font(), al_map_rgb(255,255,255) , 726 , 70 , ALLEGRO_ALIGN_CENTRE, "Press P to pause" );
        al_draw_textf(al_create_builtin_font(), al_map_rgb(255,255,255) , 730 , 90 , ALLEGRO_ALIGN_CENTRE, "Press E to exit" );
        al_flip_display();

        fractal.future();
    }

    void update(ALLEGRO_EVENT* e) {

        switch(e -> type) {

            case ALLEGRO_EVENT_KEY_DOWN:
                if(e->keyboard.keycode == ALLEGRO_KEY_M)
                    fractal.changeRule();
                break;

        }

    }

    inline void exit() { running = false; }

    inline auto isRunning() const { return running; }

    private:

    FRACTAL fractal;
    bool running;


};


#endif //FRACTAL_APPLICATION_H
