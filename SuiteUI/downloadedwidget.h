#ifndef DOWNLOADEDWIDGET_H
#define DOWNLOADEDWIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include <QSortFilterProxyModel> // <-- añadir
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
    void viewModeToggled(ImageCardDelegate::ViewMode newMode);
    void searchTextChanged(const QString &text);

private:
    void updateCompleterList();

    Ui::DownloadedWidget *ui;
    PictureManager* m_pictureManager = nullptr;
    QStandardItemModel* m_model;
    QSortFilterProxyModel* m_proxyModel;  // <-- añadir proxy
    ImageCardDelegate* m_delegate;

    // Para el autocompletado
    QCompleter* m_completer;
    QStringListModel* m_completerModel;
};

#endif // DOWNLOADEDWIDGET_H
