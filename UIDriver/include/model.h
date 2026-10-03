#ifndef MODEL_H
#define MODEL_H

#include <NeuroNetwork.h>
#include <QMutex>

struct SharedModel {
    NeuroNetwork* nn;

    QMutex mtx;
    std::vector<double> inputValues;
};

inline int getMaxNodes(const SharedModel& model)
{
    int maxNodes = model.inputValues.size();

    for (const Layer& layer : model.nn->getLayers())
    {
        maxNodes = std::max(maxNodes, (int) layer.getValues().size());
    }

    return maxNodes;
}



#endif // MODEL_H
