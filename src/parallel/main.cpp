//
// Created by Perfidio Matteo on 04/07/20.
//
// to compile :    mpic++ -lallegro_font -lallegro_main -lallegro_primitives -lallegro main.cpp -o1
//                 mpirun ./1
//

#include <mpi.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <cstdlib>
#include <malloc.h>
#include <random>
#include <list>
#include <algorithm>

#define PRIMARY           0
#define DIM             400
#define WIDTH           800
#define HEIGHT          800
#define SCALE        (WIDTH/DIM)
#define FPS             24.0

ALLEGRO_DISPLAY * display;
ALLEGRO_EVENT_QUEUE * queue;
ALLEGRO_TIMER * timer;
ALLEGRO_TIMER * timer_draw;

bool isRunning = true;
bool isPaused = false;
bool change = false;
bool swapRule = false;
bool cleaned = false;
bool clickedON = false;
bool advance = true;
bool screenEmpty = false;

inline int getUpper(const int& rank, const int& num_thread) { return rank == 0 ? num_thread - 1 : rank - 1; }


inline int getLower(const int& rank, const int& num_thread) { return rank == num_thread - 1 ? 0 : rank + 1; }


inline void restart(int& it) { it = 0; }


inline void next(int& it) { it++; }


static inline bool parity(int neighborhood) { return neighborhood % 2 != 0; }


static int getNeighborhood(const int &i, const int &j, const int* fractal, const int* topFractal, const int* downFractal, int& rank, int& num_thread, int& localDIM) {

    auto Neighborhood = 0;

    if(i == 0) {

        if(rank != PRIMARY) {

            if (j - 1 >= 0)
                if (topFractal[j - 1] & 1)
                    Neighborhood++;

            if (j + 1 < DIM)
                if (topFractal[j + 1] & 1)
                    Neighborhood++;
        }


        if (i + 1 < localDIM && j + 1 < DIM)
            if (fractal[(i + 1) * DIM + (j + 1)] & 1)
                Neighborhood++;

        if (i + 1 < localDIM && j - 1 >= 0)
            if (fractal[(i + 1) * DIM + (j - 1)] & 1)
                Neighborhood++;

    }

    else if(i == localDIM - 1) {

        if(rank != num_thread - 1) {

            if (j - 1 >= 0)
                if (downFractal[j - 1] & 1)
                    Neighborhood++;


            if (j + 1 < DIM)
                if (downFractal[j + 1] & 1)
                    Neighborhood++;

        }

        if (i - 1 >= 0 && j + 1 < DIM)
            if (fractal[(i - 1) * DIM + (j + 1)] & 1)
                Neighborhood++;

        if (i - 1 >= 0 && j - 1 >= 0)
            if (fractal[(i - 1) * DIM + (j - 1)] & 1)
                Neighborhood++;

    }

    else {


        if( i - 1 >= 0 && j - 1 >= 0)
            if(fractal[(i-1) * DIM + (j-1)] & 1)
                Neighborhood++;

        if( i + 1 < localDIM && j + 1 < DIM)
            if(fractal[(i+1) * DIM + (j+1)] & 1)
                Neighborhood++;


        if( i - 1 >= 0 && j + 1 < DIM)
            if(fractal[(i-1) * DIM + (j+1)] & 1)
                Neighborhood++;


        if( i + 1 < localDIM && j - 1 >= 0)
            if(fractal[(i+1) * DIM + (j-1)] & 1)
                Neighborhood++;


     }

    return Neighborhood;
}



static int getNeighborhood(int i, int j, const int* fractal) {

    auto Neighborhood = 0;


    if( i - 1 >= 0 && j - 1 >= 0)
        if(fractal[(i-1) * DIM + (j-1)])
            Neighborhood++;


    if( i + 1 < DIM && j + 1 < DIM)
        if(fractal[(i+1) * DIM + (j+1)])
            Neighborhood++;


    if( i - 1 >= 0 && j + 1 < DIM)
        if(fractal[(i-1) * DIM + (j+1)])
            Neighborhood++;


    if( i + 1 < DIM && j - 1 >= 0)
        if(fractal[(i+1) * DIM + (j-1)])
            Neighborhood++;


    return Neighborhood;

}



void clean(int* plane, int* planeSupport, const int& localDIM) {

    for (auto i = 0; i < localDIM ; ++i)
        for (auto j = 0; j < DIM; ++j){
            plane[i * DIM + j] = 0;
            planeSupport[i * DIM + j] = 0;
        }

}



