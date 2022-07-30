#include <iostream>
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include "Tensor.h"
#include "PythonConnector.h"

using namespace std::chrono;

#define PORT 1372
using namespace std;
PythonConnector con(PORT);

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

void print_maze(const vector<vector<int>> &maze) {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            cout << maze[i][j] << " ";
        }
        cout << endl;
    }
}

vector<vector<int>> init_maze(int pos[2], int end[2]) {
    vector<vector<int>> maze(8, vector<int>(8, 0));
    //maze[0][6]=-1;
    maze[pos[0]][pos[1]] = 1;
    maze[end[0]][end[1]] = 2;
    return maze;
}
bool reached_fin(int pos[2],int end[2]){
    return pos[0] == end[0] && pos[1] == end[1];
}
double do_action(vector<vector<int>> &maze,int &action,int pos[2],int end[2])
{
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
    if (reached_fin(pos,end))
        reward = 100;
    return reward;

}
void mazeGame() {
    int pos[2] = {0, 0};
    int end[2] = {7, 7};
    auto maze = init_maze(pos, end);
    Tensor t = mazeToTensor(maze, pos);

    string out_message;
    Tensor *res = con.send_tensor_data("QLearning_sample", "init_state", "", t, out_message);
    cout << out_message << string(*res) << endl;
    int steps = 0;
    while (!reached_fin(pos,end)) {
        res = con.send_tensor_data("QLearning_sample", "get_action", "", t, out_message);
        print_maze(maze);
        int action = (*res)[{0}];
        cout << out_message << " " << action << endl;
        double reward= do_action(maze,action,pos,end);
        t = mazeToTensor(maze, pos);
        bool is_fin = reached_fin(pos,end);
        string reward_fin = to_string(reward) + " " + to_string(is_fin);
        res = con.send_tensor_data("QLearning_sample", "get_reward_new_state", reward_fin, t, out_message);
        cout << out_message << string(*res) << endl;
        steps++;
    }
    cout << "Game Done in " << steps << " steps" << endl;

}

int main(int argc, char const *argv[]) {
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
//    return 0;
}

