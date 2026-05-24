#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>
#include <QLabel>
#include <QDoubleSpinBox>
#include <sstream>

inline std::string vecToString(const std::vector<double>& v)
{
    std::ostringstream ss;
    ss << "{";

    for (size_t i = 0; i < v.size(); ++i)
    {
        ss << v[i];

        if (i + 1 != v.size())
            ss << ", ";
    }

    ss << "}";

    return ss.str();
}
inline std::string vecToString(const std::vector<int>& v)
{
    std::string s = "{";

    for (size_t i = 0; i < v.size(); i++)
    {
        s += std::to_string(v[i]);

        if (i + 1 != v.size())
            s += ", ";
    }

    s += "}";
    return s;
}

inline std::string vecToString(const std::vector<std::vector<double>>& v)
{
    std::string s = "{";

    for (size_t i = 0; i < v.size(); i++)
    {
        s += "{";

        for (size_t j = 0; j < v[i].size(); j++)
        {
            s += std::to_string(v[i][j]);

            if (j + 1 != v[i].size())
                s += ", ";
        }

        s += "}";

        if (i + 1 != v.size())
            s += ", ";
    }

    s += "}";
    return s;
}
inline std::string joinVector(const std::vector<double> &v) {
    std::string str;

    for (size_t i = 0; i < v.size(); i++) {
        str += std::to_string(v[i]);
        if (i+1 != v.size()) {
            str += " ";
        }
    }

    return str;
}



inline QDoubleSpinBox* takeLastSpin(std::vector<QDoubleSpinBox*>& v)
{
    if (v.empty()) return nullptr;

    auto* ptr = v.back();
    v.pop_back();
    return ptr;
}
inline QLabel* takeLastLabel(std::vector<QLabel*>& v)
{
    if (v.empty()) return nullptr;

    auto* ptr = v.back();
    v.pop_back();
    return ptr;
}

#endif // UTILS_H
