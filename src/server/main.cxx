#include <iostream>
#include <cppzmq/zmq.hpp>
#include <cppzmq/zmq_addon.hpp>
#include <thread>
#include <atomic>
#include <chrono>

std::atomic_bool stop = false;
using namespace std::chrono_literals;
using std::string;

static const string ADDR_BIND("tcp://*:5555");
static const string ADDR_CONNECT("tcp://localhost:5555");

void launch_reqrep_client() {
    //  Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REQ);

    std::cout << "Connecting to hello world server…" << std::endl;
    socket.connect (ADDR_CONNECT);

    //  Do 10 requests, waiting each time for a response
    for (int request_nbr = 0; request_nbr != 10; request_nbr++) {
        zmq::message_t request (5);
        memcpy (request.data (), "Hello", 5);
        std::cout << "Sending Hello " << request_nbr << "…" << std::endl;
        socket.send (request);

        //  Get the reply.
        zmq::message_t reply;
        socket.recv (&reply);
        std::cout << "Received World " << request_nbr << std::endl;
    }
}

void launch_reqrep_server() {
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REP);
    socket.bind (ADDR_BIND);

    while (true) {
        zmq::message_t request;
        //  Wait for next request from client
        socket.recv (&request);
        std::cout << "Received Hello" << std::endl;
        //  Do some 'work'
        std::this_thread::sleep_for(1s);
        //  Send reply back to client
        zmq::message_t reply (5);
        memcpy (reply.data (), "World", 5);
        socket.send (reply);
    }
}

void launch_reqrouter_server() {
    zmq::context_t context;
    zmq::socket_t socket(context, ZMQ_ROUTER);
    socket.bind(ADDR_BIND);
    while(true) {
        zmq::multipart_t msg;
        int64_t has_more = socket.getsockopt<int64_t>(ZMQ_RCVMORE);
        std::cout << "has_more: " << has_more << std::endl;
        while(msg.recv(socket/*, ZMQ_DONTWAIT*/)) {
            std::cout << "message received" << std::endl;
            std::string msg_ident(msg.popstr());
            std::string msg_null(msg.popstr());
            std::string msg_content(msg.popstr());
            std::cout << "[" << msg_ident << "] " << msg_content << std::endl;
        }
        std::cout << "sleeping" << std::endl;
        std::this_thread::sleep_for(2s);
    }
}

void launch_reqrouter_clientex() {
    zmq::context_t context;
    zmq::socket_t socket(context, ZMQ_ROUTER);
    string identity("client");
    socket.connect(ADDR_CONNECT);
    string hello_world("hello world ex!");
    zmq::multipart_t msg;
    msg.addstr(identity.c_str());
    msg.add(zmq::message_t());
    msg.addstr(hello_world.c_str());
    bool sent = msg.send(socket);
    std::cout << "sent: " << sent << std::endl;
}

void launch_reqrouter_client() {
    zmq::context_t context;
    zmq::socket_t socket(context, ZMQ_REQ);
    string identity("client");
    socket.setsockopt(ZMQ_IDENTITY, identity.c_str(), identity.length() + 1);
    socket.connect(ADDR_CONNECT);
    string hello_world("hello world!");
    socket.send(hello_world.c_str(), hello_world.length() + 1);
}

int main(int argc, char** argv) {
    static const string REQ_REP("reqrep");
    static const string REQ_ROUTER("reqrouter");
    string socket_type(argv[1]);
    char pcs_type = argv[2][0];
    std::cout << "socket type: " << socket_type << std::endl
        << "pcs type: " << pcs_type << std::endl << std::endl;
    if(pcs_type == 'c') {
        if(socket_type == REQ_REP) {
            launch_reqrep_client();
        } else if(socket_type == REQ_ROUTER) {
            char client_type = argv[3][0];
            if(client_type == '0') {
                launch_reqrouter_client();
            } else if(client_type == '1') {
                launch_reqrouter_clientex();
            }
        }
    } else if(pcs_type == 's') {
        if(socket_type == REQ_REP) {
            launch_reqrep_server();
        } else if(socket_type == REQ_ROUTER) {
            launch_reqrouter_server();
        }
    }


    //if(argc >= 2) {
    //    char type = argv[1][0];
    //    std::cout << "Type: ";
    //    if(type == 's') {
    //        std::cout << "server" << std::endl;
    //        zmq::socket_t sock(ctx, zmq::socket_type::router);
    //        sock.bind("tcp://*:5556");
    //        while(!stop) {
    //            zmq::multipart_t msg;
    //            if(msg.recv(sock, ZMQ_DONTWAIT)) {
    //                
    //                std::string msg_ident(msg.popstr());
    //                std::string msg_null(msg.popstr());
    //                std::string msg_content(msg.popstr());

    //                std::cout << (msg.empty() ? "nothing to see here" : "YES!") << std::endl;
    //            }

    //            std::cout << "sleeping..." << std::endl;
    //            std::this_thread::sleep_for(5s);
    //        }
    //        std::cout << "SHUT DOWN." << std::endl;
    //    } else if(type == 'c') {
    //        std::cout << "client" << std::endl;

    //        zmq::socket_t sock(ctx, zmq::socket_type::req);
    //        std::string identity("foobar");
    //        sock.setsockopt(ZMQ_IDENTITY, identity.c_str(), identity.length() + 1);
    //        sock.connect("tcp://localhost:5556");
    //        if(sock.connected()) {
    //            std::cout << "connected." << std::endl;
    //            bool stop = false;
    //            std::string line;
    //            do {
    //                std::cin >> line;
    //                stop = line == "q";
    //                if(stop) {
    //                    std::cout << "quitting." << std::endl;
    //                    line = "quitting";
    //                }
    //                std::cout << "sent: '" << line << "'" << std::endl;
    //                //zmq::multipart_t msg;
    //                //msg.addstr(identity);
    //                //msg.add(zmq::message_t()); // empty
    //                //msg.addstr(line);
    //                //std::cout << "message sent: " << msg.send(sock) << std::endl;
    //                zmq::message_t msg(line.c_str(), line.length());
    //                std::cout << "message sent: " << sock.send(msg) << std::endl;
    //            } while(!stop);
    //        }
    //    } else {
    //        std::cout << "unknown" << std::endl;
    //    }
    //} else {
    //    std::cout << "Nothing happened" << std::endl;
    //}
    return 0;
}
