//
// Created by not_tyche on 04/07/20.
//

#include <mpi.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <cstdlib>
#include <malloc.h>
#include <random>

#define PRIMARY           0
#define DIM             120
#define WIDTH           600
#define HEIGHT          600
#define SCALE            5
#define FPS             4.0

ALLEGRO_DISPLAY * display;
ALLEGRO_EVENT_QUEUE* queue;
ALLEGRO_TIMER* timer;

bool isRunning = true;
bool isPaused = false;
bool change = false;
bool swapRule = false;

inline int getUpper(const int& rank, const int& num_thread) { return rank == 0 ? num_thread - 1 : rank - 1; }

inline int getLower(const int& rank, const int& num_thread) { return rank == num_thread - 1 ? 0 : rank + 1; }

static inline bool parity(int neighborhood) { return neighborhood % 2 != 0; }

static int getNeighborhood(const int &i, const int &j, const bool* fractal, const bool* topFractal, const bool* downFractal, int& rank, int& num_thread, int& localDIM) {

    auto Neighborhood = 0;

    if(i == 0 && rank != PRIMARY) {

        if( j - 1 >= 0)
            if(topFractal[j-1])
                Neighborhood++;

        if( j + 1 < DIM)
            if(topFractal[j+1])
                Neighborhood++;

        if( i + 1 < localDIM && j + 1 < DIM )
            if(fractal[(i+1) * DIM + (j+1)])
                Neighborhood++;

        if( i + 1 < localDIM && j - 1 >= 0 )
            if(fractal[(i+1) * DIM + (j-1)])
                Neighborhood++;

    }

    else if(i == localDIM - 1 && rank != num_thread - 1) {

        if( j - 1 >= 0)
            if(downFractal[j-1])
                Neighborhood++;


        if( j + 1 < DIM)
            if(downFractal[j+1])
                Neighborhood++;

        if( i - 1 >= 0 && j + 1 < DIM )
            if(fractal[(i-1) * DIM + (j+1)])
                Neighborhood++;

        if( i - 1 >= 0 && j - 1 >= 0 )
            if(fractal[(i-1) * DIM + (j-1)])
                Neighborhood++;

    }

    else {

        if( i - 1 >= 0 && j - 1 >= 0)
            if(fractal[(i-1) * DIM + (j-1)])
                Neighborhood++;


        if( i + 1 < localDIM && j + 1 < DIM)
            if(fractal[(i+1) * DIM + (j+1)])
                Neighborhood++;


        if( i - 1 >= 0 && j + 1 < DIM)
            if(fractal[(i-1) * DIM + (j+1)])
                Neighborhood++;


        if( i + 1 < localDIM && j - 1 >= 0)
            if(fractal[(i+1) * DIM + (j-1)])
                Neighborhood++;

     }

    return Neighborhood;
}


void update(ALLEGRO_EVENT* e) {

    switch(e -> type) {

        case ALLEGRO_EVENT_KEY_DOWN:

        if(e -> keyboard.keycode == ALLEGRO_KEY_R)
            swapRule = !swapRule;

        if(e -> keyboard.keycode == ALLEGRO_KEY_P)
            isPaused = !isPaused;

        if(e -> keyboard.keycode == ALLEGRO_KEY_E)
            isRunning = !isRunning;

        if(e -> keyboard.keycode == ALLEGRO_KEY_C)
            change = !change;

        break;

    }

}

