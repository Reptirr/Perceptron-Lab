// Matrix.cpp

#include <Matrix.h>

inline int Matrix::idx(int r, int c) const {
    return r * cols + c;
}

Matrix::Matrix(int r, int c, double value)
    : rows(r), cols(c), data(r * c, value) {}

Matrix::Matrix(const std::vector<std::vector<double>>& init) {
    if (init.empty())
        throw std::invalid_argument("Empty matrix");

    rows = init.size();
    cols = init[0].size();

    for (const auto& row : init)
        if ((int)row.size() != cols)
            throw std::invalid_argument("Jagged matrix");

    data.resize(rows * cols);

    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++)
            data[idx(r, c)] = init[r][c];
}

Matrix::Matrix(const Matrix& other) = default;
Matrix& Matrix::operator=(const Matrix& other) = default;

Matrix::Matrix(Matrix&& other) noexcept = default;
Matrix& Matrix::operator=(Matrix&& other) noexcept = default;

void Matrix::setData(const std::vector<double>& newData, int r, int c)
{
    if ((int)newData.size() != r * c)
        throw std::invalid_argument("Data size != rows * cols");

    rows = r;
    cols = c;
    data = newData;
}

std::string Matrix::toString() const
{
    std::stringstream ss;

    for (size_t i = 0; i < data.size(); i++)
    {
        ss << data[i];

        if (i + 1 != data.size())
            ss << " ";
    }

    return ss.str();
}

int Matrix::getRows() const {
    return rows;
}

int Matrix::getCols() const {
    return cols;
}

std::vector<double>& Matrix::raw() {
    return data;
}

double& Matrix::operator()(int r, int c) {
    return data[idx(r, c)];
}

const double& Matrix::operator()(int r, int c) const {
    return data[idx(r, c)];
}

void Matrix::resize(int r, int c, double value) {
    rows = r;
    cols = c;
    data.assign(r * c, value);
}

void Matrix::addRow(const std::vector<double>& row) {
    if ((int)row.size() != cols)
        throw std::invalid_argument("Row size != cols");

    data.insert(data.end(), row.begin(), row.end());
    rows++;
}

void Matrix::removeRow(int r) {
    if (r < 0 || r >= rows)
        throw std::out_of_range("Matrix: Invalid row index");

    data.erase(
        data.begin() + r * cols,
        data.begin() + (r + 1) * cols
        );

    rows--;
}

void Matrix::addColumn(const std::vector<double>& col)
{
    if ((int)col.size() != rows)
        throw std::invalid_argument("Column size != rows");

    std::vector<double> newData;
    newData.reserve(rows * (cols + 1));

    for (int r = 0; r < rows; r++) {

        for (int c = 0; c < cols; c++) {
            newData.push_back(data[r * cols + c]);
        }

        newData.push_back(col[r]);
    }

    cols++;
    data = std::move(newData);
}

void Matrix::removeColumn(int c)
{
    if (c < 0 || c >= cols)
        throw std::out_of_range("Matrix: Invalid column index");

    std::vector<double> newData;
    newData.reserve(rows * (cols - 1));

    for (int r = 0; r < rows; r++) {

        for (int j = 0; j < cols; j++) {

            if (j == c)
                continue;

            newData.push_back(data[r * cols + j]);
        }
    }

    cols--;
    data = std::move(newData);
}

Matrix Matrix::scaleColumns(const std::vector<double>& v) const {
    if ((int)v.size() != cols)
        throw std::invalid_argument("Vector size != cols");

    Matrix result(rows, cols);

    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++)
            result(r, c) = (*this)(r, c) * v[c];

    return result;
}

std::vector<double> Matrix::sumRows() const {
    std::vector<double> result(rows, 0.0);

    for (int r = 0; r < rows; r++) {
        double sum = 0.0;

        for (int c = 0; c < cols; c++)
            sum += data[idx(r, c)];

        result[r] = sum;
    }

    return result;
}

void Matrix::fillRandom(double minVal, double maxVal) {
    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::uniform_real_distribution<> dist(minVal, maxVal);

    for (auto& x : data)
        x = dist(gen);
}

std::ostream& operator<<(std::ostream& os, const Matrix& m) {
    os << std::fixed << std::setprecision(2);

    os << "[";

    for (int r = 0; r < m.rows; r++) {

        os << "[";

        for (int c = 0; c < m.cols; c++) {

            os << m.data[m.idx(r, c)];

            if (c + 1 != m.cols)
                os << " ";
        }

        os << "]";

        if (r + 1 != m.rows)
            os << "\n ";
    }

    os << "]";

    return os;
}