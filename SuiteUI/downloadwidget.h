#ifndef DOWNLOADWIDGET_H
#define DOWNLOADWIDGET_H

#include <QWidget>
#include <QStandardItemModel>  // <-- cambio aquí
#include "PictureManager.h"

namespace Ui {
class DownloadWidget;
}

class DownloadWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadWidget(QWidget *parent = nullptr);
    ~DownloadWidget();

    void setPictureManager(PictureManager* manager);
    void refreshList();

signals:
    void pictureDownloaded();  // señal para avisar a MainWindow o DownloadedWidget

private slots:
    void onSelectionChanged(const QModelIndex &current);
    void onDownloadClicked();
    void onInfoClicked();

    void onDownloadProgress(int progress, const QString& pictureName);
    void onPictureDownloaded(const Picture& picture);

private:
    Ui::DownloadWidget *ui;
    PictureManager* m_pictureManager = nullptr;

    QStandardItemModel* m_model;  // <-- cambio aquí
    int m_currentIndex = -1;
};

#endif // DOWNLOADWIDGET_H
