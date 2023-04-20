# README

This is a C++ program that implements a simple TCP client-server chat application. The program allows two clients to connect to a server and communicate with each other by sending and receiving text messages. Additionally, the program allows clients to transfer files to each other using the "transfer" command.

## Prerequisites
Windows OS
Visual Studio (or other C++ compiler)
Winsock2.h library

## Installation
Clone or download the repository to your local machine
Open the solution file chat_application.sln in Visual Studio
Build the solution to generate the executable file
Run the executable file

## Usage
Run the executable file on the server machine
Note the port number displayed on the console (default is 0, which will assign a random available port number)
Run the executable file on two client machines, specifying the IP address and port number of the server
Enter text messages on the client console to send to the other client
Use the "transfer" command followed by a file name to send a file to the other client (e.g., "transfer example.txt")
Type "exit" to close the connection and exit the program


## Code Explanation

### Header files
The following header files are used in the program:

**iostream**: standard input/output stream library
**thread**: allows the program to run multiple threads simultaneously
**string**: used for manipulating strings in the program
**WS2tcpip.h**: provides Windows Sockets implementation
**WinSock2.h**: provides access to Windows Sockets functions
**Windows.h**: provides access to Windows functions
**fstream**: provides file input/output stream functionality

### Namespaces
The program uses the std namespace for standard library functions and objects.

### Constants
The program defines two constants:

*FILEBUFSIZE*: the buffer size used for transferring files
*STRINGBUFSIZE*: the buffer size used for sending and receiving text messages

***readThread function***
The readThread function is called by the server and runs in a separate thread. It reads incoming data from a client, which can be either a text message or a file transfer. If the received data is a text message, it prints the message to the console. If the received data is a file transfer command, it reads the file sent by the client and saves it to disk.

***writeThread function***
The writeThread function is called by the client and runs in a separate thread. It reads input from the console and sends it to the server. If the input is a file transfer command, it reads the file specified and sends it to the other client.

***main function***
The main function initializes the Winsock library and creates a socket for the server. It then binds the socket to a port and listens for incoming connections. When a connection is established, it creates two threads to handle incoming and outgoing data for the client. The threads run the readThread and writeThread functions, respectively. The main function also handles errors that may occur during socket creation and connection.

## License
This project is licensed under the MIT License - see the LICENSE file for details.