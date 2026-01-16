#include "PictureDAO.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

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
        list.append(pic);
    }
    return list;
}

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

// --------------------------------------------------
// Guardar estado de imágenes descargadas
bool PictureDAO::saveDownloaded(const QList<Picture>& pictures, const QString& filepath)
{
    QJsonArray array;
    for (const Picture& pic : pictures) {
        if (!pic.descargada()) continue; // solo guardamos descargadas
        QJsonObject obj;
        obj["nombre"] = pic.nombre();
        obj["url"] = pic.url();
        obj["descripcion"] = pic.descripcion();
        obj["favorito"] = pic.favorito();
        obj["descargada"] = pic.descargada();
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

// --------------------------------------------------
// Cargar imágenes descargadas
QList<Picture> PictureDAO::loadDownloaded(const QString& filepath)
{
    QList<Picture> list;
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "No se pudo abrir descargadas:" << filepath;
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
        pic.setDescargada(obj["descargada"].toBool());
        pic.setFavorito(obj["favorito"].toBool());
        list.append(pic);
    }

    return list;
}
