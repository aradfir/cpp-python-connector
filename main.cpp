#include <iostream>
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <sstream>


using namespace std::chrono;
#define MAX_PACKET_SIZE_KB 56
#define PORT 1372
using namespace std;
int sock = 0, client_fd = 0;


class Tensor {
    vector<int> shape;
    vector<double> data;
public:
//    Tensor(vector<int> &shape,vector<T> &data){
//        std::cout<<"HIIIIII";
//        this->shape=shape;
//        this->data=data;
//    }
//    Tensor(vector<int>& shape,T default_value){
//        std::cout<<"HIIIIII";
//        this->shape=shape;
//        this->data=vector<T>(getSize(shape),default_value);
//    }
    Tensor(const vector<int> &shape, const vector<double> &data) {
        this->shape = shape;
        this->data = data;
    }

    Tensor(const vector<int> &shape, const double &default_value) {
        this->shape = shape;
        this->data = vector<double>(this->getSize(), default_value);
    }

    vector<int> &getShape() {
        return shape;
    }

    vector<double> &getData() {
        return data;
    }

    bool reShape(const vector<int> &new_shape) {
        if (getSize(new_shape) != this->getSize()) {
            throw std::invalid_argument("Shape does not match input data length!");
        }
        this->shape = new_shape;
        return true;
    }

    long long getSize() {
        return getSize(this->shape);
    }

    explicit operator std::string() const {
        stringstream ss;

        for (int i = 0; i < this->shape.size(); i++) {
            ss << shape[i];
            if (i != shape.size() - 1) {
                ss << ",";
            }
        }
        ss << " ";
        for (int i = 0; i < data.size(); i++) {
            ss << data[i];
            if (i != data.size() - 1) {
                ss << ",";
            }
        }
        return ss.str();
    }


    double &operator[](const vector<int> &idx) {
        int idx_ = 0;
        if (idx.size() != this->shape.size()) {
            throw std::invalid_argument("Index does not match shape!");
        }
        for (int i = 0; i < idx.size(); i++) {
            idx_ *= shape[i];
            idx_ += idx[i];
        }
        return data[idx_];
    }

    static Tensor *from_string(string str) {
        vector<int> shape;
        vector<double> data;
        int i = 0;
        while (str[i] != ' ') {
            int j = i;
            while (str[j] != ',' && str[j] != ' ') {
                j++;
            }
            shape.push_back(stoi(str.substr(i, j - i)));

            i = j + 1;
            if (str[j] == ' ')
                break;
        }
        //i++;
        while (i < str.size()) {
            int j = i;
            while (str[j] != ',' && str[j] != ' ') {
                j++;
            }

            data.push_back(stod(str.substr(i, j - i)));

            i = j + 1;
        }
        return new Tensor(shape, data);
    }


    static Tensor *from_string(const vector<int> &shape, string str) {
        vector<double> data;
        for (int i = 0; i < str.size(); i++) {
            if (str[i] == ',') {
                data.push_back(stod(str.substr(0, i)));
                str = str.substr(i + 1);
                i = 0;
            }
        }

        data.push_back(stod(str));
        if (data.size() != getSize(shape)) {
            throw std::invalid_argument("Shape does not match input data length!");
        }
        return new Tensor(shape, data);
    }


private:
    static long long getSize(const vector<int> &input_shape) {
        long long size = 1;
        for (int i: input_shape) {
            size *= i;
        }
        return size;
    }
};

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
    return t;

}

void mazeGame() {
    vector<vector<int>> maze(8, vector<int>(8, 0));
    int pos[2] = {0, 0};
    int end[2] = {7, 7};
    //maze[0][6]=-1;
    maze[pos[0]][pos[1]] = 1;
    maze[end[0]][end[1]] = 1;
    Tensor t = mazeToTensor(maze, pos);
    t.reShape({1, 66});
    string out_message;
    Tensor *res = send_data("goalie_model init_state", "", t, out_message);
    cout << out_message << string(*res) << endl;
    while (pos[0] != end[0] && pos[1] != end[1]) {
        res = send_data("goalie_model get_action", "", t, out_message);
        cout << out_message << string(*res) << endl;
        int action=(*res)[{0}];
        if(action==0) {
            pos[0]--;
            if(pos[0]<0)
                pos[0]=0;
        }
        else if(action==1) {
            pos[0]++;
            if(pos[0]>7)
                pos[0]=7;
        }
        else if(action==2) {
            pos[1]--;
            if(pos[1]<0)
                pos[1]=0;
        }
        else if(action==3) {
            pos[1]++;
            if(pos[1]>7)
                pos[1]=7;
        }

    }
}

int main(int argc, char const *argv[]) {
    initialize_data_socket();
    mazeGame();
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
//    close(client_fd);
//    return 0;
}

