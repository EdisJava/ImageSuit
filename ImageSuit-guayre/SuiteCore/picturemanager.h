#ifndef PICTUREMANAGER_H
#define PICTUREMANAGER_H

#include <QObject>
#include <QList>
#include <QString>
#include <QSet>
#include <QMutex>
#include "Picture.h"
#include "SuiteCore_global.h"

class PictureDAO;

class SUITECORE_EXPORT PictureManager : public QObject
{
    Q_OBJECT

public:
    explicit PictureManager(QObject* parent = nullptr);

    void setBasePath(const QString& path);
    void removeDownloaded(const Picture& picture, int seconds);

    QString getDownloadedJsonPath() const;
    QString getImagesFolderPath() const;
    QVector<Picture> notDownloaded() const;
    QString resolveImagePath(const QString& relativePath) const;

    // Carga y guardado
    bool loadFromJson(const QString& filepath);
    bool loadState(const QString& filepath);
    bool saveState(const QString& filepath);
    bool loadCatalog(const QString& filepath);
    bool loadDownloaded(const QString& filepath);
    bool saveDownloaded(const QString& filepath);


    int indexOf(const QString& name) const;

    // Acceso a las listas
    const QList<Picture>& pictures() const;
    const QList<Picture>& allPictures() const;

    QList<Picture> favorites() const;
    QList<Picture> toDownload() const;
    QList<Picture> downloaded() const;

    // Operaciones
    void downloadPicture(int index, int seconds);
    void toggleFavorite(int indexReal); // Se recomienda usar índice real de m_pictures
    void toggleFavoriteByName(const QString& name);
    void removeDownloadedByName(const QString& name);

signals:
    void pictureDownloaded(const Picture& picture);
    void downloadProgress(int progress, const QString& pictureName);
    void pictureRemoved(const Picture& picture);

private:
    QList<Picture> m_pictures;
    QString m_basePath;
    mutable QMutex m_mutex;
    QSet<QString> m_activeTasks;
};

#endif // PICTUREMANAGER_H
