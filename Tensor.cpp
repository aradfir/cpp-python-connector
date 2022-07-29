//
// Created by aradf on 7/30/2022.
//

#include "Tensor.h"

bool Tensor::reShape(const std::vector<int> &new_shape) {
    if (getSize(new_shape) != this->getSize()) {
        throw std::invalid_argument("Shape does not match input data length!");
    }
    this->shape = new_shape;
    return true;
}

long long Tensor::getSize(const std::vector<int> &input_shape) {
    long long size = 1;
    for (int i: input_shape) {
        size *= i;
    }
    return size;
}

Tensor *Tensor::from_string(const std::vector<int> &shape, std::string str) {
    std::vector<double> data;
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

Tensor *Tensor::from_string(std::string str) {
    std::vector<int> shape;
    std::vector<double> data;
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

double &Tensor::operator[](const std::vector<int> &idx) {
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

Tensor::operator std::string() const {
    std::stringstream ss;

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
