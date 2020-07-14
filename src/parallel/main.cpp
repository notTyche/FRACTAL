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
#define DIM             20
#define WIDTH           800
#define HEIGHT          800
#define SCALE            8
#define FPS             2.0

ALLEGRO_DISPLAY * display;
ALLEGRO_EVENT_QUEUE* queue;
ALLEGRO_TIMER* timer;

bool swapRule = false;

static inline bool parity(int neighborhood) { return neighborhood % 2 != 0; }

static int getNeighborhood(const int &i, const int &j, const bool* fractal, const bool* topFractal, const bool* downFractal, int& rank, int& num_thread) {

    auto Neighborhood = 0;

    if(i == 0) {

        if( j - 1 > 0)
            if(topFractal[j-1])
                Neighborhood++;

        if( j + 1 < DIM)
            if(topFractal[j+1])
                Neighborhood++;

        if( i + 1 < DIM && j + 1 < DIM )
            if(fractal[(i+1) * DIM + (j+1)])
                Neighborhood++;

        if( i + 1 < DIM && j - 1 >= 0 )
            if(fractal[(i+1) * DIM + (j-1)])
                Neighborhood++;

    }

    else if(i == DIM-1 ) {

        if( j - 1 > 0)
            if(downFractal[j-1])
                Neighborhood++;

        if( j + 1 < DIM)
            if(downFractal[j+1])
                Neighborhood++;

        if( i - 1 < DIM && j + 1 < DIM )
            if(fractal[(i-1) * DIM + (j+1)])
                Neighborhood++;

        if( i - 1 < DIM && j - 1 >= 0 )
            if(fractal[(i-1) * DIM + (j-1)])
                Neighborhood++;

    }

    else {

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

     }

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

            if(!fractal[i * DIM + j])
                continue;

            al_draw_filled_rectangle(i * SCALE, j * SCALE, i * SCALE + SCALE, j * SCALE + SCALE, al_map_rgb(147,112,219));

        }

    al_draw_textf(al_create_builtin_font(), al_map_rgb(255,255,255) , 730 , 780 , ALLEGRO_ALIGN_CENTRE, "Generation: %d" , generation);
    al_draw_textf(al_create_builtin_font(), al_map_rgb(255,255,255) , 724 , 10 , ALLEGRO_ALIGN_CENTRE, "Press P to pause" );
    al_draw_textf(al_create_builtin_font(), al_map_rgb(255,255,255) , 700 , 20 , ALLEGRO_ALIGN_CENTRE, "Press R to change rule" );
    al_flip_display();
}

void step(bool* subFractal, bool* subSupport, bool* topRow, bool* downRow, int& localDIM, int& rank, int& num_thread){

    for (auto i = 0; i < localDIM; ++i)
        for (auto j = 0; j < DIM; ++j) {

            auto Neighborhood = getNeighborhood(i, j, subSupport, topRow, downRow, rank, num_thread);

            auto oldSelf = subSupport[i * DIM + j];
            auto newSelf = (2 + (parity(Neighborhood) - oldSelf)) % 2;

            subFractal[i * DIM + j] = newSelf;

        }
}

