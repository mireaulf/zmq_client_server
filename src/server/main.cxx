#include <iostream>
#include <cppzmq/zmq.hpp>
#include <cppzmq/zmq_addon.hpp>
#include <thread>
#include <atomic>
#include <chrono>

zmq::context_t ctx;
std::atomic_bool stop = false;


void input_thread_fn() {
    char c;
    while(!stop) {
        std::cin >> c;
        stop = c == 'q';
    }
    std::cout << "stoping input thread" << std::endl;
    zmq::socket_t sock(ctx, zmq::socket_type::router);
    sock.connect("tcp://localhost:5556");
    sock.send("quit", 5);
    //std::cout << "sending quit packet to self" << std::endl;
    /*zmq::message_t msg;
    sock.recv(&msg);*/
    std::cout << "input thread stoped" << std::endl;
    //std::cout << msg.data<char>() << std::endl;
}

using namespace std::chrono_literals;

void foobar() {
    zmq::context_t recv_ctx(1);
    zmq::socket_t recv(recv_ctx, zmq::socket_type::rep);
    recv.bind("tcp://*:5556");
    //zmq::context_t send_ctx;
    zmq::socket_t send(recv_ctx, zmq::socket_type::req);
    send.connect("tcp://locahost:5556");
    
    // sending
    std::string identity("client");
    std::cout << "message sent: " << send.send(zmq::message_t(identity.c_str(), identity.length())) << std::endl;
    /*
    std::string zero("");
    zmq::multipart_t send_msg;
    send.setsockopt(ZMQ_IDENTITY, "client");
    send_msg.addstr(identity);
    send_msg.add(zmq::message_t(zero.c_str(), zero.length())); // empty
    send_msg.addstr("hello world!");
    std::cout << "message sent: " << send_msg.send(send) << std::endl;
    */

    // receive
    //zmq::multipart_t recv_msg;
    //std::cout << "message received: " << recv_msg.recv(recv) << std::endl;
    zmq::message_t recv_msg;
    std::cout << "message received: " << recv.recv(&recv_msg) << std::endl;
    
    std::cout << "done..." << std::endl;
}




int main(int argc, char** argv) { 
    //foobar();


    if(argc >= 2) {
        char type = argv[1][0];
        std::cout << "Type: ";
        if(type == 's') {
            std::cout << "server" << std::endl;
            zmq::socket_t sock(ctx, zmq::socket_type::router);
            sock.bind("tcp://*:5556");
            while(!stop) {
                zmq::multipart_t msg;
                if(msg.recv(sock, ZMQ_DONTWAIT)) {
                    
                    std::string msg_ident(msg.popstr());
                    std::string msg_null(msg.popstr());
                    std::string msg_content(msg.popstr());

                    std::cout << (msg.empty() ? "nothing to see here" : "YES!") << std::endl;
                }

                std::cout << "sleeping..." << std::endl;
                std::this_thread::sleep_for(5s);
            }
            std::cout << "SHUT DOWN." << std::endl;
        } else if(type == 'c') {
            std::cout << "client" << std::endl;

            zmq::socket_t sock(ctx, zmq::socket_type::req);
            std::string identity("foobar");
            sock.setsockopt(ZMQ_IDENTITY, identity.c_str(), identity.length() + 1);
            sock.connect("tcp://localhost:5556");
            if(sock.connected()) {
                std::cout << "connected." << std::endl;
                bool stop = false;
                std::string line;
                do {
                    std::cin >> line;
                    stop = line == "q";
                    if(stop) {
                        std::cout << "quitting." << std::endl;
                        line = "quitting";
                    }
                    std::cout << "sent: '" << line << "'" << std::endl;
                    //zmq::multipart_t msg;
                    //msg.addstr(identity);
                    //msg.add(zmq::message_t()); // empty
                    //msg.addstr(line);
                    //std::cout << "message sent: " << msg.send(sock) << std::endl;
                    zmq::message_t msg(line.c_str(), line.length());
                    std::cout << "message sent: " << sock.send(msg) << std::endl;
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
