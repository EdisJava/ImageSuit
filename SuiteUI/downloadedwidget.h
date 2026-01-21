#ifndef DOWNLOADEDWIDGET_H
#define DOWNLOADEDWIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include "PictureManager.h"
#include "ImageCardDelegate.h"

class QCompleter;
class QStringListModel;

namespace Ui {
class DownloadedWidget;
}

class DownloadedWidget : public QWidget {
    Q_OBJECT

public:
    explicit DownloadedWidget(QWidget *parent = nullptr);
    ~DownloadedWidget();

    void setPictureManager(PictureManager* manager);
    void refreshList();

signals:
    void pictureDeleted();
    void openPicture(const Picture& picture);

private:
    void updateCompleterList();

    Ui::DownloadedWidget *ui;
    PictureManager* m_pictureManager = nullptr;
    QStandardItemModel* m_model;
    ImageCardDelegate* m_delegate;

    // Para el autocompletado
    QCompleter* m_completer;
    QStringListModel* m_completerModel;
};

#endif // DOWNLOADEDWIDGET_H
