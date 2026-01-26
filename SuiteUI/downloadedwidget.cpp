#include <QCompleter>
#include <QStringListModel>
#include "DownloadedWidget.h"
#include "ui_DownloadedWidget.h"
#include <QMessageBox>
#include <QTimer>
#include <QLineEdit>
#include <QPushButton>

DownloadedWidget::DownloadedWidget(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::DownloadedWidget),
    m_downloadedModel(new QStandardItemModel(this)),
    m_downloadedProxy(new QSortFilterProxyModel(this)),
    m_delegate(new ImageCardDelegate(this))
{
    ui->setupUi(this);

    // Configurar proxy model para la lista de descargados
    m_downloadedProxy->setSourceModel(m_downloadedModel);
    m_downloadedProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_downloadedProxy->setFilterKeyColumn(0);

    // Asignar proxy y delegado a la lista
    ui->DownloadedPictureList->setModel(m_downloadedProxy);
    ui->DownloadedPictureList->setItemDelegate(m_delegate);
    ui->DownloadedPictureList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    disableDragDrop(ui->DownloadedPictureList);

    // Autocompletar
    m_completer = new QCompleter(this);
    m_completerModel = new QStringListModel(this);
    m_completer->setModel(m_completerModel);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setFilterMode(Qt::MatchContains);
    ui->searchLineEdit->setCompleter(m_completer);

    // Conexiones
    setupConnections();
}

void DownloadedWidget::setupConnections() {
    // Búsqueda
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, [this](const QString &text){
        m_downloadedProxy->setFilterFixedString(text);
        emit searchTextChanged(text);  // Para sincronizar con DownloadWidget
    });

    // Toggle vista
    connect(ui->toggleViewButton, &QPushButton::clicked, this, [this](){
        auto newMode = (m_delegate->viewMode() == ImageCardDelegate::Grid)
        ? ImageCardDelegate::List
        : ImageCardDelegate::Grid;

        m_delegate->setViewMode(newMode);
        ui->DownloadedPictureList->setViewMode(newMode == ImageCardDelegate::Grid ? QListView::IconMode : QListView::ListMode);
        ui->DownloadedPictureList->doItemsLayout();
        ui->DownloadedPictureList->viewport()->update();

        emit viewModeToggled(newMode);  // Para sincronizar con DownloadWidget
    });

    // Favoritos

    connect(m_delegate, &ImageCardDelegate::favoriteToggled, this, [this](const QModelIndex &idx){
        m_pictureManager->toggleFavoriteByName(idx.data().toString());
        refreshList();
    });


    // Info
    connect(m_delegate, &ImageCardDelegate::infoRequested, this, [this](const QModelIndex &idx){
        QModelIndex sourceIdx = m_downloadedProxy->mapToSource(idx);
        if (sourceIdx.isValid() && m_pictureManager) {
            const auto &pic = m_pictureManager->downloaded().at(sourceIdx.row());
            QMessageBox::information(this, "Info", "Nombre: " + pic.nombre() + "\nURL: " + pic.url());
        }
    });

    // Doble clic abrir (con control de caducidad)
    connect(m_delegate, &ImageCardDelegate::doubleClicked, this, [this](const QModelIndex &idx){
        QModelIndex sourceIdx = m_downloadedProxy->mapToSource(idx);
        if (sourceIdx.isValid() && m_pictureManager) {
            bool expired = sourceIdx.data(Qt::UserRole + 6).toBool(); // <-- revisa si está caducada
            if (expired) {
                QMessageBox::warning(this, "Caducada", "Esta imagen ha caducado y no se puede abrir.");
                return; // No abrir la imagen
            }
            const auto &pic = m_pictureManager->downloaded().at(sourceIdx.row());
            emit openPicture(pic);
        }
    });



    // Borrar
    connect(m_delegate, &ImageCardDelegate::deleteRequested, this, [this](const QModelIndex &idx){
        QModelIndex sourceIdx = m_downloadedProxy->mapToSource(idx);
        if (sourceIdx.isValid() && m_pictureManager) {
            const auto &pic = m_pictureManager->downloaded().at(sourceIdx.row());
            m_pictureManager->removeDownloaded(pic);
        }
    });
    // Conexión para mostrar progreso de eliminación
    connect(m_pictureManager, &PictureManager::downloadProgress,
            this, &DownloadedWidget::onDownloadProgress);

    // Mostrar nombre de la imagen seleccionada
    connect(ui->DownloadedPictureList->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex &current, const QModelIndex &previous){
                Q_UNUSED(previous);
                if(current.isValid() && m_downloadedProxy) {
                    QModelIndex sourceIdx = m_downloadedProxy->mapToSource(current);
                    ui->namelabel->setText(sourceIdx.data(Qt::DisplayRole).toString());
                } else {
                    ui->namelabel->setText("Ninguna imagen seleccionada");
                }
            });

    // Filtro botón favoritos
    connect(ui->btnFilterFavorites, &QPushButton::toggled, this, &DownloadedWidget::refreshList);
}