void redraw(const bool* fractal, const int& generation) {

    al_clear_to_color(al_map_rgb(0, 0, 0));

    for (unsigned i = 0; i < DIM; ++i)
        for (unsigned j = 0; j < DIM; ++j){

            if(!fractal[i * DIM + j])
                continue;

            al_draw_filled_rectangle(i * SCALE, j * SCALE, i * SCALE + SCALE, j * SCALE + SCALE, al_map_rgb(147,112,219));

        }

    al_draw_textf(al_create_builtin_font(), al_map_rgb(255,255,255) , 530 , 580 , ALLEGRO_ALIGN_CENTRE, "Generation: %d" , generation);
    al_draw_textf(al_create_builtin_font(), al_map_rgb(255,255,255) , 500 , 10 , ALLEGRO_ALIGN_CENTRE, "Press R to change rule" );
    al_draw_textf(al_create_builtin_font(), al_map_rgb(255,255,255) , 524 , 20 , ALLEGRO_ALIGN_CENTRE, "Press C to change pattern" );
    al_draw_textf(al_create_builtin_font(), al_map_rgb(255,255,255) , 524 , 30 , ALLEGRO_ALIGN_CENTRE, "Press P to pause" );
    al_draw_textf(al_create_builtin_font(), al_map_rgb(255,255,255) , 526 , 40 , ALLEGRO_ALIGN_CENTRE, "Press E to exit" );
    al_flip_display();
}

void step(bool* subFractal, bool* subSupport, bool* topRow, bool* downRow, int& localDIM, int& rank, int& num_thread){

    for (auto i = 0; i < localDIM; ++i)
        for (auto j = 0; j < DIM; ++j) {

            auto Neighborhood = getNeighborhood(i, j, subSupport, topRow, downRow, rank, num_thread, localDIM);

            auto oldSelf = subSupport[i * DIM + j];
            auto newSelf = (2 + (parity(Neighborhood) - oldSelf)) % 2;

            subFractal[i * DIM + j] = newSelf;

        }
}


void changePattern(bool* subFractal, const int& localDIM, int &rank, int &num_thread) {

    for (auto i = 0; i < localDIM ; ++i)
        for (auto j = 0; j < DIM; ++j)
            subFractal[i * DIM + j] = false;

    static bool init = false;
    static std::random_device rd;
    static std::mt19937 eng;
    static std::uniform_int_distribution<int> dist(0,5);

    if (!init) {
        eng.seed(rd());
        init = true;
    }

    auto choice = dist(eng);


    switch (choice) {

        case 0:
            subFractal[localDIM/2 * DIM + DIM/2] = true;
            break;

        case 1:
            subFractal[localDIM/2 * DIM + DIM/2 - 1] = true;
            break;

        case 2:
            subFractal[localDIM/2 + 1 * DIM + DIM/2 + 1] = true;
            break;

        case 3:
            subFractal[localDIM/2 * DIM + localDIM/2] = true;
            break;

        case 4:
            subFractal[localDIM/2 * DIM + DIM/2] = true;
            break;

        case 5:
            subFractal[localDIM/2 * DIM ] = true;
            break;

    }

}

