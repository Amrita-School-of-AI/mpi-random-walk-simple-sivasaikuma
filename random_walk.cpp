#include <iostream>
#include <cstdlib> // For atoi, rand, srand
#include <ctime>   // For time
#include <mpi.h>

void walker_process();
void controller_process();

int domain_size;
int max_steps;
int world_rank;
int world_size;

int main(int argc, char **argv)
{
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the number of processes and the rank of this process
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if (argc != 3)
    {
        if (world_rank == 0)
        {
            std::cerr << "Usage: mpirun -np <p> " << argv[0] << " <domain_size> <max_steps>" << std::endl;
        }
        MPI_Finalize();
        return 1;
    }

    domain_size = atoi(argv[1]);
    max_steps = atoi(argv[2]);

    if (world_rank == 0)
    {
        // Rank 0 is the controller
        controller_process();
    }
    else
    {
        // All other ranks are walkers
        walker_process();
    }

    // Finalize the MPI environment
    MPI_Finalize();
    return 0;
}

void walker_process()
{
    // Seed the random number generator.
    srand(time(NULL) + world_rank);

    int position = 0;

    for (int step = 1; step <= max_steps; ++step)
    {
        // Random move: -1 or +1
        int move = (rand() % 2 == 0) ? -1 : 1;
        position += move;

        // Check if walker is out of bounds
        if (position < -domain_size || position > domain_size)
        {
            std::cout << "Rank " << world_rank << ": Walker finished in " << step << " steps." << std::endl;
            int done = step;
            MPI_Send(&done, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            break;
        }

        // If reached max_steps without exiting, still finish
        if (step == max_steps)
        {
            std::cout << "Rank " << world_rank << ": Walker finished in " << step << " steps." << std::endl;
            int done = step;
            MPI_Send(&done, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    }
}

void controller_process()
{
    int finished_count = 0;
    int expected = world_size - 1;

    while (finished_count < expected)
    {
        int msg;
        MPI_Status status;
        MPI_Recv(&msg, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        finished_count++;
    }

    std::cout << "Controller: All " << (world_size - 1) << " walkers have finished." << std::endl;
}