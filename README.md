# Non blocking multi-threaded sockets
This PoC project was motivated by some work we were doing with the libmodbus-dev library. 

We were creating a server which was doing listen / accept in a thread, and wanting to cleanly shut it down. Once the `accept` call is reached, from what I can
tell, it's impossible to get it to unblock until an actual connection is made. 

This differs from java, and other high level languages in that, typically you can simply close the socket and it will release the block. This is not the case
with c++/c sockets.

At first, I though it might be just a libmodbus problem, but I built another PoC using just simple tcp sockets and found it was the same.

This PoC shows how it can be cleanly shutdown using both tcp and modbus sockets.

## self pipe, select approach
There are a few approaches which could be taken, however this one uses low cpu (doesn't poll, doesn't require timeouts).

The approach I'm using is to set the sockets into non-blocking mode, and use `SELECT` to monitor groups of file descriptors which are ready to
read (accept) and write. 

A self-pipe is used, and added to the read descriptors. A signal handler is registered to the `SIGUSR1` signal. When this signal is received, it
writes to the self-pipe. This triggers the `SELECT` since it is waiting on a read from the pipe + the server socket. It then breaks out of its
listening loop and cleanly closes.

A `stop` call can used to raise the SIGUSR1 signal to kick this off from another thread.
