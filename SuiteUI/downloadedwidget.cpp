#include "DownloadedWidget.h"
#include "ui_DownloadedWidget.h"

#include <QMessageBox>
#include <QTimer>
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
    ui->DownloadedPictureList->setViewMode(QListView::IconMode);

    // Bloqueo de arrastre estático
    DownloadedWidget::disableDragDrop(ui->DownloadedPictureList);

    // --- SELECCIÓN: Actualizar el nameLabel al seleccionar ---
    connect(ui->DownloadedPictureList->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex &current, const QModelIndex &previous) {
        Q_UNUSED(previous);
        if (current.isValid()) {
            // Suponiendo que tu label en el .ui se llama nameLabel
            ui->namelabel->setText(current.data(Qt::DisplayRole).toString());
        } else {
            ui->namelabel->setText("Ninguna imagen seleccionada");
        }
    });

    // Configuración Botón Filtro Corazón
    ui->btnFilterFavorites->setCheckable(true);
    ui->btnFilterFavorites->setStyleSheet("QPushButton:checked { background-color: rgba(255, 0, 0, 20); border-radius: 8px; }");

    connect(ui->btnFilterFavorites, &QPushButton::toggled, this, &DownloadedWidget::refreshList);
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, [this](const QString &text){
        refreshList();
        emit searchChanged(text);
    });

    connect(ui->toggleViewButton, &QPushButton::clicked, this, [this](){
        auto newMode = (m_delegate->viewMode() == ImageCardDelegate::Grid) ? ImageCardDelegate::List : ImageCardDelegate::Grid;
        m_delegate->setViewMode(newMode);
        ui->DownloadedPictureList->setViewMode(newMode == ImageCardDelegate::Grid ? QListView::IconMode : QListView::ListMode);
        ui->DownloadedPictureList->doItemsLayout();
        ui->DownloadedPictureList->viewport()->update();
        emit viewModeChanged(newMode);
    });

    connect(m_delegate, &ImageCardDelegate::doubleClicked, this, [this](const QModelIndex &idx){
        QString name = idx.data().toString();
        for(const auto &pic : m_pictureManager->downloaded()) {
            if(pic.nombre() == name) { emit openPicture(pic); break; }
        }
    });

    connect(m_delegate, &ImageCardDelegate::favoriteToggled, this, [this](const QModelIndex &idx){
        m_pictureManager->toggleFavoriteByName(idx.data().toString());
        refreshList();
    });

    connect(m_delegate, &ImageCardDelegate::infoRequested, this, [this](const QModelIndex &idx){
        QString name = idx.data().toString();
        for(const auto &p : m_pictureManager->downloaded()) {
            if(p.nombre() == name) {
                QMessageBox::information(this, "Info", "Nombre: " + p.nombre() + "\nURL: " + p.url());
                break;
            }
        }
    });

    connect(m_delegate, &ImageCardDelegate::deleteRequested, this, [this](const QModelIndex &idx){
        QString name = idx.data().toString();
        QTimer* timer = new QTimer(this);
        int* val = new int(0);
        connect(timer, &QTimer::timeout, this, [this, timer, val, idx, name]() mutable {
            *val += 10;
            if (*val <= 100) {
                m_model->setData(idx, *val, Qt::UserRole + 5);
            } else {
                timer->stop();
                m_pictureManager->removeDownloadedByName(name);
                refreshList();
                ui->namelabel->setText("-"); // Limpiar label al borrar
                emit pictureDeleted();
                timer->deleteLater();
                delete val;
            }
        });
        timer->start(20);
    });
}

void DownloadedWidget::disableDragDrop(QAbstractItemView* view) {
    if(!view) return;
    view->setDragEnabled(false);
    view->setAcceptDrops(false);
    view->setDragDropMode(QAbstractItemView::NoDragDrop);
}

void DownloadedWidget::refreshList() {
    m_model->clear();
    if (!m_pictureManager) return;
    bool onlyFavs = ui->btnFilterFavorites->isChecked();
    QString search = ui->searchLineEdit->text().toLower();
    for(const auto &pic : m_pictureManager->downloaded()) {
        if(onlyFavs && !pic.favorito()) continue;
        if(!search.isEmpty() && !pic.nombre().toLower().contains(search)) continue;
        QStandardItem *item = new QStandardItem(pic.nombre());
        item->setData(QIcon(pic.url()), Qt::DecorationRole);
        item->setData(pic.favorito(), Qt::UserRole + 1);
        item->setData(true, Qt::UserRole + 2);
        item->setData(-1, Qt::UserRole + 5);
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
