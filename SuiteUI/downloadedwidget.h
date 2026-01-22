#ifndef DOWNLOADEDWIDGET_H
#define DOWNLOADEDWIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QAbstractItemView>
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

    static void disableDragDrop(QAbstractItemView* view);

signals:
    void pictureDeleted();
    void openPicture(const Picture& picture);
    void searchTextChanged(const QString &text);
    void viewModeToggled(ImageCardDelegate::ViewMode mode);

private:
    void updateCompleterList();
    void setupConnections();
    void updateViews();

    Ui::DownloadedWidget *ui;
    PictureManager* m_pictureManager = nullptr;

    // Modelos
    QStandardItemModel* m_downloadedModel;
    QStandardItemModel* m_toDownloadModel;
    QSortFilterProxyModel* m_downloadedProxy;
    QSortFilterProxyModel* m_toDownloadProxy;

    ImageCardDelegate* m_delegate;

    // Autocompletar
    QCompleter* m_completer;
    QStringListModel* m_completerModel;
};

#endif // DOWNLOADEDWIDGET_H
