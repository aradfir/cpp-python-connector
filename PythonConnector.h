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
    int port;
    int sock = 0, client_fd = 0;
public:
    int initialize_data_socket() {
        int valread;
        struct sockaddr_in serv_addr;


        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("\n Socket creation error \n");
            return -1;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);

        // Convert IPv4 and IPv6 addresses from text to binary
        // form
        if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
            printf("\nInvalid address/ Address not supported \n");
            return -1;
        }

        if ((client_fd = connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0) {
            printf("\nConnection Failed \n");
            return -1;
        }
        // closing the connected socket
        //close(client_fd);
        return 0;
    }

    string send_receive_data(string &data, int out_len) {
        char out_buffer[out_len];
        send(sock, data.c_str(), data.length(), 0);
        recv(sock, out_buffer, out_len, 0);
        return string(out_buffer);

    }

    string send_string(const string &module_name, const string &function_name, const string &data) {
        string final_message = module_name + " " + function_name + " " + data;
        return send_receive_data(final_message, 1024 * MAX_PACKET_SIZE_KB);
    }

    Tensor *
    send_tensor_data(const string &module_name, const string &function_name, const string &message, const Tensor &data,
                     string &out_message) {

        string data_string = string(data);
        string final_message = module_name + " " + function_name + " " + message + " " + data_string;
        string response = send_string(module_name, function_name, message + " " + data_string);
        out_message = response.substr(0, response.find(' '));
        return Tensor::from_string(response.substr(response.find(' ') + 1));
    }


    PythonConnector(int port) {
        this->port = port;
        initialize_data_socket();
    }

    virtual ~PythonConnector() {
        close(client_fd);
    }
};


#endif //SOCKET_TEST_PYTHONCONNECTOR_H
