#pragma once
// Garuda Standard Library: Machine Learning Primitives Engine (`ml_primitives.hpp`)
// Mengimplementasikan Tensor Multidimensi, Matmul, Fungsi Aktivasi, & Neural Network SGD Optimizer.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

namespace garuda::beam::ml {

class Tensor {
public:
    Tensor() : shape_({0}) {}
    explicit Tensor(std::vector<std::size_t> shape, double val = 0.0)
        : shape_(std::move(shape)) {
        std::size_t total = 1;
        for (auto dim : shape_) total *= dim;
        data_.assign(total, val);
    }
    Tensor(std::vector<std::size_t> shape, std::vector<double> data)
        : shape_(std::move(shape)), data_(std::move(data)) {}

    const std::vector<std::size_t>& shape() const { return shape_; }
    const std::vector<double>& data() const { return data_; }
    std::vector<double>& data() { return data_; }

    std::size_t size() const { return data_.size(); }
    std::size_t rows() const { return shape_.size() > 0 ? shape_[0] : 0; }
    std::size_t cols() const { return shape_.size() > 1 ? shape_[1] : 1; }

    double get(std::size_t i) const { return data_[i]; }
    void set(std::size_t i, double val) { data_[i] = val; }

    double at(std::size_t r, std::size_t c) const {
        return data_[r * cols() + c];
    }
    void set_at(std::size_t r, std::size_t c, double val) {
        data_[r * cols() + c] = val;
    }

    Tensor reshape(std::vector<std::size_t> new_shape) const {
        std::size_t total = 1;
        for (auto dim : new_shape) total *= dim;
        if (total != data_.size()) {
            throw std::runtime_error("Reshape total size mismatch");
        }
        return Tensor(new_shape, data_);
    }

    Tensor transpose() const {
        if (shape_.size() != 2) return *this;
        std::size_t r = rows();
        std::size_t c = cols();
        Tensor res({c, r}, 0.0);
        for (std::size_t i = 0; i < r; ++i) {
            for (std::size_t j = 0; j < c; ++j) {
                res.set_at(j, i, at(i, j));
            }
        }
        return res;
    }

    Tensor matmul(const Tensor& other) const {
        if (cols() != other.rows()) {
            throw std::runtime_error("Matmul inner dimensions mismatch");
        }
        std::size_t m = rows();
        std::size_t n = cols();
        std::size_t p = other.cols();
        Tensor res({m, p}, 0.0);

        for (std::size_t i = 0; i < m; ++i) {
            for (std::size_t j = 0; j < p; ++j) {
                double sum = 0.0;
                for (std::size_t k = 0; k < n; ++k) {
                    sum += at(i, k) * other.at(k, j);
                }
                res.set_at(i, j, sum);
            }
        }
        return res;
    }

    Tensor add(const Tensor& other) const {
        Tensor res(shape_, 0.0);
        for (std::size_t i = 0; i < data_.size(); ++i) {
            res.data_[i] = data_[i] + (i < other.data_.size() ? other.data_[i] : 0.0);
        }
        return res;
    }

    Tensor relu() const {
        Tensor res(shape_, 0.0);
        for (std::size_t i = 0; i < data_.size(); ++i) {
            res.data_[i] = std::max(0.0, data_[i]);
        }
        return res;
    }

    Tensor sigmoid() const {
        Tensor res(shape_, 0.0);
        for (std::size_t i = 0; i < data_.size(); ++i) {
            res.data_[i] = 1.0 / (1.0 + std::exp(-data_[i]));
        }
        return res;
    }

    Tensor softmax() const {
        Tensor res(shape_, 0.0);
        double max_val = *std::max_element(data_.begin(), data_.end());
        double sum = 0.0;
        for (std::size_t i = 0; i < data_.size(); ++i) {
            res.data_[i] = std::exp(data_[i] - max_val);
            sum += res.data_[i];
        }
        for (std::size_t i = 0; i < data_.size(); ++i) {
            res.data_[i] /= sum;
        }
        return res;
    }

private:
    std::vector<std::size_t> shape_;
    std::vector<double> data_;
};

class NeuralNetworkEngine {
public:
    NeuralNetworkEngine(std::size_t input_dim, std::size_t output_dim)
        : weights_({input_dim, output_dim}, 0.1), bias_({1, output_dim}, 0.0) {}

    Tensor forward(const Tensor& input) {
        last_input_ = input;
        Tensor logits = input.matmul(weights_).add(bias_);
        return logits.sigmoid();
    }

    double train_step(const Tensor& input, const Tensor& target, double lr = 0.01) {
        Tensor pred = forward(input);
        double loss = 0.0;
        for (std::size_t i = 0; i < pred.size(); ++i) {
            double diff = pred.get(i) - target.get(i);
            loss += diff * diff;
        }

        // Simple SGD weight update
        for (std::size_t i = 0; i < weights_.size(); ++i) {
            weights_.data()[i] -= lr * (pred.get(i % pred.cols()) - target.get(i % target.cols()));
        }
        return loss / pred.size();
    }

    const Tensor& weights() const { return weights_; }

private:
    Tensor weights_;
    Tensor bias_;
    Tensor last_input_;
};

} // namespace garuda::beam::ml
