/**
 * @file PictureManager.cpp
 * @brief Gestión en memoria y persistencia de objetos Picture.
 *
 * Esta clase actúa como un servicio central que mantiene la lista de imágenes
 * (m_pictures), delega la carga/guardado en PictureDAO y expone métodos para
 * descargar, marcar como favorito y eliminar imágenes descargadas.
 *
 * Las operaciones que simulan progreso usan QTimer para emitir señales
 * (downloadProgress, pictureDownloaded, pictureRemoved).
 *
 * Nota: los métodos que modifican el estado guardan automáticamente el fichero
 * de descargadas mediante saveDownloaded(getDownloadedJsonPath()).
 */

#include "PictureManager.h"
#include "PictureDAO.h"
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QDate>
#include <QDebug>
#include <QThread>
#include <QMutexLocker>

/**
 * @brief Constructor.
 * @param parent Objeto padre (por defecto nullptr).
 */
PictureManager::PictureManager(QObject* parent) : QObject(parent) {}

/**
 * @brief Establece la ruta base donde se guardan las imágenes y el JSON.
 * @param path Ruta base (normalmente una carpeta del usuario).
 */
void PictureManager::setBasePath(const QString& path) {
    m_basePath = path;
}

/**
 * @brief Obtiene la ruta del fichero downloaded.json (donde se serializa el estado).
 * @return QString Ruta completa al fichero downloaded.json dentro de basePath.
 */
QString PictureManager::getDownloadedJsonPath() const {
    return m_basePath + "/downloaded.json";
}

/**
 * @brief Carga un catálogo desde un fichero JSON y lo une al listado interno.
 *
 * Para cada Picture del catálogo se añade a m_pictures únicamente si no existe
 * ya una entrada con la misma URL (evita duplicados).
 *
 * @param filepath Ruta del JSON de catálogo.
 * @return true Siempre devuelve true (la función no reporta errores a nivel API).
 */
bool PictureManager::loadCatalog(const QString& filepath)
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

        // Obtener URL relativa del JSON
        QString relativeUrl = obj["url"].toString();

        // Convertir a ruta absoluta
        QString absoluteUrl = resolveImagePath(relativeUrl);

        Picture pic(obj["nombre"].toString(),
                    absoluteUrl,  // Usar ruta absoluta
                    obj["descripcion"].toString());
        m_pictures.append(pic);
    }
    return true;
}

/**
 * @brief Carga el estado de las imágenes descargadas y actualiza los objetos internos.
 *
 * Para cada Picture cargada desde el JSON de descargadas, se busca el Picture
 * correspondiente en m_pictures (por URL) y se actualizan sus flags y filePath.
 *
 * @param filepath Ruta del JSON de descargadas.
 * @return true Siempre devuelve true (no se expone error en la firma).
 */
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

/**
 * @brief Guarda el estado actual de las imágenes descargadas en el JSON correspondiente.
 * @param filepath Ruta destino donde persistir las descargadas.
 * @return true si la operación de escritura tuvo éxito, false en caso contrario.
 */
bool PictureManager::saveDownloaded(const QString& filepath) {
    return PictureDAO::saveDownloaded(m_pictures, filepath);
}

/**
 * @brief Simula la descarga de una imagen indicada por índice dentro de la lista "toDownload()".
 *
 * Este método:
 *  - crea un QTimer que actúa como simulador de progreso,
 *  - emite downloadProgress(progress, nombre) periódicamente,
 *  - cuando el progreso alcanza 100 marca la imagen como descargada,
 *    asigna filePath y expirationDate (si no lo tenía),
 *  - guarda el estado descargado y emite pictureDownloaded(p).
 *
 * @param index Índice relativo dentro de la lista devuelta por toDownload().
 */
void PictureManager::downloadPicture(int index, int seconds) {
    // 1. Obtener la información básica de la imagen a descargar
    // Lo hacemos fuera del mutex porque toDownload() ya devuelve una copia
    QList<Picture> list = toDownload();
    if (index < 0 || index >= list.size()) return;

    QString targetUrl = list[index].url();
    QString targetName = list[index].nombre();

    // 2. --- CONTROL ANTI-BUG (Triple Click) ---
    // Bloqueamos brevemente para comprobar si esta URL ya se está descargando
    {
        QMutexLocker locker(&m_mutex);
        if (m_activeTasks.contains(targetUrl)) {
            return; // Si ya está en la lista de tareas activas, ignoramos la petición
        }
        m_activeTasks.insert(targetUrl); // Marcamos la imagen como "en proceso"
    }

    // 3. --- SIMULACIÓN DE DESCARGA ---
    // Este bucle corre en un hilo secundario, por lo que msleep no bloquea la UI
    for (int p = 0; p <= 100; p += 5) {
        QThread::msleep((seconds * 1000) / 20);
        emit downloadProgress(p, targetName);
    }

    // 4. --- ACTUALIZACIÓN DE DATOS Y PERSISTENCIA ---
    {
        QMutexLocker locker(&m_mutex);

        for (Picture& p : m_pictures) {
            if (p.url() == targetUrl) {
                p.setDescargada(true);
                p.setFilePath(m_basePath + "/images/" + p.nombre() + ".jpg");

                if (!p.expirationDate().isValid()) {
                    if (p.nombre() == "Tranvia entre arboles")
                        p.setExpirationDate(QDate::currentDate().addDays(-3));
                    else
                        p.setExpirationDate(QDate::currentDate().addDays(30));
                }

                saveDownloaded(getDownloadedJsonPath());

                emit pictureDownloaded(p);
                break;
            }
        }

        m_activeTasks.remove(targetUrl);
    }
}

