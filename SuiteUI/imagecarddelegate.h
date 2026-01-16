#ifndef IMAGECARDDELEGATE_H
#define IMAGECARDDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>

class ImageCardDelegate : public QStyledItemDelegate {
public:
    explicit ImageCardDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif
