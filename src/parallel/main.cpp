//
// Created by not_tyche on 04/07/20.
//

#include <mpi.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <cstdlib>
#include <malloc.h>

#define MASTER           0
#define DIM             100
#define WIDTH           800
#define HEIGHT          800
#define SCALE            8
#define FPS             2.0

ALLEGRO_DISPLAY * display;
ALLEGRO_EVENT_QUEUE* queue;
ALLEGRO_TIMER* timer;
bool swapRule = false;

static inline bool parity(int neighborhood) { return neighborhood % 2 != 0; }

static int getNeighborhood(const int &i, const int &j, bool* &fractal, bool* &topFractal, bool* &downFractal) {

    auto Neighborhood = 0;

    if(i == 0) {

        if(j - 1 > 0)
            if(topFractal[j-1])
                Neighborhood++;

        if(j + 1 < DIM)
            if(topFractal[j+1])
                Neighborhood++;

        if( i + 1 < DIM && j + 1 < DIM )
            if(fractal[i+1 * DIM + j+1])
                Neighborhood++;

        if( i + 1 < DIM && j - 1 >= 0 )
            if(fractal[i+1 * DIM + j-1])
                Neighborhood++;

    }

    else if(i == DIM-1) {

        if(j - 1 > 0)
            if(downFractal[j-1])
                Neighborhood++;

        if(j + 1 < DIM)
            if(downFractal[j+1])
                Neighborhood++;

        if( i + 1 < DIM && j + 1 < DIM )
            if(fractal[i+1 * DIM + j+1])
                Neighborhood++;

        if( i + 1 < DIM && j - 1 >= 0 )
            if(fractal[i+1 * DIM + j-1])
                Neighborhood++;

    }

    else {

        if( i - 1 >= 0 && j - 1 >= 0)
            if(fractal[i-1 * DIM + j-1])
                Neighborhood++;

        if( i + 1 < DIM && j + 1 < DIM)
            if(fractal[i+1 * DIM + j+1])
                Neighborhood++;

        if( i - 1 >= 0 && j + 1 < DIM)
            if(fractal[i-1 * DIM + j+1])
                Neighborhood++;

        if( i + 1 < DIM && j - 1 >= 0)
            if(fractal[i+1 * DIM + j-1])
                Neighborhood++;
    }

    std::cerr<<Neighborhood<<std::endl;

    return Neighborhood;

}

void update(ALLEGRO_EVENT* e) {

    switch(e -> type) {

        case ALLEGRO_EVENT_KEY_DOWN:
            if(e->keyboard.keycode == ALLEGRO_KEY_M)
                swapRule = !swapRule;

//          if(e->keyboard.keycode == ALLEGRO_KEY_F)
//              this->paused = !this->paused;  //TODO...

            break;

    }

}


void redraw(const bool* fractal, const int& generation) {

    al_clear_to_color(al_map_rgb(0, 0, 0));

    for (unsigned i = 0; i < DIM; ++i)
        for (unsigned j = 0; j < DIM; ++j){

            if(fractal[i * DIM + j])
                al_draw_filled_rectangle(i * SCALE, j * SCALE,
                                         i * SCALE + SCALE,
                                         j * SCALE + SCALE, al_map_rgb(0,400,0));

        }

    al_draw_textf(al_create_builtin_font(), al_map_rgb(255,255,255) , 730 , 780 , ALLEGRO_ALIGN_CENTRE, "Generation: %d" , generation);
    al_draw_textf(al_create_builtin_font(), al_map_rgb(255,255,255) , 724 , 10 , ALLEGRO_ALIGN_CENTRE, "Press P to pause" );
    al_draw_textf(al_create_builtin_font(), al_map_rgb(255,255,255) , 700 , 20 , ALLEGRO_ALIGN_CENTRE, "Press R to change rule" );
    al_flip_display();
    al_rest(0.10);
}


