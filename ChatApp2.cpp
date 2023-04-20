#include <iostream>
#include <thread>
#include <string>
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>
#include <fstream>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

const int FILEBUFSIZE = 1024;
const int STRINGBUFSIZE = 4096;

void readThread(SOCKET sock) {
    char buf[STRINGBUFSIZE];
    int bytesReceived;

    while (true) {
        bytesReceived = recv(sock, buf, STRINGBUFSIZE, 0);
        string message(buf, bytesReceived);

        if (bytesReceived == SOCKET_ERROR) {
            cerr << "recv failed with error: " << WSAGetLastError() << endl;
            break;
        }
        else if (string(buf) == "exit") {
            break;
        }

        else if (message.substr(0, 9) == "transfer ")
        {
            string fileName = message.substr(9);
            string newFileName = "new" + fileName;

            ofstream file(newFileName, ios::binary);

            if (!file.is_open()) {
                cerr << "Error: Could not create file " << newFileName << endl;
            }

            else {
                cout << "Receiving file " << fileName << endl;

                int bytesR;
                char buffer[FILEBUFSIZE];

                do {
                    bytesR = recv(sock, buffer, FILEBUFSIZE, 0);
                    if (string(buffer) == "&8e913") {
                        cout << "Receive complete" << endl;
                        break;
                    }

                    else if (bytesR > 0) {
                        file.write(buffer, bytesR);
                    }
                    else if (bytesR == 0) {
                        cout << "Connection closed or file not present" << endl;
                        break;
                    }
                    else {
                        cerr << "recv failed: " << WSAGetLastError() << endl;
                        break;
                    }
                } while (bytesR > 0);

                cout << "File transaction closed" << endl;
            }

            file.close();
        }

        else {
            cout << "Received: " << string(buf, 0, bytesReceived) << endl;
        }
    }
}

void writeThread(SOCKET sock) {
    string input;
    int bytesSent;
    do {
        getline(cin, input);
        bytesSent = send(sock, input.c_str(), input.size() + 1, 0);

        if (input.substr(0, 9) == "transfer ") {
            string fileName = input.substr(9);
            ifstream file(fileName, ios::binary);

            if (!file.is_open()) {
                cerr << "Error: Could not open file " << fileName << endl;
            }

            else
            {
                cout << "Sending file " << fileName << endl;

                char buffer[FILEBUFSIZE];
                int bytesR = 0, bytesS = 0;

                while (!file.eof()) {
                    file.read(buffer, FILEBUFSIZE);
                    bytesR = file.gcount();

                    bytesS = send(sock, buffer, bytesR, 0);

                    if (bytesS == SOCKET_ERROR) {
                        cerr << "send failed: " << WSAGetLastError() << endl;
                        break;
                    }

                    if (bytesS != bytesR) {
                        cerr << "Error: partial send, could not send everything." << endl;
                        break;
                    }
                }
                send(sock, "&8e913", 7, 0);

                if(bytesS == bytesR && bytesS != SOCKET_ERROR) cout << "File sent successfully" << endl;
            }

            file.close();
        }


        else if (bytesSent == SOCKET_ERROR) {
            cerr << "send failed with error: " << WSAGetLastError() << endl;
            break;
        }

        else {
            cout << "sent: " << input << endl;
        }
    } while (input != "exit");
}

int main() {
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cerr << "WSAStartup failed with error: " << iResult << endl;
        return 1;
    }


    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (serverSocket == INVALID_SOCKET) {
        cerr << "Failed to create socket: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }


    sockaddr_in serverAddress = { 0 };
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(0);
    iResult = bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress));
    if (iResult == SOCKET_ERROR) {
        cerr << "Failed to bind socket: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    iResult = listen(serverSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        cerr << "listen failed with error: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "listen successful" << endl; 

    sockaddr_in tempAddress;
    int addressLength = sizeof(tempAddress);
    int portRead = getsockname(serverSocket, (sockaddr*)&tempAddress, &addressLength);

    cout << "server socket created at port: " << ntohs(tempAddress.sin_port) << endl;

    int cport;
    cout << "Type in the client's port number: ";
    cin >> cport;

    SOCKET otherSock = socket(AF_INET, SOCK_STREAM, 0);
    
    if (otherSock == INVALID_SOCKET) {
        cerr << "Can't create socket, Err #" << WSAGetLastError() << endl;
        closesocket(serverSocket);
        closesocket(otherSock);
        WSACleanup();
        return 1;
    }

    // Connect to the other instance's reading thread
    sockaddr_in otherServer;
    otherServer.sin_family = AF_INET;
    otherServer.sin_port = htons(cport);
    inet_pton(AF_INET, "127.0.0.1", &otherServer.sin_addr);

    iResult = connect(otherSock, (sockaddr*)&otherServer, sizeof(otherServer));
    if (iResult == SOCKET_ERROR) {
        cerr << "Can't connect to the other server, Err #" << WSAGetLastError() << endl;
        closesocket(otherSock);
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }


    int serverSocketAddrLen = sizeof(serverAddress);
    
    SOCKET clientSocket;

    while (true) {
        clientSocket = accept(serverSocket, (sockaddr*)&serverAddress, &serverSocketAddrLen);

        if (clientSocket == INVALID_SOCKET) {
            continue;
        }

        else {
            break;
        }
    }
    
    
    cout << "connection to the other server successful" << endl;

    thread t1(readThread, clientSocket);
    thread t2(writeThread, otherSock);

    t1.join();
    t2.join();

    cout << "Connection closed" << endl;

    closesocket(serverSocket);
    closesocket(clientSocket);
    closesocket(otherSock);
    WSACleanup();

    return 0;
}

