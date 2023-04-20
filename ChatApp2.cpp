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
        std::string message(buf, bytesReceived);

        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl;
            break;
        }
        else if (bytesReceived == 0 || string(buf) == "exit") {
            std::cout << "Connection closed" << std::endl;
            break;
        }

       
        else if (message.substr(0, 9) == "transfer ")
        {
            string fileName = message.substr(9);
            string newFileName = "new" + fileName;

            ofstream file(newFileName, ios::binary);

            if (!file.is_open())
            {
                cerr << "Error: Could not create file " << newFileName << endl;
            }

            else {
                std::cout << "Receiving file " << fileName << endl;

                int bytesRec;
                char buffer[FILEBUFSIZE];

                do {
                    bytesRec = recv(sock, buffer, FILEBUFSIZE, 0);
                    if (bytesRec > 0) {
                        file.write(buffer, bytesRec);
                    }
                    else if (bytesRec == 0) {
                        std::cout << "Connection closed or file not present" << endl;
                    }
                    else {
                        cerr << "recv failed: " << WSAGetLastError() << endl;
                    }
                } while (bytesRec > 0);

                file.close();

                cout << "File transaction closed" << endl;
            }
        }

        else {
            std::cout << "Received: " << std::string(buf, 0, bytesReceived) << std::endl;
        }
    }
}

void writeThread(SOCKET sock) {
    std::string input;
    int bytesSent;
    do {
        std::getline(std::cin, input);
        bytesSent = send(sock, input.c_str(), input.size() + 1, 0);

        if (input.substr(0, 9) == "transfer ") {
            string fileName = input.substr(9);
            ifstream file(fileName, ios::binary);

            if (!file.is_open()) {
                cerr << "Error: Could not open file " << fileName << endl;
            }

            else
            {
                std::cout << "Sending file " << fileName << endl;

                char buffer[FILEBUFSIZE];
                int bytesR, bytesS;

                while (!file.eof()) {
                    file.read(buffer, FILEBUFSIZE);
                    bytesR = file.gcount();

                    bytesS = send(sock, buffer, bytesR, 0);

                    if (bytesS == SOCKET_ERROR) {
                        cerr << "send failed: " << WSAGetLastError() << endl;
                        file.close();
                    }

                    if (bytesS != bytesR) {
                        cerr << "Error: partial send, could not send everything." << endl;
                        file.close();
                    }
                }

                std::cout << "File sent successfully" << endl;
                file.close();
            }
        }


        else if (bytesSent == SOCKET_ERROR) {
            std::cerr << "send failed with error: " << WSAGetLastError() << std::endl;
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
        std::cerr << "WSAStartup failed with error: " << iResult << std::endl;
        return 1;
    }


    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }


    sockaddr_in serverAddress = { 0 };
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(0);
    iResult = bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress));
    if (iResult == SOCKET_ERROR) {
        std::cerr << "Failed to bind socket: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    iResult = listen(serverSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "listen failed with error: " << WSAGetLastError() << std::endl;
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
        std::cerr << "Can't create socket, Err #" << WSAGetLastError() << std::endl;
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
        std::cerr << "Can't connect to the other server, Err #" << WSAGetLastError() << std::endl;
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

    std::thread t1(readThread, clientSocket);
    std::thread t2(writeThread, otherSock);

    t1.join();
    t2.join();

    closesocket(serverSocket);
    closesocket(clientSocket);
    closesocket(otherSock);
    WSACleanup();

    return 0;
}

