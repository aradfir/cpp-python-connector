#include <iostream>
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include "Tensor.h"


using namespace std::chrono;
#define MAX_PACKET_SIZE_KB 56
#define PORT 1372
using namespace std;
int sock = 0, client_fd = 0;


int initialize_data_socket() {
    int valread;
    struct sockaddr_in serv_addr;


    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

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


string send_receive_data(const char *data, int len, int out_len) {
    char out_buffer[out_len];
    send(sock, data, len, 0);
    recv(sock, out_buffer, out_len, 0);
    return string(out_buffer);

}


Tensor *send_data(const string &function_name, const string &message, const Tensor &data, string &out_message) {

    string data_string = string(data);
    string final_message = function_name + " " + message + " " + data_string;
    const char *final_cast = final_message.c_str();

    string s = send_receive_data(final_cast, strlen(final_cast),
                                 1024 * MAX_PACKET_SIZE_KB); // NOLINT(cppcoreguidelines-narrowing-conversions)
    out_message = s.substr(0, s.find(' '));
    return Tensor::from_string(s.substr(s.find(' ') + 1));
}

Tensor mazeToTensor(vector<vector<int>> maze, int pos[2]) {
    Tensor t({66}, 0);
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            t[{8 * i + j}] = maze[i][j];
    t[{64}] = pos[0];
    t[{65}] = pos[1];
    t.reShape({1, 66});
    return t;

}

void mazeGame() {
    vector<vector<int>> maze(8, vector<int>(8, 0));
    int pos[2] = {0, 0};
    int end[2] = {7, 7};
    //maze[0][6]=-1;
    maze[pos[0]][pos[1]] = 1;
    maze[end[0]][end[1]] = 2;
    Tensor t = mazeToTensor(maze, pos);

    string out_message;
    Tensor *res = send_data("QLearning_sample init_state", "", t, out_message);
    cout << out_message << string(*res) << endl;
    int steps = 0;
    while (pos[0] != end[0] || pos[1] != end[1]) {
        res = send_data("QLearning_sample get_action", "", t, out_message);

        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                cout << maze[i][j] << " ";
            }
            cout << endl;
        }
        int action = (*res)[{0}];
        cout << out_message << " " << action << endl;
        maze[pos[0]][pos[1]] = 0;
        double reward = -5;
        if (action == 0) {
            pos[0]--;
            if (pos[0] < 0)
                pos[0] = 0;
        } else if (action == 1) {
            pos[0]++;
            if (pos[0] > 7)
                pos[0] = 7;
            else reward = 10;
        } else if (action == 2) {
            pos[1]--;
            if (pos[1] < 0)
                pos[1] = 0;
        } else if (action == 3) {
            pos[1]++;
            if (pos[1] > 7)
                pos[1] = 7;
            else reward = 10;
        }
        maze[pos[0]][pos[1]] = 1;

        if (pos[0] == end[0] && pos[1] == end[1])
            reward = 100;
        t = mazeToTensor(maze, pos);
        bool is_fin = pos[0] == end[0] && pos[1] == end[1];
        string reward_fin = to_string(reward) + " " + to_string(is_fin);
        res = send_data("QLearning_sample get_reward_new_state", reward_fin, t, out_message);
        cout << out_message << string(*res) << endl;
        steps++;
    }
    cout << "Game Done in " << steps << " steps" << endl;

}

int main(int argc, char const *argv[]) {
    initialize_data_socket();
    for (int i = 0; i < 10; ++i) {

        mazeGame();
    }

//    Tensor tensor2({7}, 0);
//    tensor2[{0}] = 1.0;
//    tensor2.reShape({1, 7});
//    tensor2[{0, 2}] = 5.0;
//    cout << string(tensor2) << endl;
//    string out_message;

//    auto start = high_resolution_clock::now();
//
//    Tensor *result = send_data("models predict", "abalone", tensor2, out_message);
//    auto elapsed = std::chrono::high_resolution_clock::now() - start;
//    std::cout << string(*result) << std::endl;
//    long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
//            elapsed).count();
//    std::cout << "Time taken by function: " << microseconds << " microseconds" << std::endl;
      close(client_fd);

//    return 0;
}

