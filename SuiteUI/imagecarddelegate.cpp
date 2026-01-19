#include "ImageCardDelegate.h"
#include <QPainter>
#include <QMouseEvent>

void getRects(const QRect &cardRect, ImageCardDelegate::ViewMode mode, QRect &favR, QRect &infR, QRect &delR) {
    QRect r = cardRect.adjusted(5, 5, -5, -5);
    if (mode == ImageCardDelegate::Grid) {
        favR = QRect(r.left() + 15, r.bottom() - 35, 30, 30);
        infR = QRect(r.center().x() - 15, r.bottom() - 35, 30, 30);
        delR = QRect(r.right() - 45, r.bottom() - 35, 30, 30);
    } else {
        int centerY = r.top() + (r.height() - 30) / 2;
        delR = QRect(r.right() - 40, centerY, 30, 30);
        infR = QRect(r.right() - 80, centerY, 30, 30);
        favR = QRect(r.right() - 120, centerY, 30, 30);
    }
}

void ImageCardDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QRect rect = option.rect.adjusted(5, 5, -5, -5);
    bool isSelected = option.state & QStyle::State_Selected;
    painter->setBrush(isSelected ? QColor(255, 165, 0, 40) : Qt::white);
    painter->setPen(QPen(isSelected ? QColor(255, 140, 0) : QColor(200, 200, 200), 1));
    painter->drawRoundedRect(rect, 10, 10);
    QRect favR, infR, delR;
    getRects(option.rect, m_mode, favR, infR, delR);
    QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    if (m_mode == Grid) {
        painter->drawPixmap(rect.adjusted(15, 15, -15, -85), icon.pixmap(150, 150));
        painter->setPen(Qt::black);
        painter->drawText(rect.left(), rect.bottom() - 65, rect.width(), 20, Qt::AlignCenter, index.data().toString());
    } else {
        painter->drawPixmap(rect.left() + 10, rect.top() + 10, 60, 60, icon.pixmap(60, 60));
        painter->setPen(Qt::black);
        painter->drawText(rect.left() + 85, rect.top(), rect.width() - 250, rect.height(), Qt::AlignVCenter, index.data().toString());
    }
    painter->setBrush(QColor(240, 240, 240));
    painter->drawEllipse(infR); painter->drawText(infR, Qt::AlignCenter, "i");
    if (index.data(Qt::UserRole + 2).toBool()) {
        painter->setBrush(QColor(255, 200, 200));
        painter->drawEllipse(delR); painter->drawText(delR, Qt::AlignCenter, "X");
    }
    QVariant fav = index.data(Qt::UserRole + 1);
    if (fav.isValid()) {
        painter->setBrush(fav.toBool() ? Qt::yellow : Qt::white);
        painter->drawEllipse(favR); painter->drawText(favR, Qt::AlignCenter, "★");
    }
    painter->restore();
}

bool ImageCardDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
    QRect favR, infR, delR;
    getRects(option.rect, m_mode, favR, infR, delR);
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        if (infR.contains(me->pos())) { emit infoRequested(index); return true; }
        if (favR.contains(me->pos()) && index.data(Qt::UserRole + 1).isValid()) { emit favoriteToggled(index); return true; }
        if (delR.contains(me->pos()) && index.data(Qt::UserRole + 2).toBool()) { emit deleteRequested(index); return true; }
    }
    if (event->type() == QEvent::MouseButtonDblClick) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        if (!infR.contains(me->pos()) && !favR.contains(me->pos()) && !delR.contains(me->pos())) {
            emit doubleClicked(index); return true;
        }
    }
    return false;
}

QSize ImageCardDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return (m_mode == List) ? QSize(option.rect.width(), 80) : QSize(170, 220);
}
