#include "DownloadedWidget.h"
#include "ui_DownloadedWidget.h"
#include <QMessageBox>
#include <QTimer>

DownloadedWidget::DownloadedWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::DownloadedWidget), m_model(new QStandardItemModel(this))
{
    ui->setupUi(this);
    m_delegate = new ImageCardDelegate(this);

    ui->DownloadedPictureList->setItemDelegate(m_delegate);
    ui->DownloadedPictureList->setModel(m_model);
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
        m_model->appendRow(item);
    }
}

void DownloadedWidget::setPictureManager(PictureManager *manager) {
    m_pictureManager = manager;
    refreshList();
}

DownloadedWidget::~DownloadedWidget() { delete ui; }
