#include "PictureManager.h"
#include "PictureDAO.h"
#include <QTimer>
#include <QDebug>

PictureManager::PictureManager(QObject* parent) : QObject(parent) {}

void PictureManager::setBasePath(const QString& path) {
    m_basePath = path;
}

QString PictureManager::getDownloadedJsonPath() const {
    return m_basePath + "/downloaded.json";
}

bool PictureManager::loadCatalog(const QString& filepath) {
    QList<Picture> catalog = PictureDAO::loadCatalog(filepath);
    for (const Picture& pic : catalog) {
        bool exists = false;
        for (const Picture& p : m_pictures) {
            if (p.url() == pic.url()) { exists = true; break; }
        }
        if (!exists) m_pictures.append(pic);
    }
    return true;
}

bool PictureManager::loadDownloaded(const QString& filepath) {
    QList<Picture> downloadedPics = PictureDAO::loadDownloaded(filepath);
    for (const Picture& pic : downloadedPics) {
        for (Picture& existing : m_pictures) {
            if (existing.url() == pic.url()) {
                existing.setDescargada(true);
                existing.setFavorito(pic.favorito());
                existing.setFilePath(pic.filePath());
                break;
            }
        }
    }
    return true;
}

bool PictureManager::saveDownloaded(const QString& filepath) {
    return PictureDAO::saveDownloaded(m_pictures, filepath);
}

void PictureManager::downloadPicture(int index) {
    // Esta lista es la que viene de toDownload()
    QList<Picture> list = toDownload();
    if (index < 0 || index >= list.size()) return;

    QString targetUrl = list[index].url();

    QTimer* timer = new QTimer(this);
    int* progress = new int(0);

    connect(timer, &QTimer::timeout, this, [this, timer, progress, targetUrl]() {
        *progress += 10;

        // Buscar la imagen en la lista maestra para actualizar progreso
        for(Picture& p : m_pictures) {
            if(p.url() == targetUrl) {
                emit downloadProgress(*progress, p.nombre());
                if (*progress >= 100) {
                    p.setDescargada(true);
                    p.setFilePath(m_basePath + "/images/" + p.nombre() + ".jpg");
                    timer->stop();
                    saveDownloaded(getDownloadedJsonPath());
                    emit pictureDownloaded(p);
                    timer->deleteLater();
                    delete progress;
                }
                break;
            }
        }
    });
    timer->start(50);
}

void PictureManager::removeDownloaded(const Picture& picture) {
    for (int i = 0; i < m_pictures.size(); ++i) {
        if (m_pictures[i].url() == picture.url()) {
            m_pictures[i].setDescargada(false);
            m_pictures[i].setFavorito(false);
            saveDownloaded(getDownloadedJsonPath()); // Guardado inmediato
            emit pictureRemoved(m_pictures[i]);
            return;
        }
    }
}

void PictureManager::toggleFavorite(int indexReal) {
    if (indexReal >= 0 && indexReal < m_pictures.size()) {
        m_pictures[indexReal].setFavorito(!m_pictures[indexReal].favorito());
        saveDownloaded(getDownloadedJsonPath());
    }
}

QList<Picture> PictureManager::downloaded() const {
    QList<Picture> list;
    for (const Picture& p : m_pictures) if (p.descargada()) list.append(p);
    return list;
}

QList<Picture> PictureManager::toDownload() const {
    QList<Picture> list;
    for (const Picture& p : m_pictures) if (!p.descargada()) list.append(p);
    return list;
}
const QList<Picture>& PictureManager::allPictures() const {
    return m_pictures;
}

void PictureManager::toggleFavoriteByName(const QString& name) {
    for (int i = 0; i < m_pictures.size(); ++i) {
        if (m_pictures[i].nombre() == name) {
            m_pictures[i].setFavorito(!m_pictures[i].favorito());
            saveDownloaded(getDownloadedJsonPath());
            return; // Salimos en cuanto lo encontramos
        }
    }
}

void PictureManager::removeDownloadedByName(const QString& name) {
    for (int i = 0; i < m_pictures.size(); ++i) {
        if (m_pictures[i].nombre() == name) {
            m_pictures[i].setDescargada(false);
            m_pictures[i].setFavorito(false);
            saveDownloaded(getDownloadedJsonPath());
            emit pictureRemoved(m_pictures[i]);
            return;
        }
    }
    }
QVector<Picture> PictureManager::notDownloaded() const {
    QVector<Picture> result;
    for (const auto &pic : allPictures()) {  // allPictures() devuelve todas
        if (!pic.descargada()) result.append(pic);
    }
    return result;
}
