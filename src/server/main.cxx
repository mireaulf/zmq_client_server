#include <iostream>
#include <cppzmq/zmq.hpp>

int main(int argc, char** argv) {
    if(argc >= 2) {
        zmq::context_t ctx;
        char type = argv[1][0];
        std::cout << "Type: ";
        if(type == 's') {
            std::cout << "server" << std::endl;
            zmq::socket_t sock(ctx, zmq::socket_type::rep);
            sock.bind("tcp://*:5556");
            bool stop = false;
            while(!stop) {
                zmq::message_t message;
                sock.recv(&message);
                std::string str(message.data<char>());
                if(str == "quit") {
                    std::cout << "client disconnected." << std::endl;
                } else {
                    std::cout << "received: " << str << std::endl;
                }
                sock.send("ok", 3);
            }
            std::cout << "SHUT DOWN." << std::endl;
        } else if(type == 'c') {
            std::cout << "client" << std::endl;
            zmq::socket_t sock(ctx, zmq::socket_type::req);
            sock.connect(argv[2]);
            if(sock.connected()) {
                std::cout << "connected." << std::endl;
                bool stop = false;
                std::string line;
                do {
                    std::cin >> line;
                    stop = line == "quit";
                    if(stop) {
                        std::cout << "quitting." << std::endl;
                        line = "quitting";
                    }
                    std::cout << "sent: '" << line << "'" << std::endl;
                    zmq::message_t message(line.c_str(), line.length() + 1);
                    sock.send(message);
                    zmq::message_t rcv;
                    sock.recv(&rcv);
                } while(!stop);
            }
        } else {
            std::cout << "unknown" << std::endl;
        }
    } else {
        std::cout << "Nothing happened" << std::endl;
    }
    return 0;
}
