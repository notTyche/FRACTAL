//
// Created by not_tyche on 30/06/20.
//

#include "Graphic.h"
#include <cmath>

#ifndef steps
#define steps 50
#endif

int main(int argc, char **argv)
{
    int N = 4;

    int showWidth = 1280; //set to 10 for the parallel speed test
    int showHeight = 720; //set to 10 for the parallel speed test

    bool done = false;
    int FPS = 25;

    ALLEGRO_EVENT_QUEUE *event_queue = NULL;
    ALLEGRO_TIMER *timer = NULL;

    if(!al_init())
        return -1;

    ALLEGRO_DISPLAY *display = al_create_display(showWidth,showHeight);
    ALLEGRO_BITMAP *buffer = al_create_bitmap(showWidth,showHeight);


    int windowHeight = al_get_display_height(display);
    int windowWidth = al_get_display_width(display);
    float sx = windowWidth / float(showWidth);
    float sy = windowHeight / float(showHeight);
    int scale = std::min(sx, sy);
    int scaleW = showWidth * scale;
    int scaleH = showHeight * scale;
    int scaleX = (windowWidth - scaleW) / 2;
    int scaleY = (windowHeight - scaleH) / 2;
    al_set_window_title(display, "FRACTAL");
    al_set_target_bitmap(buffer);
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_install_keyboard();
    event_queue = al_create_event_queue();
    timer = al_create_timer(3.0/FPS);


    Graphic manager(scaleW, scaleH, scaleX, scaleY, buffer, display,N);

    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_start_timer(timer);

    while(!done)
    {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            done = true;

        else if(ev.type == ALLEGRO_EVENT_KEY_UP)
            if(ev.keyboard.keycode==ALLEGRO_KEY_ESCAPE)
                done=true;

        if(!done && al_is_event_queue_empty(event_queue))
        {
            manager.drawMap();
            al_flip_display();
        }

    }

    al_destroy_event_queue(event_queue);
    al_destroy_timer(timer);
    al_destroy_display(display);

    return 0;
}

