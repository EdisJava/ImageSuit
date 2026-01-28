#include "Picture.h"

// Constructor
Picture::Picture(const QString& nombre, const QString& url, const QString& descripcion)
    : m_nombre(nombre),
    m_url(url),
    m_descripcion(descripcion),
    m_favorito(false),
    m_descargada(false)
{
}

// Getters
QString Picture::nombre() const
{
    return m_nombre;
}

QString Picture::url() const
{
    return m_url;
}

QString Picture::descripcion() const
{
    return m_descripcion;
}

QPixmap Picture::imagen() const
{
    return m_imagen;
}

bool Picture::favorito() const
{
    return m_favorito;
}

bool Picture::descargada() const
{
    return m_descargada;
}

QDate Picture::expirationDate() const {
    return m_expirationDate;
}

//Setters

void Picture::setNombre(const QString& nombre)
{
    m_nombre = nombre;
}

void Picture::setUrl(const QString& url)
{
    m_url = url;
}

void Picture::setDescripcion(const QString& descripcion)
{
    m_descripcion = descripcion;
}

void Picture::setImagen(const QPixmap& imagen)
{
    m_imagen = imagen;
}

void Picture::setFavorito(bool fav)
{
    m_favorito = fav;
}

void Picture::setDescargada(bool descargada)
{
    m_descargada = descargada;
}
void Picture::setExpirationDate(const QDate &date) {
    m_expirationDate = date;

}



