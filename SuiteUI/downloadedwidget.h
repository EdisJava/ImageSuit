#ifndef DOWNLOADEDWIDGET_H
#define DOWNLOADEDWIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include <QAbstractItemView>
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

    // MÉTODO ESTÁTICO PARA BLOQUEAR ARRASTRE
    static void disableDragDrop(QAbstractItemView* view);

signals:
    void pictureDeleted();
    void openPicture(const Picture& picture);
    void searchChanged(const QString &text);
    void viewModeChanged(ImageCardDelegate::ViewMode mode);

private:
    Ui::DownloadedWidget *ui;
    PictureManager* m_pictureManager = nullptr;
    QStandardItemModel* m_model;
    ImageCardDelegate* m_delegate;
};
#endif
