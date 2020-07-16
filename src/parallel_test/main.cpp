//
// Created by not_tyche on 17/07/20.
//

#include <mpi.h>
#include <allegro5/allegro.h>
#include <malloc.h>

#define PRIMARY           0
#define DIM             100
#define FPS             24.0

inline int getUpper(const int& rank, const int& num_thread) { return rank == 0 ? num_thread - 1 : rank - 1; }

inline int getLower(const int& rank, const int& num_thread) { return rank == num_thread - 1 ? 0 : rank + 1; }

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

void step(int* plane, int * planeSupport, int* topRow, int* downRow, int& localDIM, int& rank, int& num_thread, int& it){

    // ANNOTATION:
    // mem                       -----> first bit of right - shift of current self
    // Neighborhood              -----> sum of ( NE + NO + SE + SO ) of current self and mem
    // newSelf                   -----> *parity of Neighborhood
    // planeSupport[i * DIM + j] -----> first bit of left - shift of plane[i * DIM + j] or newSelf
    // * true if is odd

    for (auto i = 0; i < localDIM; ++i)
        for (auto j = 0; j < DIM; ++j) {

            auto mem = (plane[i * DIM + j] >> 1) & 1;
            auto Neighborhood = getNeighborhood(i, j, plane, topRow, downRow, rank, num_thread, localDIM) + mem;

            auto newSelf = parity(Neighborhood);

            planeSupport[i * DIM + j] = ( (plane[i * DIM + j] & 1) << 1 ) | newSelf;

        }

    memcpy(plane, planeSupport, DIM * localDIM * sizeof(int));
}


int main(int argc, char *argv[]) {

    int num_thread, rank;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_thread);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    double start = 0.0 , fine;

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

    auto upper = getUpper(rank, num_thread);
    auto lower = getLower(rank, num_thread);

    if(rank == PRIMARY){
        fractal[(DIM/2) * DIM + (DIM/2)] = 1;
        start = MPI_Wtime();
    }


    MPI_Scatter (fractal, DIM * localDIM , MPI_INT, plane,DIM * localDIM ,MPI_INT, PRIMARY, MPI_COMM_WORLD);


    for(auto it = 0; it < 10000; ++it) {


        MPI_Send(&plane[0], DIM, MPI_INT, upper, 1, MPI_COMM_WORLD);
        MPI_Send(&plane[(localDIM) * DIM - DIM], DIM, MPI_INT, lower, 2, MPI_COMM_WORLD);

        MPI_Recv(downRow, DIM, MPI_INT, lower, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(topRow, DIM, MPI_INT, upper, 2, MPI_COMM_WORLD, &status);

        step(plane, planeSupport, topRow, downRow, localDIM, rank, num_thread, it);

        MPI_Gather(plane, DIM * (localDIM), MPI_INT, fractal, DIM * (localDIM), MPI_INT, PRIMARY, MPI_COMM_WORLD);

    }


    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == PRIMARY){
        fine=MPI_Wtime();
        std::cerr<<"Parallel execution time: "<<fine-start<<std::endl;
    }

    MPI_Finalize();

    return 0;
}