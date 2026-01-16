#ifndef PICTUREMANAGER_H
#define PICTUREMANAGER_H

#include <QObject>
#include <QList>
#include "Picture.h"
#include "SuiteCore_global.h"

class PictureDAO;

class SUITECORE_EXPORT PictureManager : public QObject
{
    Q_OBJECT

public:
    explicit PictureManager(QObject* parent = nullptr);

    // Carga las imágenes desde JSON inicial o desde persistencia
    bool loadFromJson(const QString& filepath);
    bool loadState(const QString& filepath);
    bool saveState(const QString& filepath);
    bool loadCatalog(const QString& filepath);
    bool loadDownloaded(const QString& filepath);
    bool saveDownloaded(const QString& filepath);
    void removeDownloaded(int index);
    int indexOf(const QString& name) const;

    // Acceso a la lista de imágenes
    const QList<Picture>& pictures() const;
    QList<Picture> favorites() const;
    const QList<Picture>& allPictures() const;
    QList<Picture> toDownload() const;
    QList<Picture> downloaded() const;

    // Operaciones sobre imágenes
    void downloadPicture(int index);
    void toggleFavorite(int index);

signals:
    void pictureDownloaded(const Picture& picture);
    void downloadProgress(int progress, const QString& pictureName);
    void pictureRemoved(const Picture& picture);

private:
    QList<Picture> m_pictures;
};

#endif // PICTUREMANAGER_H
