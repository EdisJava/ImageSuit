#ifndef IMAGECARDDELEGATE_H
#define IMAGECARDDELEGATE_H

#include <QStyledItemDelegate>

class ImageCardDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    enum ViewMode { Grid, List };
    enum ItemDataRole {
        FavoriteRole = Qt::UserRole + 1,
        DownloadedRole = Qt::UserRole + 2,
        ProgressRole = Qt::UserRole + 5,
        ExpiredRole = Qt::UserRole + 6
    };

    explicit ImageCardDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent), m_mode(Grid) {}

    void setViewMode(ViewMode mode) { m_mode = mode; }
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    ViewMode viewMode() const { return m_mode; }

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void setMassDownloadInProgress(bool inProgress) {
        m_massDownloadInProgress = inProgress;
    }

    bool isMassDownloadInProgress() const { return m_massDownloadInProgress; }

signals:
    void favoriteToggled(const QModelIndex &index);
    void infoRequested(const QModelIndex &index);
    void deleteRequested(const QModelIndex &index);
    void doubleClicked(const QModelIndex &index);

private:
    ViewMode m_mode;
    bool m_massDownloadInProgress = false;

};

#endif
