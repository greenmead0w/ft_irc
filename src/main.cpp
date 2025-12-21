#include "Server.hpp"
#include <iostream>
#include <signal.h>
#include <cstdlib> //for atoi

//Global boolean for signal handling (allowed ?) 
//Signal allows the poll loop in Server.cpp to stop gracefully.
bool g_stop = false;

void signalHandler(int signum) {
    (void)signum;
    std::cout << "\n[Signal received. Shutting down server...]" << std::endl;
    g_stop = true;
}

int main(int argc, char **argv) {
    //proper argument check
    if (argc != 3) {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }

    //Port Validation: 0 - 1023 are reserved for system services
    //65535 is the max port limit: port field is 16 bit long so: 2^16 - 1 == 65535
    int port = std::atoi(argv[1]);
    if (port < 1024 || port > 65535) {
        std::cerr << "Error: Invalid port. Use 1024-65535." << std::endl;
        return 1;
    }

    //Signal setup: important for freeing memory and closing sockets properly on Ctrl+C
    //SIGINT --> "Signal interrupt" == "ctrl + c" (kills program instantly)
    //SIGQUIT --> "Signal quit" == "ctrl + \"" (more aggressive than SIGINT)
    signal(SIGINT, signalHandler);
    signal(SIGQUIT, signalHandler);

    //init server
    try {
        Server irc(port, argv[2]);
        std::cout << "--- Launching ft_irc on port " << port << " ---" << std::endl;
        
        irc.init();   //To be done on Phase 3: Sockets, Bind, Listen
        irc.run();    //To be done on Phase 4: The poll() loop
    }
    catch (const std::exception &e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}