int main(int argc, char *argv[]) {

    bool isRunning = true;

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

    bool *fractal = (bool*) malloc(DIM*DIM * sizeof(bool));

    for (auto i = 0; i < localDIM ; ++i)
        for (auto j = 0; j < DIM; ++j)
            fractal[i * DIM + j] = false;


    if(rank == MASTER){

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

        fractal[DIM/2 * DIM + DIM/2] = true;
        fractal[DIM/2+1 * DIM + DIM/2+1] = true;
        fractal[DIM/2-1 * DIM + DIM/2-1] = true;
    }


    bool * subFractal = (bool*) malloc(DIM * localDIM  * sizeof(bool));
    bool * subSupport = (bool*) malloc(DIM * localDIM  * sizeof(bool));

    bool * topRow = (bool*) malloc(DIM * sizeof(bool));
    bool * downRow = (bool*) malloc(DIM * sizeof(bool));

    MPI_Scatter (fractal, DIM * localDIM , MPI_C_BOOL, subFractal,DIM * localDIM ,MPI_C_BOOL, MASTER, MPI_COMM_WORLD);

    int topCell = (rank == 0) ? num_thread - 1 : rank - 1;
    int downCell = (rank == num_thread - 1) ? 0 : rank + 1;


    for(auto k = 0; k < 1000 && isRunning; ++k) {

        ALLEGRO_EVENT e;

        if (rank == MASTER)
            al_wait_for_event(queue, &e);

        switch(e.type) {

            case ALLEGRO_EVENT_TIMER:
            MPI_Send(&subFractal[0], DIM, MPI_C_BOOL, topCell, 10, MPI_COMM_WORLD);
            MPI_Send(&subFractal[(localDIM) * DIM - DIM], DIM, MPI_C_BOOL, downCell, 11, MPI_COMM_WORLD);

            MPI_Recv(downRow, DIM, MPI_C_BOOL, downCell, 10, MPI_COMM_WORLD, &status);
            MPI_Recv(topRow, DIM, MPI_C_BOOL, topCell, 11, MPI_COMM_WORLD, &status);


            for (auto i = 0; i < localDIM; ++i)
                for (auto j = 0; j < DIM; ++j)
                    subSupport[i * DIM + j] = subFractal[i * DIM + j];


            for (auto i = 0; i < localDIM; ++i)
                for (auto j = 0; j < DIM; ++j) {

                    auto Neighborhood = getNeighborhood(i, j, subSupport, topRow, downRow);

                    auto oldSelf = subSupport[i * DIM + j];
                    auto newSelf = (2 + (parity(Neighborhood) - oldSelf)) % 2;

                    subFractal[i * DIM + j] = newSelf;

                }

            MPI_Send(topRow, DIM, MPI_C_BOOL, topCell, 2, MPI_COMM_WORLD);
            MPI_Send(downRow, DIM, MPI_C_BOOL, downCell, 0, MPI_COMM_WORLD);

            MPI_Recv(&subFractal[DIM * (localDIM - 1)], DIM, MPI_C_BOOL, downCell, 2, MPI_COMM_WORLD, &status);
            MPI_Recv(&subFractal[0], DIM, MPI_C_BOOL, topCell, 0, MPI_COMM_WORLD, &status);

            MPI_Gather(subFractal, DIM * localDIM, MPI_C_BOOL, fractal, DIM * localDIM, MPI_C_BOOL, MASTER,
                       MPI_COMM_WORLD);

            if (rank == MASTER)
                redraw(fractal, k);
            break;

            case ALLEGRO_EVENT_KEY_DOWN:
            case ALLEGRO_EVENT_KEY_UP:
                if (rank == MASTER)
                    update(&e);
                break;

            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                MPI_Abort(MPI_COMM_WORLD, 0);
                isRunning = false;
                break;

        }

    }

    MPI_Barrier(MPI_COMM_WORLD);

    free(fractal);
    free(subFractal);
    free(subSupport);
    free(topRow);
    free(downRow);

    if (rank==MASTER){
        fine=MPI_Wtime();
        std::cerr<<"Parallel execution time: "<<fine-start<<std::endl;
    }

    MPI_Finalize();

    al_destroy_event_queue(queue);
    al_destroy_timer(timer);
    al_destroy_display(display);

    return 0;
}