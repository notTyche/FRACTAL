//
// Created by not_tyche on 02/07/20.
//

#ifndef FRACTAL_APPLICATION_H
#define FRACTAL_APPLICATION_H

#include <allegro5/events.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include "FRACTAL.h"

#include <iostream>
using namespace std;

class application {

    public:

    application(): running(true) {}

    void draw(ALLEGRO_EVENT* e){

        al_clear_to_color(al_map_rgb(0, 0, 0));

        for(auto x = 0; x < WIDTH ; x++)
            for(auto y = 0; y < HEIGHT ; y++) {

                if(!fractal.getCell(x, y))
                    continue;

                al_draw_filled_rectangle(x*SCALE ,y*SCALE , x*SCALE+SCALE, y*SCALE+SCALE, al_map_rgb(0x29,0x62,0xFF));

            }

        fractal.future();
    }

    void update(ALLEGRO_EVENT* e) {

        switch(e -> type) {

            case ALLEGRO_EVENT_KEY_DOWN:
                if(e->keyboard.keycode == ALLEGRO_KEY_M)
                    fractal.changeRule();

//                if(e->keyboard.keycode == ALLEGRO_KEY_F)
//                    this->paused = !this->paused;  //TODO...
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
