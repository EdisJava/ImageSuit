#include "ImageViewer.h"
#include "ui_ImageViewer.h"
#include <QPixmap>
#include <QDebug>

ImageViewer::ImageViewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageViewer)
{
    ui->setupUi(this);

    // Configuración de ventana emergente
    this->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    this->setWindowTitle("Visor de Imagen");

    connect(ui->backButton, &QPushButton::clicked, this, &ImageViewer::close);
}

ImageViewer::~ImageViewer() {
    delete ui;
}

void ImageViewer::showPicture(const Picture &picture)
{
    // 1. Mostrar texto informativo
    ui->nameLabel->setText(picture.nombre());
    ui->descriptionlabel->setText(picture.descripcion());

    // 2. Intentar cargar la imagen desde la URL/Ruta
    QPixmap pixmap(picture.url());

    if (pixmap.isNull()) {
        qDebug() << "Error: No se pudo cargar la imagen en:" << picture.url();
        ui->ImageLabel->setText("No se pudo cargar el archivo de imagen");
        m_currentPixmap = QPixmap(); // Limpiar la imagen actual
    } else {
        m_currentPixmap = pixmap; // Guardamos la original para los resizeEvent

        // 3. Ajustar al tamaño actual del label manteniendo proporción
        ui->ImageLabel->setPixmap(
            m_currentPixmap.scaled(ui->ImageLabel->size(),
                                   Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation)
        );
    }

    this->show();
    this->raise(); // Traer al frente
    this->activateWindow();
}

void ImageViewer::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    // Si la ventana cambia de tamaño, reajustamos la foto para que no se vea pequeña
    if (!m_currentPixmap.isNull()) {
        ui->ImageLabel->setPixmap(
            m_currentPixmap.scaled(ui->ImageLabel->size(),
                                   Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation)
        );
    }
}