int main(int argc, char *argv[]) {

    //MPI INIT
    int num_thread, rank;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_thread);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    double start = 0.0 , fine;

    //ALLEGRO INIT
    al_init();
    al_install_keyboard();
    al_init_primitives_addon();
    al_set_app_name("FRACTAL");
    al_set_window_title(display, "FRACTAL");


    int localDIM  = DIM / num_thread;

    if (rank == num_thread - 1)
        localDIM  += DIM % num_thread;


    bool *fractal = (bool*) calloc(1,DIM * DIM * sizeof(bool));

    for (auto i = 0; i < DIM ; ++i)
        for (auto j = 0; j < DIM; ++j)
            fractal[i * DIM + j] = false;

    bool * subFractal = (bool*) calloc(1,DIM * localDIM  * sizeof(bool));
    bool * subSupport = (bool*) calloc(1,DIM * localDIM  * sizeof(bool));


    bool * topRow = (bool*) calloc(1, DIM * sizeof(bool));
    bool * downRow = (bool*) calloc(1,DIM * sizeof(bool));


    if(rank == PRIMARY){

        if ((queue = al_create_event_queue()) == nullptr)
            return std::cerr << "al_create_event_queue() failed!" << std::endl, 1;

        if ((display = al_create_display(WIDTH, HEIGHT)) == nullptr)
            return std::cerr << "al_create_display() failed!" << std::endl, 1;

        if ((timer = al_create_timer(1 / FPS)) == nullptr)
            return std::cerr << "al_create_timer() failed!" << std::endl, 1;

        al_register_event_source(queue, al_get_keyboard_event_source());
        al_register_event_source(queue, al_get_display_event_source(display));
        al_register_event_source(queue, al_get_timer_event_source(timer));
        al_start_timer(timer);


        start = MPI_Wtime();


        fractal[(DIM/2) * DIM + (DIM/2)] = true;
        fractal[(DIM/2 + 1) * DIM + (DIM/2 + 1)] = true;
        fractal[(DIM/2 - 1) * DIM + (DIM/2 - 1)] = true;

    }



    MPI_Scatter (fractal, DIM * localDIM , MPI_C_BOOL, subFractal,DIM * localDIM ,MPI_C_BOOL, PRIMARY, MPI_COMM_WORLD);

    auto upper = getUpper(rank, num_thread);
    auto lower = getLower(rank, num_thread);

    for(auto it = 0; it < 1000 && isRunning; ++it) {

        if(change) {

            changePattern(subFractal, localDIM, rank, num_thread);
            change = false;

        }

        if(!isPaused) {

            MPI_Send(&subFractal[0], DIM, MPI_C_BOOL, upper, 1, MPI_COMM_WORLD);
            MPI_Send(&subFractal[(localDIM) * DIM - DIM], DIM, MPI_C_BOOL, lower, 2, MPI_COMM_WORLD);

            MPI_Recv(downRow, DIM, MPI_C_BOOL, lower, 1, MPI_COMM_WORLD, &status);
            MPI_Recv(topRow, DIM, MPI_C_BOOL, upper, 2, MPI_COMM_WORLD, &status);


            for (auto i = 0; i < localDIM; ++i)
                for (auto j = 0; j < DIM; ++j)
                    subSupport[i * DIM + j] = subFractal[i * DIM + j];


            step(subFractal, subSupport, topRow, downRow, localDIM, rank, num_thread);


            MPI_Gather(subFractal, DIM * (localDIM), MPI_C_BOOL, fractal, DIM * (localDIM), MPI_C_BOOL, PRIMARY, MPI_COMM_WORLD);

        } else it--;



        if (rank == PRIMARY) {

            ALLEGRO_EVENT e;

            if (al_get_next_event(queue, &e)) {
                switch (e.type) {

                    case ALLEGRO_EVENT_TIMER:
                        redraw(fractal, it);
                        break;

                    case ALLEGRO_EVENT_KEY_DOWN:
                    case ALLEGRO_EVENT_KEY_UP:
                        update(&e);
                        break;

                    case ALLEGRO_EVENT_DISPLAY_CLOSE:
                        MPI_Abort(MPI_COMM_WORLD, 0);
                        isRunning = false;
                        break;
                }
            }
        }

        MPI_Bcast( &swapRule, 1, MPI_C_BOOL, PRIMARY, MPI_COMM_WORLD );
        MPI_Bcast( &isPaused, 1, MPI_C_BOOL, PRIMARY, MPI_COMM_WORLD );
        MPI_Bcast( &isRunning, 1, MPI_C_BOOL, PRIMARY, MPI_COMM_WORLD );
        MPI_Bcast( &change, 1, MPI_C_BOOL, PRIMARY, MPI_COMM_WORLD );


        al_rest(0.15);
    }


    MPI_Barrier(MPI_COMM_WORLD);

    if (rank= = PRIMARY){
        fine=MPI_Wtime();
        std::cerr<<"Parallel execution time: "<<fine-start<<std::endl;
    }

    MPI_Finalize();

    al_destroy_event_queue(queue);
    al_destroy_timer(timer);
    al_destroy_display(display);

    return 0;
}