void DownloadedWidget::refreshList() {
    m_downloadedModel->clear();
    if (!m_pictureManager) return;

    QString search = ui->searchLineEdit->text().toLower();
    bool onlyFavs = ui->btnFilterFavorites->isChecked();
    QDate today = QDate::currentDate();

    for (const auto &pic : m_pictureManager->downloaded()) {
        if (onlyFavs && !pic.favorito()) continue;
        if (!search.isEmpty() && !pic.nombre().toLower().contains(search)) continue;

        QString displayName = pic.nombre();
        bool expired = false;

        if (pic.expirationDate().isValid() && pic.expirationDate() < today) {
            displayName += " (Caducada)";
            expired = true;
        }

        QStandardItem* item = new QStandardItem(displayName);
        item->setData(QIcon(pic.url()), Qt::DecorationRole);
        item->setData(pic.favorito(), ImageCardDelegate::FavoriteRole);
        item->setData(true, ImageCardDelegate::DownloadedRole);
        item->setData(-1, ImageCardDelegate::ProgressRole);
        item->setData(expired, ImageCardDelegate::ExpiredRole);
        m_downloadedModel->appendRow(item);
    }

    updateCompleterList();
}



void DownloadedWidget::updateCompleterList() {
    if (!m_pictureManager) return;

    QStringList names;
    for (const auto &pic : m_pictureManager->downloaded())
        names << pic.nombre();

    names.removeDuplicates();
    names.sort(Qt::CaseInsensitive);
    m_completerModel->setStringList(names);
}

void DownloadedWidget::setPictureManager(PictureManager *manager) {
    m_pictureManager = manager;

    // Desconectar conexión anterior si existe
    if (m_pictureManager) {
        disconnect(m_pictureManager, &PictureManager::downloadProgress,
                   this, &DownloadedWidget::onDownloadProgress);

        // Conectar señal de progreso
        connect(m_pictureManager, &PictureManager::downloadProgress,
                this, &DownloadedWidget::onDownloadProgress);
    }

    refreshList();
}
void DownloadedWidget::disableDragDrop(QAbstractItemView* view) {
    if (!view) return;
    view->setDragEnabled(false);
    view->setAcceptDrops(false);
    view->setDragDropMode(QAbstractItemView::NoDragDrop);
}

DownloadedWidget::~DownloadedWidget() {
    delete ui;
}

void DownloadedWidget::onDownloadProgress(int progress, const QString& pictureName) {
    if (progress == -1) {
        // Progreso terminado, refrescar después de un pequeño delay
        QTimer::singleShot(150, this, [this]() {
            refreshList();
            emit pictureDeleted();
        });
        return;
    }

    // Buscar el ítem correspondiente en el modelo
    for (int i = 0; i < m_downloadedModel->rowCount(); ++i) {
        QStandardItem* item = m_downloadedModel->item(i);
        if (item && item->text().contains(pictureName)) {
            // Actualizar el progreso
            item->setData(progress, ImageCardDelegate::ProgressRole);
            // Forzar actualización visual
            ui->DownloadedPictureList->viewport()->update();
            break;
        }
    }
}
