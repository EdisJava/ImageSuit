#ifndef DOWNLOADEDWIDGET_H
#define DOWNLOADEDWIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QAbstractItemView>
#include "PictureManager.h"
#include "ImageCardDelegate.h"
#include "ui_DownloadedWidget.h"

class QCompleter;
class QStringListModel;

// Roles internos para identificar de forma única el Picture guardado en cada item
static const int ItemUrlRole = Qt::UserRole + 100;   // guarda picture.url()
static const int ItemNameRole = Qt::UserRole + 101;  // guarda picture.nombre() sin sufijos

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

public slots:
    void onMassDownloadStarted() {
        m_delegate->setMassDownloadInProgress(true);
        ui->DownloadedPictureList->viewport()->update();
    }

    void onMassDownloadFinished() {
        m_delegate->setMassDownloadInProgress(false);
        ui->DownloadedPictureList->viewport()->update();
    }

signals:
    void pictureDeleted();
    void openPicture(const Picture& picture);
    void searchTextChanged(const QString &text);
    void viewModeToggled(ImageCardDelegate::ViewMode mode);

private:
    void updateCompleterList();
    void setupConnections();
    void updateViews();
    void onDownloadProgress(int progress, const QString& pictureName);
    bool m_massDownloadInProgress = false;


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