void clean(int* plane, const int& localDIM) {

    for (auto i = 0; i < localDIM ; ++i)
        for (auto j = 0; j < DIM; ++j)
            plane[i * DIM + j] = 0;

}



void clean(int* plane) {

    for (auto i = 0; i < DIM ; ++i)
        for (auto j = 0; j < DIM; ++j)
            plane[i * DIM + j] = 0;

}



void update(ALLEGRO_EVENT* e, std::list<int>& fractal_event) {

    switch(e -> type) {

        case ALLEGRO_EVENT_KEY_DOWN:

            if(e -> keyboard.keycode == ALLEGRO_KEY_R)
                swapRule = !swapRule;

            if(e -> keyboard.keycode == ALLEGRO_KEY_P)
                isPaused = !isPaused;

            if(e -> keyboard.keycode == ALLEGRO_KEY_E)
                isRunning = !isRunning;

            if(e -> keyboard.keycode == ALLEGRO_KEY_X)
                change = !change;

            if(e -> keyboard.keycode == ALLEGRO_KEY_C)
                cleaned = !cleaned;

            break;

        case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:

            auto mouse_x = e->mouse.x / SCALE;
            auto mouse_y = e->mouse.y / SCALE;

            if(e -> mouse.pressure > 0) {
                fractal_event.push_back(mouse_x);
                fractal_event.push_back(mouse_y);
            }

            clickedON = true;
            break;

    }

}



void redraw(const int* fractal, const int& generation) {

    al_clear_to_color(al_map_rgb(0, 0, 0));

    for (auto i = 0; i < DIM; ++i)
        for (auto j = 0; j < DIM; ++j){


            if(! (fractal[i * DIM + j] & 1) )
                continue;

            else if(getNeighborhood(i, j, fractal) == 1) {

                al_draw_filled_rectangle(j * SCALE, i * SCALE, j * SCALE + SCALE, i * SCALE + SCALE,al_map_rgb(0, 255, 0));
                continue;

            }

            else if(getNeighborhood(i, j, fractal) == 2) {

                al_draw_filled_rectangle(j * SCALE, i * SCALE, j * SCALE + SCALE, i * SCALE + SCALE,al_map_rgb(0, 0, 255));
                continue;
            }

            al_draw_filled_rectangle(j * SCALE, i * SCALE, j * SCALE + SCALE, i * SCALE + SCALE, al_map_rgb(255, 0, 0));

        }

    al_draw_textf(al_create_builtin_font(), al_map_rgb(255,255,255) , 730 , 780 , ALLEGRO_ALIGN_CENTRE, "Generation: %d" , generation);
    al_draw_textf(al_create_builtin_font(), al_map_rgb(255,255,255) , 656 , 10 , ALLEGRO_ALIGN_CENTRE, "Click MOUSE SX to generate fractal" );
    al_draw_textf(al_create_builtin_font(), al_map_rgb(255,255,255) , 682 , 30 , ALLEGRO_ALIGN_CENTRE, "Press C to clear the screen" );
    al_draw_textf(al_create_builtin_font(), al_map_rgb(255,255,255) , 690 , 50 , ALLEGRO_ALIGN_CENTRE, "Press X to change pattern" );
    al_draw_textf(al_create_builtin_font(), al_map_rgb(255,255,255) , 702 , 70 , ALLEGRO_ALIGN_CENTRE, "Press R to change rule" );
    al_draw_textf(al_create_builtin_font(), al_map_rgb(255,255,255) , 726 , 90 , ALLEGRO_ALIGN_CENTRE, "Press P to pause" );
    al_draw_textf(al_create_builtin_font(), al_map_rgb(255,255,255) , 730 , 110 , ALLEGRO_ALIGN_CENTRE, "Press E to exit" );
    al_flip_display();
}



