#ifndef DOWNLOADEDWIDGET_H
#define DOWNLOADEDWIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include "PictureManager.h"
#include "ImageCardDelegate.h"

namespace Ui { class DownloadedWidget; }

class DownloadedWidget : public QWidget {
    Q_OBJECT
public:
    explicit DownloadedWidget(QWidget *parent = nullptr);
    ~DownloadedWidget();
    void setPictureManager(PictureManager* manager);
    void refreshList();

signals:
    // ESTAS SEÑALES SON LAS QUE PIDEN LAS LÍNEAS 43 Y 44 DE MAINWINDOW.CPP
    void pictureDeleted();
    void openPicture(const Picture& picture);

private:
    Ui::DownloadedWidget *ui;
    PictureManager* m_pictureManager = nullptr;
    QStandardItemModel* m_model;
    ImageCardDelegate* m_delegate;
};
#endif
