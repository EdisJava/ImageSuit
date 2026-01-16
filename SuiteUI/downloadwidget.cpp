#include "DownloadWidget.h"
#include "qtimer.h"
#include "ui_DownloadWidget.h"
#include "ImageCardDelegate.h" // Importante para el diseño de tarjetas
#include <QMessageBox>
#include <QDebug>

DownloadWidget::DownloadWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DownloadWidget)
    , m_model(new QStandardItemModel(this))
    , m_currentIndex(-1) // Inicializamos a -1
{
    ui->setupUi(this);

    // --- Configuración de Galería de Tarjetas ---
    ui->DownloadPictureList->setModel(m_model);
    ui->DownloadPictureList->setItemDelegate(new ImageCardDelegate(this));
    ui->DownloadPictureList->setViewMode(QListView::IconMode);
    ui->DownloadPictureList->setResizeMode(QListView::Adjust);
    ui->DownloadPictureList->setSpacing(10);
    ui->DownloadPictureList->setMovement(QListView::Static);

    ui->progressBar->setValue(0);
    ui->DownloadButton->setEnabled(false);

    // Conectar selección corregida
    connect(ui->DownloadPictureList->selectionModel(),
            &QItemSelectionModel::currentChanged,
            this, &DownloadWidget::onSelectionChanged);

    connect(ui->DownloadButton, &QPushButton::clicked, this, &DownloadWidget::onDownloadClicked);
    connect(ui->infoButton, &QPushButton::clicked, this, &DownloadWidget::onInfoClicked);
}

DownloadWidget::~DownloadWidget()
{
    delete ui;
}

void DownloadWidget::setPictureManager(PictureManager *manager)
{
    m_pictureManager = manager;

    // Conectar señales del manager
    connect(manager, &PictureManager::downloadProgress, this, &DownloadWidget::onDownloadProgress);
    connect(manager, &PictureManager::pictureDownloaded, this, &DownloadWidget::onPictureDownloaded);

    refreshList();
}

void DownloadWidget::refreshList() {
    if (!m_pictureManager) return;

    m_model->clear();
    m_visibleIndexes.clear();
    m_currentIndex = -1;

    const QList<Picture>& pics = m_pictureManager->toDownload();

    for (int i = 0; i < pics.size(); ++i) {
        const Picture& pic = pics[i];

        QStandardItem* item = new QStandardItem(pic.nombre());

        // El delegado dibujará la tarjeta con esta imagen
        QPixmap pix(pic.url());
        if (!pix.isNull()) {
            item->setData(QIcon(pix), Qt::DecorationRole);
        }

        m_model->appendRow(item);
        m_visibleIndexes.append(i); // Guardamos el índice real del catálogo
    }

    ui->DownloadButton->setEnabled(false);
}

void DownloadWidget::onSelectionChanged(const QModelIndex &current)
{
    int viewRow = current.row();
    if (viewRow < 0 || viewRow >= m_visibleIndexes.size()) {
        m_currentIndex = -1;
        ui->DownloadButton->setEnabled(false);
        return;
    }

    // TRADUCCIÓN de índice de vista a índice real
    m_currentIndex = m_visibleIndexes[viewRow];

    const Picture& pic = m_pictureManager->toDownload().at(m_currentIndex);
    ui->nameLabel->setText(pic.nombre());
    ui->DownloadButton->setEnabled(true);
}

void DownloadWidget::onDownloadClicked()
{
    if (!m_pictureManager || m_currentIndex < 0) return;

    ui->progressBar->setValue(0);
    ui->DownloadButton->setEnabled(false);

    // Inicia la descarga usando el índice real mapeado
    m_pictureManager->downloadPicture(m_currentIndex);
}

void DownloadWidget::onInfoClicked()
{
    if (!m_pictureManager || m_currentIndex < 0) return;

    const Picture& pic = m_pictureManager->toDownload().at(m_currentIndex);
    QMessageBox::information(this, pic.nombre(), pic.descripcion());
}

void DownloadWidget::onDownloadProgress(int progress, const QString &pictureName)
{
    if (m_currentIndex < 0) return;

    const Picture& pic = m_pictureManager->toDownload().at(m_currentIndex);
    // Solo actualizamos la barra si el nombre coincide con el seleccionado
    if (pic.nombre() == pictureName) {
        ui->progressBar->setValue(progress);
    }
}

void DownloadWidget::onPictureDownloaded(const Picture &picture)
{
    Q_UNUSED(picture);
    ui->progressBar->setValue(100); // Aseguramos que llegue al final

    // Pequeña pausa estética antes de limpiar
    QTimer::singleShot(500, this, [this](){
        ui->progressBar->setValue(0);
        refreshList(); // Esto actualiza la galería quitando la foto descargada
        emit pictureDownloaded(); // Avisa a MainWindow
    });
}
