#pragma once

#include <QByteArray>
#include <QString>

class VideoEncoder {
public:
    // Encoda un frame RGBA in H.264 (Annex-B). quality: "interactive"/"final"/default.
    static QByteArray encodeH264(const QByteArray &rgba, int width, int height,
                                 const QString &quality, QString &error);
};
