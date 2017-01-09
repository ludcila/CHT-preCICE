# Dockerfiles

Commands to be executed from the parent directory (`../`):

    docker build -t precice -f docker/precice.Dockerfile .
    docker build -t solvers -f docker/solvers.Dockerfile .
    docker build -t adapters -f docker/adapters.Dockerfile .
