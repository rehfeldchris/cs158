gcc p3e-server.c -lpthread -o server
gcc p3e-client.c -o client
./server &
sleep 1
./client | tee client1.txt &
./client | tee client2.txt &
wait