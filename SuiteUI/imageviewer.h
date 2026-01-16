#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QWidget>
#include "Picture.h"

namespace Ui {
class ImageViewer;
}

class ImageViewer : public QWidget
{
    Q_OBJECT

public:
    explicit ImageViewer(QWidget *parent = nullptr);
    ~ImageViewer();

public slots:
    void showPicture(const Picture& picture);
    void resizeEvent(QResizeEvent* event);

private:
    Ui::ImageViewer *ui;
     QPixmap m_currentPixmap;
};

#endif // IMAGEVIEWER_H
