
### Server and Client chat
* This chat.cpp is a source code of simple chat server and client.

* To compile the code, simply do "make" in the terminal under the folder of makefile and source code.

* The generated executable will be called "chat".


Usage: 
```
./chat [OPTION...]
  -s=SERVER_IP	Specify the IP of the server to connect
  -p=Port_Num	Specify the port number of the server
  -h		    Display help message
```

Start a server first, simply type ***./chat***, then the server will print out the port number,
Like this:

>Waiting for a connection on port 13038


use the **port number** to launch the client.


To start a client, you must use both ***-s*** and ***-p*** flags
```
$./chat -s 127.0.0.1 -p 13038
```

Use ***^c*** to terminate both client and server program.
