
#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <styleloader.h>
#include <widget.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Q_INIT_RESOURCE(resources);

    QString style = loadStyle(":/styles/label.qss") +
                    loadStyle(":/styles/pushbutton.qss") +
                    loadStyle(":/styles/spin.qss") +
                    loadStyle(":/styles/frame.qss") +
                    loadStyle(":/styles/combobox.qss") +
                    loadStyle(":/styles/arrow-down.svg");
    a.setStyleSheet(style);



    Widget w;
    w.show();
    return QCoreApplication::exec();
}
