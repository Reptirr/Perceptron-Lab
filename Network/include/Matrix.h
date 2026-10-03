// Matrix.h
#pragma once

#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <random>

class Matrix {
private:
    int rows = 0, cols = 0;
    std::vector<double> data;

    inline int idx(int r, int c) const;

public:
    Matrix(int r = 0, int c = 0, double value = 0.0);
    Matrix(const std::vector<std::vector<double>>& init);

    Matrix(const Matrix& other);
    Matrix& operator=(const Matrix& other);

    Matrix(Matrix&& other) noexcept;
    Matrix& operator=(Matrix&& other) noexcept;

    void setData(const std::vector<double>& newData, int r, int c);

    std::string toString() const;

    int getRows() const;
    int getCols() const;

    std::vector<double>& raw();

    double& operator()(int r, int c);
    const double& operator()(int r, int c) const;

    void resize(int r, int c, double value = 0.0);

    void addRow(const std::vector<double>& row);
    void removeRow(int r);

    void addColumn(const std::vector<double>& col);
    void removeColumn(int c);

    Matrix scaleColumns(const std::vector<double>& v) const;

    std::vector<double> sumRows() const;

    void fillRandom(double minVal = -0.5, double maxVal = 0.5);

    friend std::ostream& operator<<(std::ostream& os, const Matrix& m);
};