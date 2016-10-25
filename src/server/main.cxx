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
                auto str = message.data<char>();
                std::cout << "received: " << str << std::endl;
                sock.send("ok", 3);
                stop = str[0] == 'q';
            }
            std::cout << "SHUT DOWN." << std::endl;
        } else if(type == 'c') {
            std::cout << "client" << std::endl;
            zmq::socket_t sock(ctx, zmq::socket_type::req);
            sock.connect(argv[2]);
            if(sock.connected()) {
                std::cout << "connected." << std::endl;
                std::string str(argv[3]);
                //size_t str_size = str.length() + 1;
                zmq::message_t hello(str.c_str(), str.length() + 1);
                sock.send(hello);
            }
        } else {
            std::cout << "unknown" << std::endl;
        }
    } else {
        std::cout << "Nothing happened" << std::endl;
    }
    return 0;
}
