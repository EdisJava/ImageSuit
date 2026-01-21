#include "DownloadedWidget.h"
#include "ui_DownloadedWidget.h"

#include <QMessageBox>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QRadioButton>
#include <QCompleter>
#include <QStringListModel>
#include <QSortFilterProxyModel>

DownloadedWidget::DownloadedWidget(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::DownloadedWidget),
    m_model(new QStandardItemModel(this)),
    m_proxyModel(new QSortFilterProxyModel(this))
{
    ui->setupUi(this);

    // --------------------------
    // Configurar proxy y modelo
    // --------------------------
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setFilterKeyColumn(0); // filtra por nombre
    ui->DownloadedPictureList->setModel(m_proxyModel);

    // --------------------------
    // Configurar delegado
    // --------------------------
    m_delegate = new ImageCardDelegate(this);
    ui->DownloadedPictureList->setItemDelegate(m_delegate);
    ui->DownloadedPictureList->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // --------------------------
    // Configurar completer
    // --------------------------
    m_completer = new QCompleter(this);
    m_completerModel = new QStringListModel(this);
    m_completer->setModel(m_completerModel);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setFilterMode(Qt::MatchContains);
    ui->searchLineEdit->setCompleter(m_completer);

    // --------------------------
    // Conexión búsqueda -> proxy
    // --------------------------
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, [this](const QString &text){
        m_proxyModel->setFilterFixedString(text);
        emit searchTextChanged(text);
    });

    connect(ui->radioButton, &QRadioButton::toggled, this, &DownloadedWidget::refreshList);

    // --------------------------
    // Doble click -> abrir imagen
    // --------------------------
    connect(m_delegate, &ImageCardDelegate::doubleClicked, this, [this](const QModelIndex &idx){
        QModelIndex sourceIdx = m_proxyModel->mapToSource(idx);
        if (sourceIdx.isValid() && m_pictureManager) {
            int realIndex = sourceIdx.data(Qt::UserRole).toInt();
            emit openPicture(m_pictureManager->allPictures().at(realIndex));
        }
    });

    // --------------------------
    // Info
    // --------------------------
    connect(m_delegate, &ImageCardDelegate::infoRequested, this, [this](const QModelIndex &idx){
        QModelIndex sourceIdx = m_proxyModel->mapToSource(idx);
        if (sourceIdx.isValid() && m_pictureManager) {
            int realIndex = sourceIdx.data(Qt::UserRole).toInt();
            const Picture &pic = m_pictureManager->allPictures().at(realIndex);
            QString info = QString("Nombre: %1\nRuta: %2").arg(pic.nombre(), pic.url());
            QMessageBox::information(this, "Info", info);
        }
    });

    // --------------------------
    // Favorito
    // --------------------------
    connect(m_delegate, &ImageCardDelegate::favoriteToggled, this, [this](const QModelIndex &idx){
        QModelIndex sourceIdx = m_proxyModel->mapToSource(idx);
        if (sourceIdx.isValid() && m_pictureManager) {
            int realIndex = sourceIdx.data(Qt::UserRole).toInt();
            m_pictureManager->toggleFavorite(realIndex);
            refreshList();
        }
    });

    // --------------------------
    // Borrar
    // --------------------------
    connect(m_delegate, &ImageCardDelegate::deleteRequested, this, [this](const QModelIndex &idx){
        QModelIndex sourceIdx = m_proxyModel->mapToSource(idx);
        if (sourceIdx.isValid() && m_pictureManager) {
            int realIndex = sourceIdx.data(Qt::UserRole).toInt();
            const Picture &pic = m_pictureManager->allPictures().at(realIndex);
            m_pictureManager->removeDownloaded(pic);
            refreshList();
            emit pictureDeleted();
        }
    });

    // --------------------------
    // Toggle vista (cuadrícula / lista)
    // --------------------------
    connect(ui->toggleViewButton, &QPushButton::clicked, this, [this](){
        auto mode = (m_delegate->viewMode() == ImageCardDelegate::Grid)
        ? ImageCardDelegate::List
        : ImageCardDelegate::Grid;

        m_delegate->setViewMode(mode);
        ui->DownloadedPictureList->setViewMode(mode == ImageCardDelegate::Grid
                                                   ? QListView::IconMode
                                                   : QListView::ListMode);
        ui->DownloadedPictureList->doItemsLayout();
        emit viewModeToggled(mode);
    });
}

// --------------------------
// Establecer PictureManager
// --------------------------
void DownloadedWidget::setPictureManager(PictureManager *manager) {
    m_pictureManager = manager;
    refreshList();
}

// --------------------------
// Refrescar lista de elementos visibles
// --------------------------
void DownloadedWidget::refreshList() {
    m_model->clear();
    if (!m_pictureManager) return;

    bool onlyFavorites = ui->radioButton->isChecked();

    const QList<Picture> &allPics = m_pictureManager->allPictures();

    for (int i = 0; i < allPics.size(); ++i) {
        const Picture &pic = allPics.at(i);

        if (!pic.descargada()) continue;       // solo descargadas
        if (onlyFavorites && !pic.favorito()) continue;

        QStandardItem* item = new QStandardItem(pic.nombre());
        item->setData(QIcon(pic.url()), Qt::DecorationRole);
        item->setData(pic.favorito(), Qt::UserRole + 1);
        item->setData(true, Qt::UserRole + 2);
        item->setData(i, Qt::UserRole); // índice real en PictureManager
        m_model->appendRow(item);
    }

    updateCompleterList();
}

// --------------------------
// Actualizar lista de autocompletado
// --------------------------
void DownloadedWidget::updateCompleterList() {
    if (!m_pictureManager) return;

    QStringList names;
    for (const Picture &pic : m_pictureManager->allPictures()) {
        if (pic.descargada())
            names << pic.nombre();
    }
    names.removeDuplicates();
    names.sort(Qt::CaseInsensitive);
    m_completerModel->setStringList(names);
}

DownloadedWidget::~DownloadedWidget() {
    delete ui;
}
