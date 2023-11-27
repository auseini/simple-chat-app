#include <iostream>
#include <sys/socket.h>
#include <string>
#include <thread>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080

void readFromSocket(int socket);
void writeToSocket(int socket);


int main() {
    std::string userName;
    std::getline(std::cin, userName);
    std::cout << userName << std::endl;
    char buffer[1024] = {0};
    int client = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serverAddress;  
    if(client < 0) {
        std::cerr << "failed to open socket" << std::endl;;
        return 1;
    }
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);

    if(inet_pton(AF_INET,"127.0.0.1", &serverAddress.sin_addr) <= 0){
        std::cerr << "address is invalid" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Connecting to server..." << std::endl;
    if(connect(client, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0){
        std::cerr << "failed to connect to server" << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Connected." << std::endl;
    send(client, userName.c_str(), strlen(userName.c_str()) ,0); 

    std::thread readThread(readFromSocket, client);
    std::thread writeThread(writeToSocket, client);

    readThread.join();
    writeThread.join();
    // bind my socket
    //
    // create send and recieve threads
    //
    close(client);

     return 0;
}


void readFromSocket(int socket){
    char line[1024] = {0};
    while(true){
        read(socket, line, 1024 - 1);
        std::cout << line << std::endl;
        memset(line, 0, sizeof(line));
    }
}
void writeToSocket(int socket){
    std::string line;
    while(true){
        getline(std::cin, line);
        send(socket, line.c_str(), strlen(line.c_str()), 0);
        std::cin.clear();
        std::cout << "\033[A";
    }
}
