#ifndef DOWNLOADEDWIDGET_H
#define DOWNLOADEDWIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include "PictureManager.h"

namespace Ui {
class DownloadedWidget;
}

class DownloadedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadedWidget(QWidget *parent = nullptr);
    ~DownloadedWidget();

    void setPictureManager(PictureManager* manager);
    void refreshList();

signals:
    void pictureDeleted();
    void openPicture(const Picture& picture);


private slots:
    void onSelectionChanged(const QModelIndex &current);
    void onOpenClicked();
    void onFavClicked();
    void onInfoClicked();
    void onDeleteClicked();

private:
    Ui::DownloadedWidget *ui;
    PictureManager* m_pictureManager = nullptr;

    QStandardItemModel* m_model;
    QList<int> m_visibleIndexes; // índices reales en PictureManager
    int m_currentIndex = -1;

private slots:
    void onSearchClicked();
};

#endif // DOWNLOADEDWIDGET_H
