#include "ImageCardDelegate.h"
#include <QPainter>
#include <QMouseEvent>

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

void ImageCardDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
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
        borderColor = QColor(200, 0, 0);          // borde rojo
    } else if (selected) {
        fillColor = QColor(255, 165, 0, 10);
    }

    // Dibujar fondo de la tarjeta
    painter->setPen(QPen(borderColor, 1));
    painter->setBrush(fillColor);
    painter->drawRoundedRect(QRect(0, 0, s.width(), s.height()), 10, 10);

    // Calcular rectángulos de botones
    QRect favR, infR, delR, progR;
    getRectsLocal(s, m_mode, favR, infR, delR, progR);

    // Dibujar icono/imagen
    QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    if (m_mode == Grid) {
        painter->drawPixmap(QRect(10, 10, s.width() - 20, 130), icon.pixmap(150, 150));
        painter->setPen(Qt::black);
        painter->drawText(QRect(5, 145, s.width() - 10, 30),
                          Qt::AlignCenter | Qt::AlignVCenter,
                          index.data().toString());
    } else {
        painter->drawPixmap(10, 10, 60, 60, icon.pixmap(60, 60));
        painter->setPen(Qt::black);
        painter->drawText(QRect(85, 5, favR.left() - 90, 40),
                          Qt::AlignCenter | Qt::AlignVCenter,
                          index.data().toString());
    }

    // Dibujar barra de progreso si está descargando
    int progress = index.data(ProgressRole).toInt();
    if (progress >= 0) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(230, 230, 230));
        painter->drawRoundedRect(progR, 5, 5);
        painter->setBrush(QColor(255, 165, 0));
        painter->drawRoundedRect(progR.left(), progR.top(),
                                 (progR.width() * progress) / 100,
                                 progR.height(), 5, 5);
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

    // Dibujar botón de favorito
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

bool ImageCardDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index)
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

QSize ImageCardDelegate::sizeHint(const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    Q_UNUSED(index);
    if (m_mode == Grid) {
        return QSize(180, 240);
    } else {
        // En modo List, usar un ancho fijo grande o el del viewport
        return QSize(400, 85);  // O mejor aún, obtener el ancho del viewport
    }
}
