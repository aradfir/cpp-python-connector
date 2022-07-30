//
// Created by divar on 7/30/22.
//

#ifndef SOCKET_TEST_PYTHONCONNECTOR_H
#define SOCKET_TEST_PYTHONCONNECTOR_H

#include <iostream>
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include "Tensor.h"

#define MAX_PACKET_SIZE_KB 56
using namespace std;

class PythonConnector {

public:
    string send_string(const string &module_name, const string &function_name, const string &data);

    Tensor *
    send_tensor_data(const string &module_name, const string &function_name, const string &message, const Tensor &data,
                     string &out_message);


    PythonConnector(int port) {
        this->port = port;
        initialize_data_socket();
    }

    virtual ~PythonConnector() {
        close(client_fd);
    }

private:
    int port;
    int sock = 0, client_fd = 0;

    int initialize_data_socket();

    string send_receive_data(string &data, int out_len);
};


#endif //SOCKET_TEST_PYTHONCONNECTOR_H
