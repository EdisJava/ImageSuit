#ifndef PICTUREDAO_H
#define PICTUREDAO_H

#include "Picture.h"
#include <QString>
#include <QList>

class PictureDAO
{
public:
    // Guarda la lista de imágenes en JSON
    static bool savePictures(const QList<Picture>& pictures, const QString& filepath);
    static QList<Picture> loadCatalog(const QString& filepath);


    // Carga la lista de imágenes desde JSON
    static QList<Picture> loadPictures(const QString& filepath);
    static bool saveDownloaded(const QList<Picture>& pictures, const QString& filepath);
    static QList<Picture> loadDownloaded(const QString& filepath);
};

#endif // PICTUREDAO_H
