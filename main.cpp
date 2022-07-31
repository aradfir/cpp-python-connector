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

bool isPositionLegal(int new_pos[2], vector<vector<int>> &maze) {
    if (new_pos[0] < 0 || new_pos[1] < 0 || new_pos[1] > maze.size() || new_pos[0] > maze.size())
        return false;
    if (maze[new_pos[0]][new_pos[1]] == -1)
        return false;

    return true;
}

int dist_in_direction_to_wall(int pos[2], const vector<int> &dir, vector<vector<int>> &maze) {
    int res = 0;
    int curr_pos[2] = {pos[0] + dir[0], pos[1] + dir[1]};

    while (isPositionLegal(curr_pos, maze)) {
        res++;
        curr_pos[0] += dir[0];
        curr_pos[1] += dir[1];
    }
    return res;
}

Tensor mazeToTensor(vector<vector<int>> &maze, int pos[2]) {
    Tensor t({11}, 0);
//    t[{0}]= dist_in_direction_to_wall(pos,{-1,0},maze)/7.0;
//    t[{1}]= dist_in_direction_to_wall(pos,{1,0},maze)/7.0;
//    t[{2}]= dist_in_direction_to_wall(pos,{0,-1},maze)/7.0;
//    t[{3}]= dist_in_direction_to_wall(pos,{0,1},maze)/7.0;
    int start_slice_row;
    int start_slice_col;
    int end_slice_row;
    int end_slice_col;
    start_slice_row = pos[0] - 1;
    end_slice_row = pos[0] + 1;
    start_slice_col = pos[1] - 1;
    end_slice_col = pos[1] + 1;

    for (int i = start_slice_row; i <= end_slice_row; i++) {
        for (int j = start_slice_col; j <= end_slice_col; j++) {
            int h = i - start_slice_row, v = j - start_slice_col;
            t[{(end_slice_col-start_slice_col+1) * h + v}] = maze[i][j];
        }
    }
//    t[{9}] = pos[0] / 7.0;
//    t[{10}] = pos[1] / 7.0;
//    t[{11}] = (14 - pos[0] - pos[1]) / 7.0;
    t[{9}] = (8 - pos[0]) / 7.0;
    t[{10}] = (8 - pos[1]) / 7.0;
    t.reShape({1, 11});
    return t;

}

void print_maze(const vector<vector<int>> &maze) {
    for (int i = 0; i < maze.size(); ++i) {
        for (int j = 0; j < maze[i].size(); ++j) {
            char c = ' ';
            switch (maze[i][j]) {
                case -1:
                    c = '#';
                    break;
                case 1:
                    c = '@';
                    break;
                case 2:
                    c = '$';
                    break;
                default:
                    c = ' ';
            }
            cout << c << " ";
        }
        cout << endl;
    }
}

vector<vector<int>> init_maze(int pos[2], int end[2], int num_walls) {
    vector<vector<int>> maze(12, vector<int>(12, 0));
    for (int i = 0; i < 12; i++) {
        maze[0][i] = -1;
        maze[i][0] = -1;
        maze[10][i] = -1;
        maze[i][10] = -1;
        maze[1][i] = -1;
        maze[i][1] = -1;
        maze[11][i] = -1;
        maze[i][11] = -1;
    }
    srand(time(NULL));
    //maze[0][6]=-1;
    maze[pos[0]][pos[1]] = 1;
    maze[end[0]][end[1]] = 2;
    for (int i = 0; i < num_walls; i++) {
        int j = 2 + rand() % 8, k = 2 + rand() % 8;
        if (maze[j][k] == 0) {
            maze[j][k] = -1;
        } else {
            i--;
        }
    }
    return maze;
}

bool reached_fin(int pos[2], int end[2]) {
    return pos[0] == end[0] && pos[1] == end[1];
}


double do_action(vector<vector<int>> &maze, int &action, int pos[2], int end[2]) {
    double reward = 0;
    int new_pos[2];
    if (action == 0) {
        new_pos[0] = pos[0] - 1;
        new_pos[1] = pos[1];
    } else if (action == 1) {
        new_pos[0] = pos[0] + 1;
        new_pos[1] = pos[1];

    } else if (action == 2) {
        new_pos[0] = pos[0];
        new_pos[1] = pos[1] - 1;
        if (pos[1] < 0)
            pos[1] = 0;
    } else if (action == 3) {
        new_pos[0] = pos[0];
        new_pos[1] = pos[1] + 1;

    }
    if (isPositionLegal(new_pos, maze)) {
        maze[pos[0]][pos[1]] = 0;
        pos[0] = new_pos[0];
        pos[1] = new_pos[1];
        maze[pos[0]][pos[1]] = 1;
        if (reached_fin(pos, end))
            reward = 10;
        else if (action == 1 || action == 3) {
            reward = -0.35;
        } else
            reward = -0.45;
    } else
        reward = -1;
    return reward;

}

void mazeGame(vector<vector<int>> maze) {
    if (maze[0][0] != 1) {
        cout << "ERRORRRRR";
        return;
    }
}

void mazeGame(int numOfWalls = 10) {
    int pos[2] = {2, 2};
    int end[2] = {9, 9};
    auto maze = init_maze(pos, end, numOfWalls);
    Tensor t = mazeToTensor(maze, pos);

    string out_message;
    Tensor *res = con.send_tensor_data("QLearning_sample", "init_state", "", t, out_message);
    cout << out_message << string(*res) << endl;
    int steps = 0;
    while (!reached_fin(pos, end)) {
        res = con.send_tensor_data("QLearning_sample", "get_action", "", t, out_message);
        print_maze(maze);
        int action = (*res)[{0}];
        cout << out_message << " " << action << endl;
        double reward = do_action(maze, action, pos, end);
        t = mazeToTensor(maze, pos);
        bool is_fin = reached_fin(pos, end);
        string reward_fin = to_string(reward) + " " + to_string(is_fin);
        res = con.send_tensor_data("QLearning_sample", "get_reward_new_state", reward_fin, t, out_message);
        cout << out_message << string(*res) << endl;
        steps++;
    }
    cout << "Game Done in " << steps << " steps" << endl;

}

int main(int argc, char const *argv[]) {
    int pos[2]{0, 0};
    int end[2]{7, 7};
    vector<int> numOfRunsPerWallCount;
    numOfRunsPerWallCount.push_back(4);
    numOfRunsPerWallCount.push_back(4);
    numOfRunsPerWallCount.push_back(5);
    numOfRunsPerWallCount.push_back(6);
    numOfRunsPerWallCount.push_back(6);
    numOfRunsPerWallCount.push_back(6);
    numOfRunsPerWallCount.push_back(6);
    numOfRunsPerWallCount.push_back(6);
    //auto maze=init_maze(pos,end,3);
    for (int i = 0; i < numOfRunsPerWallCount.size(); ++i) {
        for (int j = 0; j < numOfRunsPerWallCount[i]; j++)
            mazeGame(i);
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

