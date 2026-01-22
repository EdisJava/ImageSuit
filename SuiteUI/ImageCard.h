#ifndef IMAGECARD_H
#define IMAGECARD_H

#include <QWidget>
#include "Picture.h"

namespace Ui { class ImageCard; }

class ImageCard : public QWidget {
    Q_OBJECT
public:
    explicit ImageCard(const Picture &pic, bool isDownloaded, QWidget *parent = nullptr);
    ~ImageCard();

signals:
    void downloadRequested(const QString &name);
    void deleteRequested(const QString &name);
    void favoriteRequested(const QString &name);
    void infoRequested(const QString &name);
    void openRequested(const Picture &pic);

private:
    Ui::ImageCard *ui;
    Picture m_picture;
};

#endif