void step(int* plane, int* planeSupport, int* topRow, int* downRow, int& localDIM, int& rank, int& num_thread){

    // ANNOTATION:
    // mem                       -----> first bit of right - shift of current self
    // Neighborhood              -----> sum of ( NE + NO + SE + SO ) of current self and mem
    // newSelf                   -----> * parity of Neighborhood
    // planeSupport[i * DIM + j] -----> first bit of left - shift of plane[i * DIM + j] or newSelf
    // swapRule                  -----> inverts the equation to return to the initial fractal
    // * true if is odd


    if(swapRule) {

        //reverses ghost cell

        for (auto j = 0; j < DIM; ++j) {

            auto b1 = topRow[j] & 1;
            auto b2 = (topRow[j] & 2) >> 1;

            topRow[j] = b2 | (b1 << 1);

        }


        for (auto j = 0; j < DIM; ++j) {

            auto b1 = downRow[j] & 1;
            auto b2 = (downRow[j] & 2) >> 1;

            downRow[j] = b2 | (b1 << 1);

        }


        //reverses thread plane

        for (auto i = 0; i < localDIM; ++i)
            for (auto j = 0; j < DIM; ++j) {

                auto b1 = plane[i * DIM + j] & 1;
                auto b2 = (plane[i * DIM + j] & 2) >> 1;

                plane[i * DIM + j] = b2 | (b1 << 1);

            }

        swapRule = false;
    }




    for (auto i = 0; i < localDIM; ++i)
        for (auto j = 0; j < DIM; ++j) {

            auto mem = (plane[i * DIM + j] >> 1) & 1;
            auto Neighborhood = getNeighborhood(i, j, plane, topRow, downRow, rank, num_thread, localDIM) + mem;

            auto newSelf = parity(Neighborhood);

            planeSupport[i * DIM + j] = ( (plane[i * DIM + j] & 1) << 1 ) | newSelf;

        }


    memcpy(plane, planeSupport, DIM * localDIM * sizeof(int));
}


int generateRandom(const int& start, const int& end){

    static bool init = false;
    static std::random_device rd;
    static std::mt19937 eng;
    static std::uniform_int_distribution<int> dist(start, end);

    if (!init) {
        eng.seed(rd());
        init = true;
    }

    return dist(eng);

}


void changePattern(int* plane, int* planeSupport, const int& localDIM, int &rank, int &num_thread) {

    clean(plane, planeSupport, localDIM);

    auto y = generateRandom(0, localDIM - 1);
    auto x = generateRandom(0, DIM);

    plane[y * DIM + x] = 1;

}


