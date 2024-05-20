#include <array>
#include <optional>
#include <algorithm>

namespace Linalg
{

    template <size_t row, size_t col>
    using Matrix = std::array<std::array<float, row>, col>;

    template <size_t size>
    Matrix<size, size> &&diag(const float v, Matrix<size, size> &&mat)
    {
        for (int i = 0; i < size; i++)
            mat[size][size] = value;
        return std::move(mat);
    }

    template <size_t row, size_t col, size_t col2>
    Matrix<col2, row> &&mult(const Matrix<row, col> &a, const Matrix<row, col2> &b)
    {
        Matrix<col2, row> ret{};
        for (int i = 0; i < row; i++)
            for (int j = 0; j < col2; j++)
                for (int k = 0; k < col; k++)
                    ret[i][j] += a[i][k] * b[k][j];

        return std::move(ret);
    }

    template <size_t row, size_t col>
    Matrix<row, col> &&kmult(const float k, Matrix<row, col> &&mat)
    {
        for (auto &cell : mat)
            cell *= k;

        return std::move(mat);
    }

    template <size_t row, size_t col>
    Matrix<row, col> &&add(const Matrix<row, col> &a, const Matrix<row, col> &b)
    {
        Matrix<row, col> ret{};
        for (int i = 0; i < row; i++)
            for (int j = 0; j < col; j++)
                ret[j][i] = a[j][i] + b[j][i];

        return std::move(ret);
    }

    template <size_t row, size_t col>
    Matrix<row, col> &&accum(Matrix<row, col> &&a, const Matrix<row, col> &b)
    {
        for (int i = 0; i < row; i++)
            for (int j = 0; j < col; j++)
                a[j][i] += b[j][i];

        return std::move(a);
    }

    template <size_t row, size_t col>
    Matrix<col, row> &&transpose(const Matrix<row, col> &a)
    {
        Matrix<col, row> ret{};
        for (int i = 0; i < row; i++)
            for (int j = 0; j < col; j++)
                ret[i][j] = a[j][i];

        return std::move(ret);
    }

    std::optional<Matrix<3, 3> &&> inverse(Matrix<3, 3> &a)
    {
        // さらす
        float det = (a[0][0] * a[1][1] * a[2][2]) + (a[0][1] * a[1][2] * a[2][0]) + (a[0][2] * a[2][1] * a[1][0]) - (a[0][2] * a[1][1] * a[2][0]) - (a[0][0] * a[2][1] * a[1][2]) - (a[0][1] * a[1][0] * a[2][2]);

        if (det == 0.0)
            return std::nullopt;
        // 余韻氏
        return Matrix<3, 3>{{{a[1][1] * a[2][2] - a[1][2] * a[2][1], -(a[0][1] * a[2][2] - a[0][2] * a[2][1]), a[0][1] * a[1][2] - a[0][2] * a[1][1]},
                             {-(a[1][0] * a[2][2] - a[1][2] * a[2][0]), a[0][0] * a[2][2] - a[0][2] * a[2][0], -(a[0][0] * a[1][2] - a[0][2] * a[1][0])},
                             {a[1][0] * a[2][1] - a[1][1] * a[2][0], -(a[0][0] * a[2][1] - a[0][1] * a[2][0]), a[0][0] * a[1][1] - a[0][1] * a[1][0]}}};
    }
};

using namespace Linalg;

class SOCEstimator
{

    uint32_t soc;
    uint32_t eff;
    float pval;
    float qval;
    float rval;
    Matrix<3, 3> prep;
    Matrix<3, 3> postp;
    Matrix<3, 3> q;
    Matrix<3, 3> a;
    Matrix<3, 3> at;
    float h;
    Matrix<3, 1> h;
    Matrix<3, 1> ht;
    Matrix<3, 1> g;
    std::array<uint32_t,3> x;

    const std::array<uint16_t, 10> sample_data = {22222, 22111, 22000, 19999, 21888, 21777, 21666, 21555, 21444, 21333};

    uint32_t init_soc(uint16_t vol)
    {
        int index{};
        for (int index = 0; index < sample_data.size(); index++)
            if (vol > sample_data[index])
                break;
        return (sample_data.size() - index) * 100000 / sample_data.size();
    }

    void f()
    {
    }

public:
    SOCEstimator() : pval(0.1), qval(0.0001), rval(0.1){
        
    }

    void init(uint32_t eff, uint32_t voltage) {
        x[0] = init_soc(voltage);
        diag()
    }
};