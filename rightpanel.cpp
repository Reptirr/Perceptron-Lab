#include "rightpanel.h"

RightPanel::RightPanel(UIDriver *d, QWidget *parent)
    : QWidget{parent}
{
    // элементы
    auto *incrementLayer = new QPushButton("Создать скрытый слой");
    auto *decrementLayer = new QPushButton("Удалить скрытый слой");
    incrementNode = new QPushButton("Добавить нейрон у выбранного слоя");
    decrementNode = new QPushButton("Удалить нейрон у выбранного слоя");;

    auto *topWidget = new QWidget();
    auto *hBox = new QHBoxLayout();
    auto *graph = new NetworkGraph(d, selectedLayer, parent);

    hBox->addWidget(incrementLayer);
    hBox->addWidget(decrementLayer);
    hBox->addWidget(incrementNode);
    hBox->addWidget(decrementNode);
    hBox->addStretch();
    topWidget->setLayout(hBox);

    // лайаут
    auto *mainLayout = new QVBoxLayout();
    setLayout(mainLayout);
    mainLayout->addWidget(topWidget);
    mainLayout->addWidget(graph);

    // параметры
    graph->setContentsMargins(0, 0, 0, 0);
    hBox->setContentsMargins(3, 3, 3, 3);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setStyleSheet(graph->styleSheet());
    topWidget->setStyleSheet(graph->styleSheet());
    topWidget->setStyleSheet("background-color: #1e1e1e");

    topWidget->setFixedHeight(45);

    // коннекты btns -> uidriver
    connect(incrementLayer, &QPushButton::clicked,
            d, &UIDriver::onAddLayer);
    connect(decrementLayer, &QPushButton::clicked,
            d, &UIDriver::onRemoveLayer);

    connect(incrementNode, &QPushButton::clicked,
            this, [this, d](){
        d->onAddNode(selectedLayer);
    });
    connect(decrementNode, &QPushButton::clicked,
            this, [this, d](){
        d->onRemoveNode(selectedLayer);
    });

    // коннекты uiDriver -> networkGraph
    connect(d, &UIDriver::sharedModelUpdated, graph, &NetworkGraph::onSharedModelUpdated);

    // коннекты networkGraph -> rightpanel
    connect(graph, &NetworkGraph::selectedLayerChanged, this, &RightPanel::onSelectedLayerChanged);

    // вызываем слот для изначального обновления
    onSelectedLayerChanged();
}


// slots
void RightPanel::onSelectedLayerChanged() {
    if (selectedLayer == -1) {
        incrementNode->setEnabled(false);
        decrementNode->setEnabled(false);
    } else {
        incrementNode->setEnabled(true);
        decrementNode->setEnabled(true);
    }
}