int main(int argc, char *argv[]) {

    //MPI INIT
    int num_thread, rank;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_thread);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    double start = 0.0 , end;

    //ALLEGRO INIT
    al_init();
    al_install_keyboard();
    al_install_mouse();
    al_init_primitives_addon();
    al_set_app_name("FRACTAL");
    al_set_window_title(display, "FRACTAL");



    int localDIM  = DIM / num_thread;

    if (rank == num_thread - 1)
        localDIM  += DIM % num_thread;



    int *fractal = (int*) calloc(1,DIM * DIM * sizeof(int));

    for (auto i = 0; i < DIM ; ++i)
        for (auto j = 0; j < DIM; ++j)
            fractal[i * DIM + j] = 0;



    int * plane = (int*) calloc(1,DIM * localDIM  * sizeof(int));
    int * planeSupport = (int*) calloc(1,DIM * localDIM  * sizeof(int));


    int * topRow = (int*) calloc(1, DIM * sizeof(int));
    int * downRow = (int*) calloc(1,DIM * sizeof(int));



    int *temp_event = (int*) calloc(1,DIM * DIM * sizeof(int));
    int *temp = (int*) calloc(1,DIM * DIM * sizeof(int));

    for (auto i = 0; i < DIM ; ++i)
        for (auto j = 0; j < DIM; ++j)
            temp_event[i * DIM + j] = 0;



    std::list<int> fractal_event;



    if(rank == PRIMARY){


        if ((queue = al_create_event_queue()) == nullptr)
            return std::cerr << "al_create_event_queue() failed!" << std::endl, 1;

        if ((display = al_create_display(WIDTH, HEIGHT)) == nullptr)
            return std::cerr << "al_create_display() failed!" << std::endl, 1;

        if ((timer = al_create_timer(1 / FPS)) == nullptr)
            return std::cerr << "al_create_timer() failed!" << std::endl, 1;

        if ((timer_draw = al_create_timer(1 / FPS)) == nullptr)
            return std::cerr << "al_create_timer_draw() failed!" << std::endl, 1;


        al_register_event_source(queue, al_get_keyboard_event_source());
        al_register_event_source(queue, al_get_display_event_source(display));
        al_register_event_source(queue, al_get_timer_event_source(timer));
        al_register_event_source(queue, al_get_timer_event_source(timer_draw));
        al_register_event_source(queue, al_get_mouse_event_source());

        al_start_timer(timer);
        al_start_timer(timer_draw);


        start = MPI_Wtime();


        fractal[(DIM/2) * DIM + (DIM/2)] = 1;

    }



    MPI_Scatter (fractal, DIM * localDIM , MPI_INT, plane,DIM * localDIM ,MPI_INT, PRIMARY, MPI_COMM_WORLD);


    auto upper = getUpper(rank, num_thread);
    auto lower = getLower(rank, num_thread);



    for(auto it = 0; it < 100000 && isRunning; ) {

        if(screenEmpty)
            restart(it);

        if(change) {

            auto rankChoice = 0;

            if(rank == PRIMARY)
                rankChoice = generateRandom(0, num_thread - 1);


            MPI_Bcast( &rankChoice, 1, MPI_INT, PRIMARY, MPI_COMM_WORLD );


            if(rank == rankChoice)
                changePattern(plane, planeSupport, localDIM, rankChoice, num_thread);
            else
                clean(plane, planeSupport, localDIM);


            restart(it);
            change = false;
            screenEmpty = false;
        }


        if(cleaned){

            restart(it);

            if(rank == PRIMARY)
                clean(fractal);

            clean(plane, planeSupport, localDIM);
            cleaned = false;
            screenEmpty = true;

        }


        if(!isPaused && !screenEmpty) {


            MPI_Send(&plane[0], DIM, MPI_INT, upper, 1, MPI_COMM_WORLD);
            MPI_Send(&plane[(localDIM) * DIM - DIM], DIM, MPI_INT, lower, 2, MPI_COMM_WORLD);

            MPI_Recv(downRow, DIM, MPI_INT, lower, 1, MPI_COMM_WORLD, &status);
            MPI_Recv(topRow, DIM, MPI_INT, upper, 2, MPI_COMM_WORLD, &status);


            if(advance) {

                step(plane, planeSupport, topRow, downRow, localDIM, rank, num_thread);
                next(it);
                advance = false;

            }


            MPI_Gather(plane, DIM * (localDIM), MPI_INT, fractal, DIM * (localDIM), MPI_INT, PRIMARY, MPI_COMM_WORLD);

        }



        if (rank == PRIMARY) {

            ALLEGRO_EVENT e;

            if (al_get_next_event(queue, &e)) {

                switch (e.type) {

                    case ALLEGRO_EVENT_TIMER:

                        if(e.timer.source == timer)
                            advance = true;

                        if(e.timer.source == timer_draw)
                            redraw(fractal, it);

                        break;

                    case ALLEGRO_EVENT_KEY_DOWN:
                    case ALLEGRO_EVENT_KEY_UP:
                    case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
                    case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
                        update(&e, fractal_event);
                        break;

                    case ALLEGRO_EVENT_DISPLAY_CLOSE:

                        isRunning = false;
                        break;
                }

            }

        }


        MPI_Bcast(&advance, 1, MPI_C_BOOL, PRIMARY, MPI_COMM_WORLD );
        MPI_Bcast(&isPaused, 1, MPI_C_BOOL, PRIMARY, MPI_COMM_WORLD);
        MPI_Bcast(&isRunning, 1, MPI_C_BOOL, PRIMARY, MPI_COMM_WORLD);
        MPI_Bcast(&change, 1, MPI_C_BOOL, PRIMARY, MPI_COMM_WORLD);
        MPI_Bcast(&swapRule, 1, MPI_C_BOOL, PRIMARY, MPI_COMM_WORLD);
        MPI_Bcast(&cleaned, 1, MPI_C_BOOL, PRIMARY, MPI_COMM_WORLD);
        MPI_Bcast(&clickedON, 1, MPI_C_BOOL, PRIMARY, MPI_COMM_WORLD);



        if(rank == PRIMARY) {


            auto i = 0;

            while ( i < fractal_event.size()) {

                auto x = fractal_event.front();
                fractal_event.pop_front();

                auto y = fractal_event.front();
                fractal_event.pop_front();

                temp_event[y * DIM + x] = 1;
            }


        }


        if(clickedON){

            MPI_Scatter (temp_event, DIM * localDIM , MPI_INT, temp,DIM * localDIM ,MPI_INT, PRIMARY, MPI_COMM_WORLD);

            for (auto i = 0; i < localDIM; ++i)
                for (auto j = 0; j < DIM; ++j) {

                    if(temp[i * DIM + j] == 1 && !((plane[i * DIM + j] &1 ) == 1))
                        plane[i * DIM + j] = 1;

                }


            clean(temp_event);
            clean(temp, localDIM);
            clickedON = false;
            screenEmpty = false;

        }


    }



    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == PRIMARY){
        end = MPI_Wtime();
        std::cerr<<"Parallel execution time: "<< end - start<<std::endl;
    }


    MPI_Finalize();

    al_destroy_event_queue(queue);
    al_destroy_timer(timer);
    al_destroy_display(display);

    return 0;
}