#include "DownloadWidget.h"
#include "ui_DownloadWidget.h"
#include <QMessageBox>
#include <QItemSelectionModel>
#include <QMetaObject>

DownloadWidget::DownloadWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::DownloadWidget), m_model(new QStandardItemModel(this))
{
    ui->setupUi(this);
    m_delegate = new ImageCardDelegate(this);
    ui->DownloadPictureList->setItemDelegate(m_delegate);
    ui->DownloadPictureList->setModel(m_model);

    // Evitar que el doble clic abra el editor de texto
    ui->DownloadPictureList->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // SELECCIÓN SIMPLE: Al pinchar una imagen, cambiar el texto del nameLabel
    connect(ui->DownloadPictureList->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [this](const QItemSelection &selected) {
        if (!selected.indexes().isEmpty()) {
            QString name = selected.indexes().first().data(Qt::DisplayRole).toString();
            ui->nameLabel->setText(name);
        }
    });

    // Botón de descarga masiva
    connect(ui->DownloadAllButton, &QPushButton::clicked, this, &DownloadWidget::onDownloadAllClicked);

    // DOBLE CLIC: Descargar imagen individual
    connect(m_delegate, &ImageCardDelegate::doubleClicked, this, [this](const QModelIndex &idx){
        if (!m_isDownloadingAll) {
            m_pictureManager->downloadPicture(idx.row());
        }
    });

    // CLIC EN INFO ('i'): Mostrar nombre y URL
    connect(m_delegate, &ImageCardDelegate::infoRequested, this, [this](const QModelIndex &idx){
        const auto &pic = m_pictureManager->toDownload().at(idx.row());
        QString info = QString("<b>Nombre:</b> %1<br><b>URL:</b> %2")
                        .arg(pic.nombre(), pic.url());
        QMessageBox::information(this, "Información de Catálogo", info);
    });
}

void DownloadWidget::onDownloadAllClicked() {
    if (!m_pictureManager || m_pictureManager->toDownload().isEmpty() || m_isDownloadingAll) return;

    m_isDownloadingAll = true;
    ui->DownloadAllButton->setEnabled(false);

    // Usamos invokeMethod con QueuedConnection para procesar la descarga
    // sin bloquear la GUI y sin errores de hilos.
    QMetaObject::invokeMethod(m_pictureManager, [this](){
        m_pictureManager->downloadPicture(0);
    }, Qt::QueuedConnection);
}

void DownloadWidget::onPictureDownloaded(const Picture &picture) {
    Q_UNUSED(picture);
    refreshList();

    if (m_isDownloadingAll) {
        if (!m_pictureManager->toDownload().isEmpty()) {
            // Siguiente descarga
            QMetaObject::invokeMethod(m_pictureManager, [this](){
                m_pictureManager->downloadPicture(0);
            }, Qt::QueuedConnection);
        } else {
            m_isDownloadingAll = false;
            ui->DownloadAllButton->setEnabled(true);
            ui->nameLabel->setText("Descargas finalizadas");
            QMessageBox::information(this, "Éxito", "Todas las imágenes se han descargado correctamente.");
        }
    }
    emit pictureDownloaded();
}

void DownloadWidget::refreshList() {
    m_model->clear();
    if (!m_pictureManager) return;

    for(const auto &pic : m_pictureManager->toDownload()) {
        QStandardItem *item = new QStandardItem(pic.nombre());
        item->setData(QIcon(pic.url()), Qt::DecorationRole);
        item->setData(QVariant(), Qt::UserRole + 1);
        item->setData(false, Qt::UserRole + 2);
        m_model->appendRow(item);
    }
}

void DownloadWidget::setPictureManager(PictureManager *manager) {
    m_pictureManager = manager;
    connect(manager, &PictureManager::pictureDownloaded, this, &DownloadWidget::onPictureDownloaded);
    connect(manager, &PictureManager::downloadProgress, this, &DownloadWidget::onDownloadProgress);
    refreshList();
}

void DownloadWidget::onDownloadProgress(int progress, const QString &name) {
    ui->progressBar->setValue(progress);
    ui->nameLabel->setText("Descargando: " + name);
}

void DownloadWidget::setViewMode(ImageCardDelegate::ViewMode mode) {
    m_delegate->setViewMode(mode);
    ui->DownloadPictureList->setViewMode(mode == ImageCardDelegate::Grid ? QListView::IconMode : QListView::ListMode);
    ui->DownloadPictureList->doItemsLayout();
}



DownloadWidget::~DownloadWidget() { delete ui; }
