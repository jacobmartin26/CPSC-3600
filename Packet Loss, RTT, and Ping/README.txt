//By: Jacob Martin and Timmy Lam

I used execlp() in udping.c, to tie the server and client together,
which can be very picky about compilation and run order,
so please follow these compilation instructions without deviation
to properly run and compile :)

TO COMPILE:
gcc -o client.out -std=gnu99 UDPEchoClient.c DieWithMessage.c AddressUtility.c
gcc -o server.out -std=gnu99 UDPEchoServer.c DieWithMessage.c AddressUtility.c
gcc udping.c -o udping

TO RUN:

    SERVER:
        ./udping followed by any flags
	for example: ./udping -S -p 50000

    CLIENT:
	./udping followed by any flags
	for example: ./udping -c 100 -s 300 -i 0.01 -p 50000 -n localhost

If you're stuck in an infinite loop and the signal doesn't work, make
sure your port numbers match and exit with Ctrl-Z instead
