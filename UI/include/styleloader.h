#ifndef STYLELOADER_H
#define STYLELOADER_H

#include <QFile>

inline QString loadStyle(const QString& path)
{
    QFile f(path);

    if (!f.open(QFile::ReadOnly)) {
        qWarning() << "Failed to open style file:" << path;
        return {};
    }

    return QString::fromUtf8(f.readAll());
}

#endif // STYLELOADER_H
