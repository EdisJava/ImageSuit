#include "DownloadWidget.h"
#include "ui_DownloadWidget.h"
#include "DownloadedWidget.h"
#include <QMessageBox>
#include <QMetaObject>

DownloadWidget::DownloadWidget(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::DownloadWidget),
    m_model(new QStandardItemModel(this)),
    m_delegate(new ImageCardDelegate(this))
{
    ui->setupUi(this);

    // Configurar lista y delegado
    ui->DownloadPictureList->setModel(m_model);
    ui->DownloadPictureList->setItemDelegate(m_delegate);
    ui->DownloadPictureList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->DownloadPictureList->setViewMode(QListView::IconMode);

    DownloadedWidget::disableDragDrop(ui->DownloadPictureList);

    // Selección
    connect(ui->DownloadPictureList->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex &current, const QModelIndex &previous) {
                Q_UNUSED(previous);
                if (current.isValid()) {
                    ui->nameLabel->setText(current.data(Qt::DisplayRole).toString());
                }
            });

    // Botón "Download All"
    connect(ui->DownloadAllButton, &QPushButton::clicked, this, &DownloadWidget::onDownloadAllClicked);

    // Doble clic abrir
    connect(m_delegate, &ImageCardDelegate::doubleClicked, this, [this](const QModelIndex &idx){
        if (!m_isDownloadingAll) {
            const auto &pic = m_pictureManager->toDownload().at(idx.row());
            if (pic.isExpired()) {
                QMessageBox::warning(this, "Caducada", "Esta imagen ha caducado. Se descargará pero no podrá abrirse.");
            }
            m_model->setData(idx, 0, Qt::UserRole + 5);
            m_pictureManager->downloadPicture(idx.row());
        }
    });

    // Info
    connect(m_delegate, &ImageCardDelegate::infoRequested, this, [this](const QModelIndex &idx){
        auto list = m_pictureManager->toDownload();
        if(idx.row() < (int)list.size()) {
            QMessageBox::information(this, "Info", "URL: " + list.at(idx.row()).url());
        }
    });
}

// --- Sincronización externa ---
void DownloadWidget::applyExternalViewMode(ImageCardDelegate::ViewMode mode) {
    m_delegate->setViewMode(mode);
    ui->DownloadPictureList->setViewMode(mode == ImageCardDelegate::Grid ? QListView::IconMode : QListView::ListMode);
    ui->DownloadPictureList->doItemsLayout();
    ui->DownloadPictureList->viewport()->update();
}

void DownloadWidget::applyExternalFilter(const QString &text) {
    m_externalFilter = text.toLower();
    refreshList();
}

// --- Refresh lista ---
void DownloadWidget::refreshList() {
    m_model->clear();
    if (!m_pictureManager) return;

    for(const auto &pic : m_pictureManager->toDownload()) {
        if(!m_externalFilter.isEmpty() && !pic.nombre().toLower().contains(m_externalFilter)) continue;
        QStandardItem *item = new QStandardItem(pic.nombre());
        item->setData(QIcon(pic.url()), Qt::DecorationRole);
        item->setData(QVariant(), ImageCardDelegate::FavoriteRole);
        item->setData(false, ImageCardDelegate::DownloadedRole);
        item->setData(-1, ImageCardDelegate::ProgressRole);
        m_model->appendRow(item);
    }
}

// --- Descarga masiva ---
void DownloadWidget::onDownloadAllClicked() {
    if (!m_pictureManager || m_pictureManager->toDownload().isEmpty() || m_isDownloadingAll) return;
    m_isDownloadingAll = true;
    ui->DownloadAllButton->setEnabled(false);

    QMetaObject::invokeMethod(m_pictureManager, [this](){
        m_pictureManager->downloadPicture(0);
    }, Qt::QueuedConnection);
}

void DownloadWidget::onPictureDownloaded(const Picture &picture) {
    Q_UNUSED(picture);
    refreshList();
    if (m_isDownloadingAll) {
        if (!m_pictureManager->toDownload().isEmpty()) {
            QMetaObject::invokeMethod(m_pictureManager, [this](){
                m_pictureManager->downloadPicture(0);
            }, Qt::QueuedConnection);
        } else {
            m_isDownloadingAll = false;
            ui->DownloadAllButton->setEnabled(true);
            QMessageBox::information(this, "Éxito", "Descarga masiva completada.");
        }
    }
    emit pictureDownloaded();
}

void DownloadWidget::onDownloadProgress(int progress, const QString &name) {
    for (int i = 0; i < m_model->rowCount(); ++i) {
        if (m_model->item(i)->text() == name) {
            m_model->item(i)->setData(progress, Qt::UserRole + 5);
            break;
        }
    }
}

// --- Set manager ---
void DownloadWidget::setPictureManager(PictureManager *manager) {
    m_pictureManager = manager;

    connect(manager, &PictureManager::pictureDownloaded, this, &DownloadWidget::onPictureDownloaded);
    connect(manager, &PictureManager::downloadProgress, this, &DownloadWidget::onDownloadProgress);

    refreshList();
}

DownloadWidget::~DownloadWidget() { delete ui; }
