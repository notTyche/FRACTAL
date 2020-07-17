//
// Created by not_tyche on 30/06/20.
//

#include "application.h"
#include <iostream>
#include <cstdlib>
#include <mpi.h>
#include <allegro5/allegro.h>

#define SCREEN_WIDTH  600
#define SCREEN_HEIGHT 600

using namespace std;

int main(int argc, char** argv) {
//
//    al_init();
//    al_install_keyboard();
//
//    al_set_app_name("FRACTAL");
//
//
//    ALLEGRO_EVENT_QUEUE* queue;
//    if((queue = al_create_event_queue()) == nullptr)
//        return std::cerr << "al_create_event_queue() failed!" << std::endl, 1;
//
//    ALLEGRO_DISPLAY* disp;
//    if((disp = al_create_display(SCREEN_WIDTH, SCREEN_HEIGHT)) == nullptr)
//        return std::cerr << "al_create_display() failed!" << std::endl, 1;
//
//    ALLEGRO_TIMER* timer;
//    if((timer = al_create_timer(1 / GAME_FRAME_PER_SECOND)) == nullptr)
//        return std::cerr << "al_create_timer() failed!" << std::endl, 1;
//
//
//    al_set_window_title(disp, "FRACTAL");
//
//
//    al_register_event_source(queue, al_get_keyboard_event_source());
//    al_register_event_source(queue, al_get_display_event_source(disp));
//    al_register_event_source(queue, al_get_timer_event_source(timer));
//
//    al_start_timer(timer);
//    application app;
//
//    do {
//
//        ALLEGRO_EVENT e;
//        al_wait_for_event(queue, &e);
//
//        switch(e.type) {
//
//            case ALLEGRO_EVENT_TIMER:
//                app.draw(&e);
//                al_flip_display();
//                break;
//
//            case ALLEGRO_EVENT_KEY_DOWN:
//            case ALLEGRO_EVENT_KEY_UP:
//                app.update(&e);
//                break;
//
//            case ALLEGRO_EVENT_DISPLAY_CLOSE:
//                app.exit();
//                break;
//        }
//
//    } while(app.isRunning());
//
//    al_destroy_event_queue(queue);
//    al_destroy_timer(timer);
//    al_destroy_display(disp);

    FRACTAL f;
    double start = 0.0 , fine;
    start = MPI_Wtime();

    for (int i = 0; i < 100; ++i) {
        f.future();
    }

    fine = MPI_Wtime();
    std::cerr<<"Parallel execution time: "<<fine-start<<std::endl;

    return 0;
}