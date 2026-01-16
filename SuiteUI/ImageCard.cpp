#include "ImageCard.h"
#include "ui_ImageCard.h"
#include <QPixmap>

ImageCard::ImageCard(const Picture &pic, bool isDownloaded, QWidget *parent) :
    QWidget(parent), ui(new Ui::ImageCard), m_picture(pic)
{
    ui->setupUi(this);
    ui->labelName->setText(pic.nombre());

    // Cargar imagen con SmoothTransformation
    QPixmap pix(pic.url());
    if (!pix.isNull()) {
        ui->labelThumbnail->setPixmap(pix.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    if (isDownloaded) {
        ui->btnAction->setText("Borrar");
        ui->btnAction->setStyleSheet("background-color: #e74c3c; color: white;");
        connect(ui->btnAction, &QPushButton::clicked, [this](){ emit deleteRequested(m_picture.nombre()); });

        // El botón de abrir (solo si está descargada)
        connect(ui->btnOpen, &QPushButton::clicked, [this](){ emit openRequested(m_picture); });
    } else {
        ui->btnAction->setText("Descargar");
        ui->btnAction->setStyleSheet("background-color: #2ecc71; color: white;");
        connect(ui->btnAction, &QPushButton::clicked, [this](){ emit downloadRequested(m_picture.nombre()); });
    }

    connect(ui->btnFav, &QPushButton::clicked, [this](){ emit favoriteRequested(m_picture.nombre()); });
}

ImageCard::~ImageCard() { delete ui; }
