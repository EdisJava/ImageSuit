#ifndef DOWNLOADWIDGET_H
#define DOWNLOADWIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include "PictureManager.h"
#include "ImageCardDelegate.h"

namespace Ui { class DownloadWidget; }

class DownloadWidget : public QWidget {
    Q_OBJECT
public:
    explicit DownloadWidget(QWidget *parent = nullptr);
    ~DownloadWidget();
    void setPictureManager(PictureManager* manager);
    void refreshList();
    void setViewMode(ImageCardDelegate::ViewMode mode);

    // MÉTODOS PÚBLICOS PARA SINCRONIZACIÓN
    void applyExternalFilter(const QString &text);
    void applyExternalViewMode(ImageCardDelegate::ViewMode mode);

signals:

    void pictureDownloaded();

private slots:
    void onDownloadAllClicked();
    void onPictureDownloaded(const Picture& picture);
    void onDownloadProgress(int progress, const QString& pictureName);

private:
    Ui::DownloadWidget *ui;
    PictureManager* m_pictureManager = nullptr;
    QStandardItemModel* m_model;
    ImageCardDelegate* m_delegate;
    bool m_isDownloadingAll = false;
    QString m_externalFilter; // Guarda el filtro que viene de fuera
};
#endif
