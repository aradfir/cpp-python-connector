//
// Created by aradf on 7/30/2022.
//

#ifndef SOCKET_TEST_TENSOR_H
#define SOCKET_TEST_TENSOR_H

#include <sstream>
#include <chrono>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>
#include <cstdio>
#include <arpa/inet.h>
#include <iostream>

class Tensor {
    std::vector<int> shape;
    std::vector<double> data;
public:
    Tensor(const std::vector<int> &shape, const std::vector<double> &data) {
        this->shape = shape;
        this->data = data;
    }

    Tensor(const std::vector<int> &shape, const double &default_value) {
        this->shape = shape;
        this->data = std::vector<double>(this->getSize(), default_value);
    }

    std::vector<int> &getShape() {
        return shape;
    }

    std::vector<double> &getData() {
        return data;
    }

    bool reShape(const std::vector<int> &new_shape);

    long long getSize() {
        return getSize(this->shape);
    }

    operator std::string() const;


    double &operator[](const std::vector<int> &idx);

    static Tensor *from_string(std::string str);


    static Tensor *from_string(const std::vector<int> &shape, std::string str);


private:
    static long long getSize(const std::vector<int> &input_shape);
};


#endif //SOCKET_TEST_TENSOR_H
