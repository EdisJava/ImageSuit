#include "ImageCardDelegate.h"
#include <QPainter>
#include <QMouseEvent>

/**
 * @file imagecarddelegate.cpp
 * @brief Delegate que pinta una tarjeta de imagen y gestiona clicks sobre botones integrados.
 *
 * Esta versión utiliza la implementación exacta que proporcionaste. He añadido
 * comentarios Doxygen para documentar las funciones principales y facilitar su lectura.
 *
 * NOTA: no he cambiado la lógica funcional; sólo añadí comentarios y documentación.
 */

/**
 * @brief Calcula los rectángulos locales (relativos al área de la tarjeta) para
 *        los botones y la barra de progreso.
 *
 * Los rects devueltos son relativos al origen de la tarjeta (no son coordenadas
 * del viewport). El comportamiento varía según el modo (Grid/List).
 *
 * @param size Tamaño disponible para la tarjeta.
 * @param mode Modo de vista (Grid o List).
 * @param favR Rect para el botón favorito (salida).
 * @param infR Rect para el botón de información (salida).
 * @param delR Rect para el botón eliminar (salida).
 * @param progR Rect para la barra de progreso (salida).
 */
static void getRectsLocal(const QSize &size, ImageCardDelegate::ViewMode mode,
                          QRect &favR, QRect &infR, QRect &delR, QRect &progR)
{
    const int btnS = 30;
    const int sp = 10;

    if (mode == ImageCardDelegate::Grid) {
        progR = QRect(20, size.height() - 55, size.width() - 40, 16);
        int bY = size.height() - 35;
        int startX = (size.width() - (btnS * 3 + sp * 2)) / 2;
        favR = QRect(startX, bY, btnS, btnS);
        infR = QRect(startX + btnS + sp, bY, btnS, btnS);
        delR = QRect(startX + (btnS + sp) * 2, bY, btnS, btnS);
    } else {
        int cY = (size.height() - btnS) / 2;
        favR = QRect(size.width() - 130, cY, btnS, btnS);
        infR = QRect(size.width() - 90, cY, btnS, btnS);
        delR = QRect(size.width() - 50, cY, btnS, btnS);
        progR = QRect(85, size.height() - 25, favR.left() - 95, 14);
    }
}

/**
 * @brief Dibuja la tarjeta de imagen con altura dinámica según el texto.
 *
 * Se utiliza option.rect ajustado (margen interior) y painter->translate(r.topLeft())
 * para dibujar contenidos relativos al origen de la tarjeta. Dibuja:
 *  - fondo redondeado con colores según estado (seleccionado/caducado),
 *  - icono/imagen,
 *  - texto con word wrap (modo Grid) o truncado (modo List),
 *  - barra de progreso (si está descargando o eliminando),
 *  - botones circulares (favorito, info, eliminar).
 *
 * En modo Grid, el texto usa word wrap y puede ocupar múltiples líneas.
 * En modo List, el texto se centra verticalmente en una línea.
 *
 * @param painter Puntero al QPainter ya inicializado.
 * @param option Opciones de estilo/rect del item.
 * @param index Índice del modelo a pintar.
 */
void ImageCardDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QRect r = option.rect.adjusted(4, 4, -4, -4);
    painter->translate(r.topLeft());
    QSize s = r.size();

    bool selected = option.state & QStyle::State_Selected;
    bool expired = index.data(ExpiredRole).toBool();

    // Determinar colores según estado
    QColor borderColor = selected ? QColor(255, 165, 0) : QColor(220, 220, 220);
    QColor fillColor = Qt::white;

    if (expired) {
        fillColor = QColor(255, 120, 120, 150);  // rojo suave semi-transparente
        // Borde más grueso y diferente si está seleccionada
        borderColor = selected ? QColor(255, 165, 0) : QColor(200, 0, 0);
    } else if (selected) {
        fillColor = QColor(255, 165, 0, 10);
        borderColor = QColor(255, 165, 0);
    } else {
        borderColor = QColor(220, 220, 220);
    }

    // Dibujar fondo de la tarjeta
    int borderWidth = selected ? 3 : 1;
    painter->setPen(QPen(borderColor, borderWidth));
    painter->setBrush(fillColor);
    painter->drawRoundedRect(QRect(0, 0, s.width(), s.height()), 10, 10);

    // Calcular rectángulos de botones
    QRect favR, infR, delR, progR;
    getRectsLocal(s, m_mode, favR, infR, delR, progR);

    // Dibujar icono/imagen y texto
    QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    if (m_mode == Grid) {
        // Modo Grid: imagen grande + texto con word wrap
        painter->drawPixmap(QRect(10, 10, s.width() - 20, 130), icon.pixmap(150, 150));
        painter->setPen(Qt::black);

        // Calcular área dinámica para el texto (entre imagen y botones)
        int textY = 145;
        int textHeight = s.height() - textY - 45;  // Espacio disponible hasta los botones

        // Dibujar texto con word wrap automático
        painter->drawText(QRect(5, textY, s.width() - 10, textHeight),
                          Qt::AlignCenter | Qt::AlignTop | Qt::TextWordWrap,
                          index.data().toString());
    } else {
        // Modo List: imagen pequeña + texto centrado en una línea
        painter->drawPixmap(10, 10, 60, 60, icon.pixmap(60, 60));
        painter->setPen(Qt::black);
        painter->drawText(QRect(85, 5, favR.left() - 90, 40),
                          Qt::AlignCenter | Qt::AlignVCenter,
                          index.data().toString());
    }

    // Dibujar barra de progreso si está descargando o eliminando
    int progress = index.data(ProgressRole).toInt();
    if (progress >= 0) {
        // Fondo de la barra (gris claro)
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(230, 230, 230));
        painter->drawRoundedRect(progR, 5, 5);

        // Relleno de progreso (naranja)
        painter->setBrush(QColor(255, 165, 0));
        painter->drawRoundedRect(progR.left(), progR.top(),
                                 (progR.width() * progress) / 100,
                                 progR.height(), 5, 5);

        // Texto del porcentaje
        painter->setPen(Qt::black);
        painter->setFont(QFont("Arial", 8, QFont::Bold));
        painter->drawText(progR, Qt::AlignCenter, QString("%1%").arg(progress));
    }

    // Función lambda para dibujar botones circulares
    auto drawButton = [&](const QRect &rect, const QString &text,
                          QColor background, QColor foreground) {
        painter->setBrush(background);
        painter->setPen(QPen(QColor(200, 200, 200), 1));
        painter->drawEllipse(rect);
        painter->setPen(foreground);
        painter->drawText(rect, Qt::AlignCenter, text);
    };

    // Dibujar botón de favorito (solo si está disponible)
    QVariant favoriteData = index.data(FavoriteRole);
    if (favoriteData.isValid()) {
        bool isFavorite = favoriteData.toBool();
        drawButton(favR,
                   isFavorite ? "★" : "☆",
                   Qt::white,
                   isFavorite ? QColor(255, 180, 0) : Qt::gray);
    }

    // Dibujar botón de información
    drawButton(infR, "i", QColor(240, 240, 240), Qt::black);

    // Dibujar botón de eliminar (solo si está descargada)
    if (index.data(DownloadedRole).toBool()) {
        drawButton(delR, "✕", QColor(255, 230, 230), Qt::red);
    }

    painter->restore();
}

