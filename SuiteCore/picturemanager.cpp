#include "PictureManager.h"
#include "PictureDAO.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QTimer>
#include <QDebug>

PictureManager::PictureManager(QObject* parent)
    : QObject(parent)
{
}

bool PictureManager::loadFromJson(const QString& filepath)
{
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "No se pudo abrir el JSON:" << filepath;
        return false;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) return false;

    QJsonArray array = doc.array();
    m_pictures.clear();

    for (auto value : array) {
        QJsonObject obj = value.toObject();
        Picture pic(obj["nombre"].toString(),
                    obj["url"].toString(),
                    obj["descripcion"].toString());
        m_pictures.append(pic);
    }
    return true;
}

bool PictureManager::loadState(const QString& filepath)
{
    m_pictures = PictureDAO::loadPictures(filepath);
    return !m_pictures.isEmpty();
}

bool PictureManager::saveState(const QString& filepath)
{
    return PictureDAO::savePictures(m_pictures, filepath);
}

const QList<Picture>& PictureManager::pictures() const
{
    return m_pictures;
}

QList<Picture> PictureManager::favorites() const
{
    QList<Picture> favs;
    for (const Picture& pic : m_pictures) {
        if (pic.favorito())
            favs.append(pic);
    }
    return favs;
}

void PictureManager::downloadPicture(int index)
{
    if (index < 0 || index >= m_pictures.size()) return;

    Picture& pic = m_pictures[index];

    QTimer* timer = new QTimer(this);
    int* progress = new int(0);

    connect(timer, &QTimer::timeout, this, [this, timer, progress, &pic]() {
        *progress += 5;
        emit downloadProgress(*progress, pic.nombre());

        if (*progress >= 100) {
            timer->stop();
            timer->deleteLater();
            delete progress;

            pic.setDescargada(true);


            QString safeName = pic.nombre();
            pic.setFilePath("C:/Users/Edgar/Documents/ImageSuite/" + safeName + ".jpeg");

            emit pictureDownloaded(pic);
        }
    });

    timer->start(100);
}

void PictureManager::toggleFavorite(int index)
{
    if (index < 0 || index >= m_pictures.size()) return;
    Picture& pic = m_pictures[index];
    pic.setFavorito(!pic.favorito());
    saveDownloaded("C:/Users/Edgar/Documents/ImageSuite/downloaded.json");
}

bool PictureManager::loadCatalog(const QString& filepath)
{
    QList<Picture> catalog = PictureDAO::loadCatalog(filepath);
    if (catalog.isEmpty()) return false;

    for (Picture& pic : catalog) {
        bool exists = false;
        for (const Picture& existing : m_pictures) {
            if (existing.url() == pic.url()) {
                exists = true;
                break;
            }
        }
        if (!exists) m_pictures.append(pic);
    }

    return true;
}

bool PictureManager::loadDownloaded(const QString& filepath)
{
    QList<Picture> downloadedPics = PictureDAO::loadDownloaded(filepath);
    if (downloadedPics.isEmpty()) return false;

    for (Picture& pic : downloadedPics) {
        bool found = false;
        for (Picture& existing : m_pictures) {
            if (existing.url() == pic.url()) {
                existing.setDescargada(pic.descargada());
                existing.setFavorito(pic.favorito());
                found = true;
                break;
            }
        }
        if (!found) {
            m_pictures.append(pic);
        }
    }

    return true;
}

bool PictureManager::saveDownloaded(const QString& filepath)
{
    return PictureDAO::saveDownloaded(m_pictures, filepath);
}

const QList<Picture>& PictureManager::allPictures() const
{
    return m_pictures;
}

QList<Picture> PictureManager::toDownload() const
{
    QList<Picture> list;
    for (const Picture& pic : m_pictures) {
        if (!pic.descargada())
            list.append(pic);
    }
    return list;
}

QList<Picture> PictureManager::downloaded() const
{
    QList<Picture> list;
    for (const Picture& pic : m_pictures) {
        if (pic.descargada())
            list.append(pic);
    }
    return list;
}

void PictureManager::removeDownloaded(int index)
{
    if (index < 0 || index >= m_pictures.size()) return;

    Picture& pic = m_pictures[index];
    if (!pic.descargada()) return;

    pic.setDescargada(false);
    pic.setFavorito(false);

    emit pictureRemoved(pic);
}

int PictureManager::indexOf(const QString& name) const
{
    for (int i = 0; i < m_pictures.size(); ++i) {
        if (m_pictures[i].nombre() == name) {
            return i;
        }
    }
    return -1; // no encontrado
}