void sendHalo(int& rank, int& num_thread , int& localDIM, bool* subFractal, bool* topRow, bool* downRow)
{
    MPI_Status status;

    if (rank == 0)
    {
        MPI_Send(&subFractal[(localDIM) * DIM - DIM], DIM, MPI_C_BOOL, 1, 0, MPI_COMM_WORLD);
        MPI_Send(&subFractal[0], DIM, MPI_C_BOOL, num_thread  - 1, 0, MPI_COMM_WORLD);

        MPI_Recv(downRow, DIM, MPI_C_BOOL, 1, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(topRow, DIM, MPI_C_BOOL, num_thread  - 1, 0, MPI_COMM_WORLD, &status);
    }
    else if (rank == num_thread  - 1)
    {
        MPI_Send(&subFractal[(localDIM) * DIM - DIM], DIM, MPI_C_BOOL, 0, 0, MPI_COMM_WORLD);
        MPI_Send(&subFractal[0], DIM, MPI_C_BOOL, rank - 1, 0, MPI_COMM_WORLD);

        MPI_Recv(downRow, DIM, MPI_C_BOOL, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(topRow, DIM, MPI_C_BOOL, rank - 1, 0, MPI_COMM_WORLD, &status);
    }
    else
    {
        MPI_Send(&subFractal[(localDIM) * DIM - DIM], DIM, MPI_C_BOOL, rank + 1, 0, MPI_COMM_WORLD);
        MPI_Send(&subFractal[0], DIM, MPI_C_BOOL, rank - 1, 0, MPI_COMM_WORLD);

        MPI_Recv(topRow, DIM, MPI_C_BOOL, rank - 1, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(downRow, DIM, MPI_C_BOOL, rank + 1, 0, MPI_COMM_WORLD, &status);
    }
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


    bool *fractal = (bool*) calloc(1,DIM * DIM * sizeof(bool));

    for (auto i = 0; i < DIM ; ++i)
        for (auto j = 0; j < DIM; ++j)
            fractal[i * DIM + j] = false;

    bool * subFractal = (bool*) calloc(1,DIM * localDIM  * sizeof(bool));
    bool * subSupport = (bool*) calloc(1,DIM * localDIM  * sizeof(bool));

    bool * topRow = (bool*) calloc(1, DIM * sizeof(bool));
    bool * downRow = (bool*) calloc(1,DIM * sizeof(bool));


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


        fractal[(DIM/2) * DIM + (DIM/2)] = true;
//        fractal[(DIM/2+1) * DIM + (DIM/2+1)] = true;
//        fractal[(DIM/2-1) * DIM + (DIM/2-1)] = true;
    }



    MPI_Scatter (fractal, DIM * localDIM , MPI_C_BOOL, subFractal,DIM * localDIM ,MPI_C_BOOL, MASTER, MPI_COMM_WORLD);

    int topGhost = rank == 0 ? num_thread - 1 : rank - 1;
    int downGhost = rank == num_thread - 1 ? 0 : rank + 1;


    for(auto it = 0; it < 2 && isRunning; ++it) {


            MPI_Send(&subFractal[0], DIM, MPI_C_BOOL, topGhost, 1, MPI_COMM_WORLD);
            MPI_Send(&subFractal[(localDIM) * DIM - DIM], DIM, MPI_C_BOOL, downGhost, 2, MPI_COMM_WORLD);

            MPI_Recv(downRow, DIM, MPI_C_BOOL, downGhost, 1, MPI_COMM_WORLD, &status);
            MPI_Recv(topRow, DIM, MPI_C_BOOL, topGhost, 2, MPI_COMM_WORLD, &status);

        //sendHalo(rank, num_thread , localDIM, subFractal, topRow, downRow);

        for (auto i = 0; i < localDIM; ++i)
            for (auto j = 0; j < DIM; ++j)
                subSupport[i * DIM + j] = subFractal[i * DIM + j];


        if (rank == 1) {

            std::cerr<<"********init****************"<<"   ----------> "<<rank<<std::endl;

            for (auto j = 0; j < DIM; ++j)
                std::cerr<<topRow[j]<<" ";
            std::cerr<<std::endl;
            for (auto i = 0; i < localDIM; ++i){
                for (auto j = 0; j < DIM; ++j)
                    std::cerr<<subFractal[i * DIM + j]<<" ";
                std::cerr<<std::endl;}
            for (auto j = 0; j < DIM; ++j)
                std::cerr<<downRow[j]<<" ";
            std::cerr<<std::endl;

            std::cerr<<std::endl;
        }

        if (rank == 1) {

            std::cerr<<"********VECCHIA****************"<<std::endl;

            for (auto j = 0; j < DIM; ++j)
                std::cerr<<topRow[j]<<" ";
            std::cerr<<std::endl;
            for (auto i = 0; i < localDIM; ++i){
                for (auto j = 0; j < DIM; ++j)
                    std::cerr<<subSupport[i * DIM + j]<<" ";
                std::cerr<<std::endl;}
            for (auto j = 0; j < DIM; ++j)
                std::cerr<<downRow[j]<<" ";
            std::cerr<<std::endl;

            std::cerr<<std::endl;
        }



        step(subFractal, subSupport, topRow, downRow, localDIM, rank, num_thread);


        if (rank == 1) {

            std::cerr<<"********VECCHIA****************"<<std::endl;

            for (auto j = 0; j < DIM; ++j)
                std::cerr<<topRow[j]<<" ";
            std::cerr<<std::endl;
            for (auto i = 0; i < localDIM; ++i){
                for (auto j = 0; j < DIM; ++j)
                    std::cerr<<subSupport[i * DIM + j]<<" ";
                std::cerr<<std::endl;}
            for (auto j = 0; j < DIM; ++j)
                std::cerr<<downRow[j]<<" ";
            std::cerr<<std::endl;

            std::cerr<<std::endl;
        }

        if (rank == 1) {

            std::cerr<<"********NUOVA*****************"<<std::endl;

            for (auto j = 0; j < DIM; ++j)
                std::cerr<<topRow[j]<<" ";
            std::cerr<<std::endl;
            for (auto i = 0; i < localDIM; ++i){
                for (auto j = 0; j < DIM; ++j)
                    std::cerr<<subFractal[i * DIM + j]<<" ";
                std::cerr<<std::endl;
            }
            for (auto j = 0; j < DIM; ++j)
                std::cerr<<downRow[j]<<" ";
            std::cerr<<std::endl;

            std::cerr<<std::endl;
        }

        MPI_Gather(subFractal, DIM * (localDIM), MPI_C_BOOL, fractal, DIM * (localDIM), MPI_C_BOOL, MASTER, MPI_COMM_WORLD);

        if (rank == MASTER) {

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

        al_rest(1);
    }


    MPI_Barrier(MPI_COMM_WORLD);

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