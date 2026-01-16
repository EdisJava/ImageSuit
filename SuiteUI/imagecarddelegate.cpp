#include "ImageCardDelegate.h"
#include <QPainter>
#include <QStyleOptionViewItem>

void ImageCardDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    painter->save();

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->setRenderHint(QPainter::TextAntialiasing);

    QRect rect = option.rect.adjusted(5, 5, -5, -5);
    bool isSelected = option.state & QStyle::State_Selected;
    bool isHovered = option.state & QStyle::State_MouseOver;

    QColor bgColor = QColor(255, 255, 255, 200);
    QColor borderColor = QColor(200, 200, 200);
    int borderWidth = 1;

    if (isSelected) {
        bgColor = QColor(255, 165, 0, 80);
        borderColor = QColor(255, 140, 0);
        borderWidth = 2;
    } else if (isHovered) {
        bgColor = QColor(255, 255, 255, 240);
    }

    painter->setBrush(bgColor);
    painter->setPen(QPen(borderColor, borderWidth));
    painter->drawRoundedRect(rect, 12, 12);

    QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    QRect imageArea = rect.adjusted(10, 10, -10, -45);

    if (!icon.isNull()) {
        QPixmap pixmap = icon.pixmap(imageArea.size());
        int x = imageArea.left() + (imageArea.width() - pixmap.width()) / 2;
        int y = imageArea.top() + (imageArea.height() - pixmap.height()) / 2;
        painter->drawPixmap(x, y, pixmap);
    } else {
        painter->setPen(QColor(150, 150, 150));
        painter->drawText(imageArea, Qt::AlignCenter, "No Preview\nAvailable");
    }

    QString name = index.data(Qt::DisplayRole).toString();
    QRect textArea = QRect(rect.left() + 5, rect.bottom() - 35, rect.width() - 10, 30);

    QFont font = painter->font();
    font.setBold(true);
    font.setPointSize(9);
    painter->setFont(font);
    painter->setPen(QColor(40, 40, 40));

    QString elidedName = painter->fontMetrics().elidedText(name, Qt::ElideRight, textArea.width());
    painter->drawText(textArea, Qt::AlignCenter, elidedName);

    painter->restore();
}

QSize ImageCardDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(160, 190);
}
