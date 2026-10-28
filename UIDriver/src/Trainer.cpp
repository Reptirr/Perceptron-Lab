#include <Trainer.h>

void Trainer::infiniteTrain() {
    std::vector<std::vector<double>>& inputs = samples.inputs;
    std::vector<std::vector<double>>& targets = samples.targets;

    NeuroNetwork *nn = model.nn;

    std::vector<double> duty{};
    int duty_interval = -1;

    // тренируем по samples
    for (;; e++) {
        double totalLoss = 0.0;

        auto start = std::chrono::steady_clock::now();

        for (size_t i = 0; i < inputs.size(); i++) {
            std::vector<double> out = nn->forward(inputs[i]);

            totalLoss += nn->loss(out, targets[i]);

            if (need_stop) {
                need_stop = false;
                emit sharedModelUpdated();
                return;
            }

            // один шаг обучения
            nn->train(inputs[i], targets[i], lr);
        }

        auto end = std::chrono::steady_clock::now();

        auto dur = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

        duty_interval = static_cast<int>(need_interval / dur);
        if (duty_interval < 1) duty_interval = 1;

        std::cout << "init duty_interval. "
                  << "dur: " << dur
                  << " | need_interval: " << need_interval
                  << " | duty_interval: " << duty_interval
                  << "\n";

        totalLoss /= inputs.size();
        duty.push_back(totalLoss);

        if (duty.size() >= duty_interval || e == 1) {
            emit epocheFinish(e, std::accumulate(duty.begin(), duty.end(), 0.0) / duty.size());
            emit sharedModelUpdated();
            duty.clear();
        }

        std::cout << "Epoch " << std::setw(4) << e
                  << " | Loss: " << totalLoss
                  << " | dur: " << dur
                  << " | duty size: " << duty.size()
                  << "\n";
    }

    emit sharedModelUpdated();
}

void Trainer::onTrainRequest() {
    if (trainThread.joinable())
        trainThread.join();

    trainThread = std::thread([this] {
        infiniteTrain();
    });
}

void Trainer::onStopTrain() {
    need_stop = true;
}

void Trainer::onLrChanged(double lr1) {
    lr = lr1;
}

void Trainer::onReset() {
    e = 0;
}

Trainer::Trainer(Samples &samples1, SharedModel &shared_model, double lr1) : samples(samples1), model(shared_model), lr(lr1) {

}
