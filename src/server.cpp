#include <functional>
#include <iostream>
#include <sstream>
#include <sys/_types/_socklen_t.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <unordered_set>
#include <mutex>
#define PORT 8080

void createUser(const std::string& userName, std::vector<int>& sockets, const int socket);
void readFromUser(const std::string& userName, std::vector<int>& sockets, const int socket);
void writeToSockets(const std::vector<int>& sockets);
void sendToSocket(const int socket);

std::vector<std::thread> readerThreads;
std::vector<std::string> messages;
std::mutex messageQueueLock;
std::mutex socketLock;
int main(){
    int server;
    std::cout << "i am ther server" << std::endl;

    server = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    int opt = 1;

    if(server < 0){
        std::cerr << "failed to create socket" << std::endl;
        return 1;
    }
   
  //  if(setsockopt(server, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &opt, sizeof(opt))){
  //      std::cerr << "failed to set socket options" << std::endl;
  //      return EXIT_FAILURE;
  //  }
    
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);

    if(bind(server,(struct sockaddr*)&address, addrlen) < 0){
        std::cerr << "failed to bind socket" << std::endl;
        return EXIT_FAILURE;
    }

    if(listen(server,10) < 0 ){
        std::cerr << "listen failure" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "done listening" << std::endl;

    //loop forever, and check for a new connection
    //accept connection, spawn nee thread with that 
    std::vector<int> sockets;
    std::thread writer(writeToSockets, std::ref(sockets));
    while(true){ 
        int newSocket = accept(server, (struct sockaddr*) &address, &addrlen);
        std::cout << "wow a new connection" << std::endl;
        char line[1024];
        std::string name;
        name.resize(1024);
        read(newSocket, &name[0], 1024-1);
        
        createUser(name, sockets, newSocket);
    }
    
    for (auto & thread : readerThreads){
        thread.join();
    }
    writer.join();
    for (int socket : sockets){
        close(socket);
    }
    close(server);
    return 0;
}

void writeToSockets(const std::vector<int>& sockets){
    while (true) {
        if(messages.size() == 0) continue;

        std::vector<std::thread> writers;
        messageQueueLock.lock();
        for(const int socket : sockets){
            writers.push_back(std::thread(sendToSocket, socket));
        }
        for(auto& writer: writers){
            writer.join();
        }
        messages.clear();
        messageQueueLock.unlock();
    }
}

void sendToSocket(const int socket){
        for(std::string& message : messages){
            send(socket, message.c_str(), 1024 - 1, 0); 
        }
}

void createUser(const std::string& userName, std::vector<int>& sockets, const int socket){
    socketLock.lock();
    sockets.push_back(socket);
    readerThreads.push_back(std::thread(readFromUser, std::ref(userName), std::ref(sockets), socket));
    socketLock.unlock();
}


void readFromUser(const std::string& userName, std::vector<int>& sockets, const int socket){
        while (true){
            char line[1024];
            read(socket, line, 1024 - 1);
            std::cout << "sending: " << line << std::endl;
            auto now = std::chrono::system_clock::now();
            std::time_t t= std::chrono::system_clock::to_time_t(now); 
            std::stringstream toSend;
            toSend << t << ' ' << &userName[0] << ": " << line;
     //       send(socket, toSend.str().c_str(), strlen(toSend.str().c_str()), 0);
            memset(line, 0, sizeof(line));

            messageQueueLock.lock();
            messages.push_back(toSend.str());
            messageQueueLock.unlock(); 
           
        }
}
