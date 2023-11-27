#include <functional>
#include <iostream>
#include <sstream>
#include <sys/_types/_socklen_t.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#define PORT 8080

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
    int newSocket = accept(server, (struct sockaddr*) &address, &addrlen);
    std::cout << "wow a new connection" << std::endl;
    
    char line[1024];
    std::string name;
    name.resize(1024);
    read(newSocket, &name[0], 1024-1);
    while(true){
        read(newSocket, line, 1024 - 1);
        std::cout << "sending: " << line << std::endl;
        std::stringstream toSend;
        toSend << &name[0] << ": " << line;
        send(newSocket, toSend.str().c_str(), strlen(toSend.str().c_str()), 0);
        memset(line, 0, sizeof(line));
    }

    close(newSocket);
    close(server);
    return 0;
}
