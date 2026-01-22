#include "ImageCardDelegate.h"
#include <QPainter>
#include <QMouseEvent>

static void getRectsLocal(const QSize &size, ImageCardDelegate::ViewMode mode,
                          QRect &favR, QRect &infR, QRect &delR, QRect &progR)
{
    int btnS = 30;
    int sp = 10;
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

void ImageCardDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QRect r = option.rect.adjusted(4, 4, -4, -4);
    painter->translate(r.topLeft());
    QSize s = r.size();

    bool sel = option.state & QStyle::State_Selected;
    bool expired = index.data(Qt::UserRole + 6).toBool(); // <-- Nuevo: caducada

    // Cambiamos fondo según caducada o seleccionada
    QColor borderColor = sel ? QColor(255,165,0) : QColor(220,220,220);
    QColor fillColor = Qt::white;

    if(expired) {
        fillColor = QColor(255, 120, 120, 150); // rojo suave semi-transparente
        borderColor = QColor(200, 0, 0);        // borde rojo más fuerte
    } else if(sel) {
        fillColor = QColor(255,165,0,10);
    }

    painter->setPen(QPen(borderColor, 1));
    painter->setBrush(fillColor);
    painter->drawRoundedRect(QRect(0,0,s.width(), s.height()), 10, 10);

    QRect favR, infR, delR, progR;
    getRectsLocal(s, m_mode, favR, infR, delR, progR);

    QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    if(m_mode == Grid) {
        painter->drawPixmap(QRect(10,10,s.width()-20, 130), icon.pixmap(150,150));
        painter->setPen(Qt::black);
        painter->drawText(QRect(5,145,s.width()-10,30), Qt::AlignCenter, index.data().toString());
    } else {
        painter->drawPixmap(10,10,60,60, icon.pixmap(60,60));
        painter->setPen(Qt::black);
        painter->drawText(QRect(85,5,favR.left()-90,40), Qt::AlignLeft|Qt::AlignVCenter, index.data().toString());
    }

    int prog = index.data(Qt::UserRole + 5).toInt();
    if(prog >= 0) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(230,230,230));
        painter->drawRoundedRect(progR, 5, 5);
        painter->setBrush(QColor(255,165,0));
        painter->drawRoundedRect(progR.left(), progR.top(), (progR.width()*prog)/100, progR.height(), 5, 5);
        painter->setPen(Qt::black);
        painter->setFont(QFont("Arial", 8, QFont::Bold));
        painter->drawText(progR, Qt::AlignCenter, QString("%1%").arg(prog));
    }

    auto draw = [&](const QRect &rect, const QString &t, QColor b, QColor f) {
        painter->setBrush(b);
        painter->setPen(QPen(QColor(200,200,200),1));
        painter->drawEllipse(rect);
        painter->setPen(f);
        painter->drawText(rect, Qt::AlignCenter, t);
    };

    QVariant fv = index.data(Qt::UserRole + 1);
    if(fv.isValid()) draw(favR, fv.toBool()?"★":"☆", Qt::white, fv.toBool()?QColor(255,180,0):Qt::gray);
    draw(infR, "i", QColor(240,240,240), Qt::black);
    if(index.data(Qt::UserRole+2).toBool()) draw(delR, "✕", QColor(255,230,230), Qt::red);

    painter->restore();
}


bool ImageCardDelegate::editorEvent(QEvent *e, QAbstractItemModel *m, const QStyleOptionViewItem &o, const QModelIndex &i) {
    if(e->type() == QEvent::MouseButtonPress) {
        QRect favR, infR, delR, progR;
        getRectsLocal(o.rect.size(), m_mode, favR, infR, delR, progR);
        QPoint p = static_cast<QMouseEvent*>(e)->pos() - o.rect.topLeft();
        if(favR.contains(p) && i.data(Qt::UserRole+1).isValid()) { emit favoriteToggled(i); return true; }
        if(infR.contains(p)) { emit infoRequested(i); return true; }
        if(delR.contains(p) && i.data(Qt::UserRole+2).toBool()) { emit deleteRequested(i); return true; }
    }
    if(e->type() == QEvent::MouseButtonDblClick) { emit doubleClicked(i); return true; }
    return false;
}

QSize ImageCardDelegate::sizeHint(const QStyleOptionViewItem &o, const QModelIndex &) const {
    return (m_mode == Grid) ? QSize(180, 240) : QSize(o.rect.width(), 85);
}
