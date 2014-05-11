gcc p3e-server.c -lpthread -o server
gcc p3e-client.c -o client
./server &
./client &
./client &
wait