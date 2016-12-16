# Remove stopped containers
docker rm $(docker ps -a -q)

# Remove dangling images
docker rmi $(docker images -q -f dangling=true)
