#ifndef IMAGECARDDELEGATE_H
#define IMAGECARDDELEGATE_H

#include <QStyledItemDelegate>

class ImageCardDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    enum ViewMode { Grid, List };
    enum ItemDataRole {
        FavoriteRole = Qt::UserRole + 1,   // bool: es favorita
        DownloadedRole = Qt::UserRole + 2, // bool: está descargada (muestra botón eliminar)
        ProgressRole = Qt::UserRole + 5,   // int: progreso de descarga (0-100, -1 = sin descarga)
        ExpiredRole = Qt::UserRole + 6     // bool: imagen caducada
    };
    explicit ImageCardDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent), m_mode(Grid) {}

    void setViewMode(ViewMode mode) { m_mode = mode; }
    ViewMode viewMode() const { return m_mode; }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

signals:
    void favoriteToggled(const QModelIndex &index);
    void infoRequested(const QModelIndex &index);
    void deleteRequested(const QModelIndex &index);
    void doubleClicked(const QModelIndex &index);

private:
    ViewMode m_mode;
};

#endif
