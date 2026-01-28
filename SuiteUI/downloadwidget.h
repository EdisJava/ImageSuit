#ifndef DOWNLOADWIDGET_H
#define DOWNLOADWIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include "PictureManager.h"
#include "ImageCardDelegate.h"
#include "qpushbutton.h"

namespace Ui { class DownloadWidget; }

class DownloadWidget : public QWidget {
    Q_OBJECT
public:
    explicit DownloadWidget(QWidget *parent = nullptr);
    ~DownloadWidget();
    void setPictureManager(PictureManager* manager);
    void refreshList();
    void setViewMode(ImageCardDelegate::ViewMode mode);

    void applyExternalFilter(const QString &text);
    void applyExternalViewMode(ImageCardDelegate::ViewMode mode);
    static void disableDragDrop(QAbstractItemView* view);

signals:

    void pictureDownloaded(const Picture &picture);
    void massDownloadStarted();
    void massDownloadFinished();

private slots:
    void onDownloadAllClicked();
    void onPictureDownloaded(const Picture& picture);
    void onDownloadProgress(int progress, const QString& pictureName);
    void downloadNextInMass();
    void setMassDownloadInProgress(bool inProgress) {
        if (m_deleteButton) {
            m_deleteButton->setEnabled(!inProgress);
        }
    }


private:
    Ui::DownloadWidget *ui;
    PictureManager* m_pictureManager = nullptr;
    QStandardItemModel* m_model;
    ImageCardDelegate* m_delegate;
    bool m_isDownloadingAll = false;
    QString m_externalFilter; // Guarda el filtro que viene de fuera
    QHash<QString, int> m_progressCache;
    int m_pendingDownloads = 0;
    QPushButton* m_deleteButton;

};
#endif
