#include <iostream>
#include <sys/socket.h>
#include <string>
#include <thread>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <iomanip>
#include <sstream>
#include <mutex>

#define PORT 8080

void readFromSocket(int socket);
void writeToSocket(int socket);
std::string formatMessage(const std::string& message);
std::string getDateString(const std::string& timestamp);
bool shouldClose = false;
std::mutex lock;

int main() {
    std::string userName;
    std::cout << "Enter a username: ";
    std::getline(std::cin, userName);
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
    close(client);

    return 0;
}


void readFromSocket(int socket){
    char line[1024] = {0};
    while(!shouldClose){
        read(socket, line, 1024 - 1);
        if(strcmp(line, "exit") == 0){
            break;
        }
        std::cout << formatMessage(line) << std::endl;
        memset(line, 0, sizeof(line));
    }
}
void writeToSocket(int socket){
    std::string line;
    while(!shouldClose){
        getline(std::cin, line);
        send(socket, line.c_str(), strlen(line.c_str()), 0);
        std::cin.clear();
        std::cout << "\033[A";
        if(line.compare("exit") == 0) {
            shouldClose = true;
            continue;
        }
    }
}
std::string formatMessage(const std::string& message){
    int pos = message.find(' ');
    
    std::string newMessage = message.substr(pos);
    std::string timestamp = message.substr(0, pos);
    std::string dateString = getDateString(timestamp);
    dateString.erase(std::remove(dateString.begin(), dateString.end(), '\n'), dateString.end());
    std::stringstream ss;
    ss<< dateString << newMessage;
    return ss.str();
}

std::string getDateString(const std::string& timestamp){
    time_t t(atoll(timestamp.c_str()));
    auto s = localtime(&t);
    return asctime(s);
}
