#ifndef PICTURE_H
#define PICTURE_H

#include "QtGui/qpixmap.h"
#include "SuiteCore_global.h"

class SUITECORE_EXPORT Picture
{
public:
    Picture() = default;
    Picture(const QString& nombre, const QString& url, const QString& descripcion);

    // Getters
    QString nombre() const;
    QString url() const;
    QString descripcion() const;
    QPixmap imagen() const;
    bool favorito() const;
    bool descargada() const;

    // Setters
    void setNombre(const QString& nombre);
    void setUrl(const QString& url);
    void setDescripcion(const QString& descripcion);
    void setImagen(const QPixmap& imagen);
    void setFavorito(bool fav);
    void setDescargada(bool descargada);
    void setFilePath(const QString& path) { m_filePath = path; }
    QString filePath() const { return m_filePath; }

private:
    QString m_nombre;
    QString m_url;
    QString m_descripcion;
    QPixmap m_imagen;
    bool m_favorito = false;
    bool m_descargada = false;
    QString m_filePath; // ruta local de la imagen

};

#endif // PICTURE_H
