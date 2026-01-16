#include "ImageViewer.h"
#include "ui_ImageViewer.h"
#include <QPixmap>

ImageViewer::ImageViewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageViewer)
{
    ui->setupUi(this);
    // Para que sea ventana emergente
    this->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);


    connect(ui->backButton, &QPushButton::clicked, this, &ImageViewer::close);

}

ImageViewer::~ImageViewer()
{
    delete ui;
}

/*void ImageViewer::showPicture(const Picture& picture)
{
    ui->nameLabel->setText(picture.nombre());
    ui->descriptionlabel->setText(picture.descripcion());

    QPixmap pixmap(picture.filePath());
    if (!pixmap.isNull()) {
        ui->ImageLabel->setPixmap(pixmap.scaled(ui->ImageLabel->size(),
                                                Qt::KeepAspectRatio,
                                                Qt::SmoothTransformation));
    }
    this->show();
}
*/
void ImageViewer::showPicture(const Picture &picture)
{
    // Nombre y descripción
    ui->nameLabel->setText(picture.nombre());
    ui->descriptionlabel->setText(picture.descripcion());

    // Intentamos cargar la imagen
    QPixmap pixmap(picture.url());
    if (pixmap.isNull()) {
        qDebug() << "No se pudo cargar la imagen:" << picture.url();
        ui->ImageLabel->setText("Imagen no encontrada");
        return;
    }

    m_currentPixmap = pixmap; // guardamos la imagen original

    // Ajustar pixmap al label
    ui->ImageLabel->setPixmap(
        pixmap.scaled(ui->ImageLabel->size(),
                      Qt::IgnoreAspectRatio,
                      Qt::SmoothTransformation)
        );
    this->show();

}

void ImageViewer::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    if (!m_currentPixmap.isNull()) {
        ui->ImageLabel->setPixmap(
            m_currentPixmap.scaled(ui->ImageLabel->size(),
                                   Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation)
            );
    }
}