/**
 * @brief Maneja eventos de ratón para detectar clicks sobre botones dentro de la tarjeta.
 *
 * Esta implementación original:
 *  - calcula clickPos relativo al rect ajustado y verifica contains() en los rects locales,
 *  - emite favoriteToggled/infoRequested/deleteRequested o doubleClicked según corresponda.
 *
 * @param event Evento entrante.
 * @param model Modelo asociado (no usado).
 * @param option Opciones de estilo/rect proporcionadas por la vista.
 * @param index Índice del item sobre el que se recibió el evento.
 * @return true si el evento fue consumido por el delegado; false si debe dejarse pasar.
 */
bool ImageCardDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        // Usar el mismo ajuste que en paint()
        QRect r = option.rect.adjusted(4, 4, -4, -4);

        // Calcular posición del clic en coordenadas ABSOLUTAS primero
        QPoint absoluteClickPos = mouseEvent->pos();

        // Verificar si el clic está dentro del rectángulo del item
        if (!option.rect.contains(absoluteClickPos)) {
            return false;
        }

        // Ahora calcular la posición relativa al rectángulo ajustado
        QPoint clickPos = absoluteClickPos - r.topLeft();

        // Obtener rectángulos de botones con el tamaño ajustado
        QRect favR, infR, delR, progR;
        getRectsLocal(r.size(), m_mode, favR, infR, delR, progR);

        // Click en favorito
        if (favR.contains(clickPos) && index.data(FavoriteRole).isValid()) {
            emit favoriteToggled(index);
            return true;
        }

        // Click en info
        if (infR.contains(clickPos)) {
            emit infoRequested(index);
            return true;
        }

        // Click en eliminar (solo si está descargada)
        if (delR.contains(clickPos) && index.data(DownloadedRole).toBool()) {
            emit deleteRequested(index);
            return true;
        }
    }

    // Doble click
    if (event->type() == QEvent::MouseButtonDblClick) {
        emit doubleClicked(index);
        return true;
    }

    return false;
}

/**
 * @brief Devuelve el tamaño sugerido para un item según el modo.
 *
 * En modo Grid devuelve un tamaño dependiente del ancho del texto . En modo List devuelve un tamaño con
 * ancho fijo (aquí fijado a 400) y altura 85. Puedes optar por usar option.rect.width()
 * para que ocupe el ancho del viewport (más flexible).
 */
QSize ImageCardDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (m_mode == List) {
        return QSize(400, 85);
    }
      // Modo Grid - calcular altura dinámica según el texto
     QString text = index.data(Qt::DisplayRole).toString();

    // Crear QFontMetrics para calcular el tamaño del texto
     QFontMetrics fm(option.font);

     // Calcular el rectángulo que necesita el texto con word wrap
     int textWidth = 170;  // Ancho disponible para el texto (180 - márgenes)
     QRect textRect = fm.boundingRect(
         0, 0, textWidth, 1000,  // Ancho fijo, alto ilimitado
         Qt::AlignCenter | Qt::TextWordWrap,
         text
         );

     // Altura base: imagen(130) + margen(15) + botones(40) + padding(10)
     int baseHeight = 195;

     // Añadir la altura del texto (mínimo 30 para una línea, máximo 90 para 3 líneas)
     int textHeight = qMax(30, qMin(textRect.height() + 10, 90));

     return QSize(180, baseHeight + textHeight);
}