/**
 * @brief Elimina (marca como no descargada) una imagen y simula progreso de eliminación.
 *
 * Busca el Picture por URL en m_pictures, simula un progreso con QTimer y cuando
 * finaliza marca la imagen como no descargada, desmarca el favorito y emite
 * pictureRemoved(m_pictures[i]) además de saveDownloaded(...).
 *
 * @param picture Picture a eliminar (por valor; se compara su URL).
 */
void PictureManager::removeDownloaded(const Picture& picture, int seconds) {
    QString pictureName = picture.nombre();
    QString pictureUrl = picture.url();

    // 1. --- CONTROL ANTI-BUG (Clicks repetidos) ---
    {
        QMutexLocker locker(&m_mutex);
        if (m_activeTasks.contains(pictureUrl)) {
            return; // Si ya se está borrando (o descargando), ignoramos
        }
        m_activeTasks.insert(pictureUrl);
    }

    // 2. --- SIMULACIÓN DE DESINSTALACIÓN ---
    // Usamos pasos de 10% para que sea visualmente distinto a la descarga
    for (int p = 0; p <= 100; p += 10) {
        QThread::msleep((seconds * 1000) / 10);
        emit downloadProgress(p, pictureName);
    }

    // 3. --- LIMPIEZA DE DATOS Y PERSISTENCIA ---
    {
        QMutexLocker locker(&m_mutex);

        for (int i = 0; i < m_pictures.size(); ++i) {
            if (m_pictures[i].url() == pictureUrl) {
                // Cambiamos el estado a "no descargada"
                m_pictures[i].setDescargada(false);
                m_pictures[i].setFavorito(false); // Al borrarla, quitamos el favorito

                // Persistimos el cambio en el JSON
                saveDownloaded(getDownloadedJsonPath());

                // Notificamos a la UI para que desaparezca de "Downloaded"
                // Enviamos -1 al progreso para indicar que la barra debe ocultarse
                emit downloadProgress(-1, pictureName);
                emit pictureRemoved(m_pictures[i]);
                break;
            }
        }

        // 4. --- LIBERAR TAREA ---
        m_activeTasks.remove(pictureUrl);
    }
}

/**
 * @brief Alterna la marca de favorito para un índice real en m_pictures.
 *
 * Este método cambia el flag favorito y persiste el estado inmediatamente.
 *
 * @param indexReal Índice dentro de m_pictures.
 */
void PictureManager::toggleFavorite(int indexReal) {
    if (indexReal >= 0 && indexReal < m_pictures.size()) {
        m_pictures[indexReal].setFavorito(!m_pictures[indexReal].favorito());
        saveDownloaded(getDownloadedJsonPath());
    }
}

/**
 * @brief Devuelve la lista de imágenes marcadas como descargadas.
 * @return QList<Picture> Lista filtrada de imágenes con descargada() == true.
 */
QList<Picture> PictureManager::downloaded() const {
    QList<Picture> list;
    for (const Picture& p : m_pictures) if (p.descargada()) list.append(p);
    return list;
}

/**
 * @brief Devuelve la lista de imágenes que aún no están descargadas.
 * @return QList<Picture> Lista filtrada de imágenes con descargada() == false.
 */
QList<Picture> PictureManager::toDownload() const {
    QList<Picture> list;
    for (const Picture& p : m_pictures) if (!p.descargada()) list.append(p);
    return list;
}

/**
 * @brief Acceso const a todas las imágenes gestionadas.
 * @return const QList<Picture>& Referencia const a la lista interna m_pictures.
 */
const QList<Picture>& PictureManager::allPictures() const {
    return m_pictures;
}

/**
 * @brief Alterna favorito buscando por nombre.
 *
 * Si se encuentra el primer elemento con el nombre dado, se alterna su flag
 * favorito y se persiste el estado.
 *
 * @param name Nombre de la imagen a buscar.
 */
void PictureManager::toggleFavoriteByName(const QString& name) {
    for (int i = 0; i < m_pictures.size(); ++i) {
        if (m_pictures[i].nombre() == name) {
            m_pictures[i].setFavorito(!m_pictures[i].favorito());
            saveDownloaded(getDownloadedJsonPath());
            return; // Salimos en cuanto lo encontramos
        }
    }
}

/**
 * @brief Marca como no descargada la imagen con el nombre dado y emite pictureRemoved.
 *
 * Persiste el cambio y emite la señal correspondiente.
 *
 * @param name Nombre de la imagen a eliminar (marcar como no descargada).
 */
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

/**
 * @brief Devuelve las imágenes que no están descargadas en un QVector (alternativa a toDownload()).
 * @return QVector<Picture> Vector con las imágenes no descargadas.
 */
QVector<Picture> PictureManager::notDownloaded() const {
    QVector<Picture> result;
    for (const auto &pic : allPictures()) {  // allPictures() devuelve todas
        if (!pic.descargada()) result.append(pic);
    }
    return result;
}

/**
 * @brief resuelve las rutas de las imagenes
 * @param relativePath
 * @return  QDir(m_basePath).filePath(relativePath); ruta obsoluta desde el basepath
 */

QString PictureManager::resolveImagePath(const QString& relativePath) const
{
    if (QFileInfo(relativePath).isAbsolute()) {
        return relativePath;  // Ya es absoluta
    }


    return QDir(m_basePath).filePath(relativePath);
}
