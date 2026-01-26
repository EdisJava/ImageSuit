/**
 * @file PictureDAO.cpp
 * @brief Operaciones de persistencia para objetos Picture (cargar/guardar JSON).
 *
 * Implementa utilidades para serializar y deserializar listas de Picture a archivos JSON.
 * Las fechas de caducidad se guardan/leen en formato ISO (Qt::ISODate).
 *
 * Notas:
 * - Las funciones devuelven QList<Picture> o bool según correspondan.
 * - Se usan qWarning/qDebug para mensajes de diagnóstico cuando hay errores de I/O.
 */

#include "PictureDAO.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

/**
 * @brief Guarda una lista de Picture en un fichero JSON.
 *
 * Cada Picture se mapea a un objeto JSON con las siguientes claves:
 * - "nombre", "url", "descripcion", "favorito", "descargada", "expirationDate" (opcional).
 *
 * @param pictures Lista de objetos Picture a serializar.
 * @param filepath Ruta completa del fichero donde se guardará el JSON.
 * @return true si el fichero se escribió correctamente; false en caso de error de I/O.
 */
bool PictureDAO::savePictures(const QList<Picture>& pictures, const QString& filepath)
{
    QJsonArray array;
    for (const Picture& pic : pictures) {
        QJsonObject obj;
        obj["nombre"] = pic.nombre();
        obj["url"] = pic.url();
        obj["descripcion"] = pic.descripcion();
        obj["favorito"] = pic.favorito();
        obj["descargada"] = pic.descargada();
        if (pic.expirationDate().isValid())
            obj["expirationDate"] = pic.expirationDate().toString(Qt::ISODate); // ISO (YYYY-MM-DD)
        array.append(obj);
    }

    QJsonDocument doc(array);
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "No se pudo guardar el JSON:" << filepath;
        return false;
    }

    file.write(doc.toJson());
    file.close();
    return true;
}

/**
 * @brief Carga una lista de Picture desde un fichero JSON.
 *
 * Se espera que el documento JSON sea un array de objetos con las mismas claves que
 * produce savePictures(). Si el fichero no puede abrirse o el JSON no es un array,
 * se devuelve una lista vacía.
 *
 * @param filepath Ruta del fichero JSON a leer.
 * @return QList<Picture> con los objetos cargados (puede estar vacío).
 */
QList<Picture> PictureDAO::loadPictures(const QString& filepath)
{
    QList<Picture> list;
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "No se pudo abrir el JSON:" << filepath;
        return list;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) return list;

    QJsonArray array = doc.array();
    for (auto value : array) {
        QJsonObject obj = value.toObject();
        Picture pic(obj["nombre"].toString(),
                    obj["url"].toString(),
                    obj["descripcion"].toString());
        pic.setFavorito(obj["favorito"].toBool());
        pic.setDescargada(obj["descargada"].toBool());

        // Leer fecha de caducidad si existe (ISO date)
        if (obj.contains("expirationDate")) {
            QDate date = QDate::fromString(obj["expirationDate"].toString(), Qt::ISODate);
            pic.setExpirationDate(date);
        }

        list.append(pic);
    }
    return list;
}

/**
 * @brief Carga un catálogo (lista de Pictures) desde un fichero JSON.
 *
 * Función similar a loadPictures pero pensada para catálogos (no lee flags ni fechas
 * adicionales salvo nombre/url/descripcion). Se registran mensajes de depuración con
 * información sobre la apertura del fichero.
 *
 * @param filepath Ruta del fichero JSON del catálogo.
 * @return QList<Picture> con los elementos del catálogo (vacío si error).
 */
QList<Picture> PictureDAO::loadCatalog(const QString& filepath)
{
    QList<Picture> list;
    QFile file(filepath);
    qDebug() << "Intentando abrir catalogo:" << filepath;
    qDebug() << "¿Existe el archivo?" << file.exists();

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "No se pudo abrir catalogo:" << filepath
                   << "Error:" << file.errorString();
        return list;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) return list;

    for (auto value : doc.array()) {
        QJsonObject obj = value.toObject();
        Picture pic(obj["nombre"].toString(),
                    obj["url"].toString(),
                    obj["descripcion"].toString());
        list.append(pic);
    }

    return list;
}

/**
 * @brief Guarda el estado de las imágenes descargadas en un fichero JSON.
 *
 * Solo se serializan las Picture cuya propiedad descargada() es true. Si la imagen
 * no tiene fecha de caducidad válida, se añade una fecha por defecto a 7 días
 * desde la fecha actual.
 *
 * @param pictures Lista completa de Picture; se filtrarán las descargadas.
 * @param filepath Ruta del fichero donde se guardará el estado.
 * @return true si la escritura tuvo éxito; false en caso contrario.
 */
bool PictureDAO::saveDownloaded(const QList<Picture>& pictures, const QString& filepath)
{
    QJsonArray array;
    for (const Picture& pic : pictures) {
        if (!pic.descargada()) continue;

        QJsonObject obj;
        obj["nombre"] = pic.nombre();
        obj["url"] = pic.url();
        obj["descripcion"] = pic.descripcion();
        obj["favorito"] = pic.favorito();
        obj["descargada"] = pic.descargada();

        // Si no tiene fecha de caducidad válida, establecer una por defecto (7 días)
        if (!pic.expirationDate().isValid()) {
            QDate newExp = QDate::currentDate().addDays(7);
            obj["expirationDate"] = newExp.toString(Qt::ISODate);
        } else {
            obj["expirationDate"] = pic.expirationDate().toString(Qt::ISODate);
        }

        array.append(obj);
    }

    QJsonDocument doc(array);
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "No se pudo guardar descargadas:" << filepath;
        return false;
    }

    file.write(doc.toJson());
    file.close();
    return true;
}

/**
 * @brief Carga la lista de imágenes descargadas desde un JSON.
 *
 * Se espera que cada objeto contenga al menos las claves: nombre, url, descripcion.
 * Además se leen las flags 'descargada' y 'favorito' y la fecha de caducidad si existe.
 *
 * @param filepath Ruta del fichero JSON con las imágenes descargadas.
 * @return QList<Picture> con los elementos cargados (vacío si error).
 */
QList<Picture> PictureDAO::loadDownloaded(const QString& filepath)
{
    QList<Picture> list;
    QFile file(filepath);

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "No se pudo abrir descargadas:" << filepath;
        return list;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isArray()) return list;

    for (const QJsonValue &value : doc.array()) {
        QJsonObject obj = value.toObject();

        Picture pic(
            obj["nombre"].toString(),
            obj["url"].toString(),
            obj["descripcion"].toString()
            );

        pic.setDescargada(obj["descargada"].toBool());
        pic.setFavorito(obj["favorito"].toBool());

        // Leer fecha de caducidad si existe (ISO date)
        if (obj.contains("expirationDate")) {
            QDate date = QDate::fromString(obj["expirationDate"].toString(), Qt::ISODate);
            pic.setExpirationDate(date);
        }

        list.append(pic);
    }

    return list;
}
