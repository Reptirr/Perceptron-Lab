#ifndef SAMPLES_H
#define SAMPLES_H

#include <vector>
#include <QDebug>
#include "defaultmodel.h"

struct Samples {
    std::vector<std::vector<double>> inputs{
        std::vector<double>(DefaultModel.conf[0])
    };

    std::vector<std::vector<double>> targets{
        std::vector<double>(DefaultModel.conf.back())
    };

    void debugPrint() const
    {
        qDebug() << "[Samples] inputs count:" << inputs.size();
        qDebug() << "[Samples] targets count:" << targets.size();

        if (!inputs.empty())
            qDebug() << "[Samples] input size:" << inputs[0].size();

        if (!targets.empty())
            qDebug() << "[Samples] target size:" << targets[0].size();
    }
};

#endif // SAMPLES_H
