#include "DownloadWidget.h"
#include "ui_DownloadWidget.h"
#include <QMessageBox>
#include <QDebug>

DownloadWidget::DownloadWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DownloadWidget)
    , m_model(new QStandardItemModel(this))
{
    ui->setupUi(this);

    ui->DownloadPictureList->setModel(m_model);
    ui->DownloadPictureList->setIconSize(QSize(100, 100)); // tamaño miniaturas
    ui->DownloadPictureList->setViewMode(QListView::IconMode);
    ui->DownloadPictureList->setResizeMode(QListView::Adjust);
    ui->DownloadPictureList->setGridSize(QSize(120, 120)); // ajusta tamaño de celda
    ui->DownloadPictureList->setIconSize(QSize(100, 100)); // tamaño de la imagen


    ui->progressBar->setValue(0);
    ui->DownloadButton->setEnabled(false);

    connect(ui->DownloadPictureList->selectionModel(),&QItemSelectionModel::currentChanged,this,&DownloadWidget::onSelectionChanged);
    connect(ui->DownloadButton, &QPushButton::clicked,this, &DownloadWidget::onDownloadClicked);
    connect(ui->infoButton, &QPushButton::clicked,this, &DownloadWidget::onInfoClicked);
    connect(m_pictureManager, &PictureManager::downloadProgress,this, &DownloadWidget::onDownloadProgress);
    connect(m_pictureManager, &PictureManager::pictureDownloaded,this, &DownloadWidget::onPictureDownloaded);
}

DownloadWidget::~DownloadWidget()
{
    delete ui;
}

void DownloadWidget::setPictureManager(PictureManager *manager)
{
    m_pictureManager = manager;

    connect(manager, &PictureManager::downloadProgress,
            this, &DownloadWidget::onDownloadProgress);

    connect(manager, &PictureManager::pictureDownloaded,
            this, &DownloadWidget::onPictureDownloaded);

    refreshList();
}

void DownloadWidget::refreshList()
{
    if (!m_pictureManager) return;

    m_model->clear();

    const QList<Picture>& pics = m_pictureManager->toDownload();

    for (const Picture& pic : pics) {
        QStandardItem* item = new QStandardItem(pic.nombre());

        QPixmap pixmap;
        if (!pixmap.load(pic.url())) {
            qDebug() << "No se pudo cargar la imagen:" << pic.url();
        } else {
            item->setIcon(QIcon(pixmap));
        }

        m_model->appendRow(item);
    }

    ui->progressBar->setValue(0);
    ui->DownloadButton->setEnabled(false);
    m_currentIndex = -1;
}

void DownloadWidget::onSelectionChanged(const QModelIndex &current)
{
    m_currentIndex = current.row();
    if (m_currentIndex < 0) return;

    const Picture& pic = m_pictureManager->toDownload().at(m_currentIndex);
    ui->nameLabel->setText(pic.nombre());
    ui->DownloadButton->setEnabled(true);
}

void DownloadWidget::onDownloadClicked()
{
    if (!m_pictureManager || m_currentIndex < 0) return;

    ui->progressBar->setValue(0);
    ui->DownloadButton->setEnabled(false);

    // Inicia simulación de descarga
    m_pictureManager->downloadPicture(m_currentIndex);
}

void DownloadWidget::onInfoClicked()
{
    if (!m_pictureManager || m_currentIndex < 0) return;

    const Picture& pic = m_pictureManager->toDownload().at(m_currentIndex);

    QMessageBox::information(this,
                             pic.nombre(),
                             pic.descripcion());
}

void DownloadWidget::onDownloadProgress(int progress, const QString &pictureName)
{
    if (m_currentIndex < 0) return;

    const Picture& pic = m_pictureManager->toDownload().at(m_currentIndex);
    if (pic.nombre() == pictureName) {
        ui->progressBar->setValue(progress);
    }
}

void DownloadWidget::onPictureDownloaded(const Picture &picture)
{
    // Reiniciamos la progress bar
    ui->progressBar->setValue(0);
    ui->DownloadButton->setEnabled(true);

    // Refrescamos la lista de “para descargar”
    refreshList();

    // Avisamos a MainWindow / DownloadedWidget
    emit pictureDownloaded();
}

