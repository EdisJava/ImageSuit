/**
 * @file DownloadedWidget.cpp
 * @brief Widget que muestra las imágenes descargadas y proporciona acciones sobre ellas.
 *
 * Este widget encapsula:
 * - un modelo (QStandardItemModel) con un proxy (QSortFilterProxyModel) para búsqueda/filtrado,
 * - un delegado personalizado (ImageCardDelegate) para dibujar cada tarjeta,
 * - autocompletado para la búsqueda,
 * - controles para alternar vista, filtrar favoritos, mostrar info y borrar imágenes descargadas.
 *
 * Las responsabilidades principales son: mantener la lista visual actualizada (refreshList),
 * conectar señales/slots con PictureManager y propagar eventos (openPicture, pictureDeleted, ...).
 */

#include "DownloadedWidget.h"
#include "ui_DownloadedWidget.h"

#include <QCompleter>
#include <QStringListModel>
#include <QMessageBox>
#include <QTimer>
#include <QLineEdit>
#include <QPushButton>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QDate>
#include <QDebug>

/**
 * @brief Constructor.
 *
 * Inicializa UI, modelos, proxy, delegado y configura autocompletado y conexiones.
 *
 * @param parent Widget padre (por defecto nullptr).
 */
DownloadedWidget::DownloadedWidget(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::DownloadedWidget),
    m_downloadedModel(new QStandardItemModel(this)),
    m_downloadedProxy(new QSortFilterProxyModel(this)),
    m_delegate(new ImageCardDelegate(this)),
    m_completer(nullptr),
    m_completerModel(nullptr),
    m_pictureManager(nullptr)
{
    ui->setupUi(this);

    // Configurar proxy model para la lista de descargados
    m_downloadedProxy->setSourceModel(m_downloadedModel);
    m_downloadedProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_downloadedProxy->setFilterKeyColumn(0); // filtrar por texto mostrado (DisplayRole)

    // Asignar proxy y delegado a la vista
    ui->DownloadedPictureList->setModel(m_downloadedProxy);
    ui->DownloadedPictureList->setItemDelegate(m_delegate);
    ui->DownloadedPictureList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    disableDragDrop(ui->DownloadedPictureList);

    // Autocompletar para la búsqueda
    m_completer = new QCompleter(this);
    m_completerModel = new QStringListModel(this);
    m_completer->setModel(m_completerModel);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setFilterMode(Qt::MatchContains);
    ui->searchLineEdit->setCompleter(m_completer);

    // Conectar señales/slots locales
    setupConnections();
}

/**
 * @brief Configura las conexiones internas del widget.
 *
 * - TextChanged del QLineEdit actualiza el filtro del proxy y emite searchTextChanged.
 * - Toggle de vista alterna entre Grid/List en el delegado y la QListView.
 * - Conexiones con las señales del delegado (favoriteToggled, infoRequested, doubleClicked, deleteRequested).
 * - Conexión con PictureManager::downloadProgress para actualizar progresos visuales.
 */
void DownloadedWidget::setupConnections() {
    // Búsqueda: filtrar proxy y notificar (para sincronizar con otras vistas)
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, [this](const QString &text){
        m_downloadedProxy->setFilterFixedString(text);
        emit searchTextChanged(text);  // Para sincronizar con DownloadWidget si existe
    });

    // Toggle vista Grid <-> List
    connect(ui->toggleViewButton, &QPushButton::clicked, this, [this](){
        auto newMode = (m_delegate->viewMode() == ImageCardDelegate::Grid)
        ? ImageCardDelegate::List
        : ImageCardDelegate::Grid;

        m_delegate->setViewMode(newMode);
        ui->DownloadedPictureList->setViewMode(newMode == ImageCardDelegate::Grid ? QListView::IconMode : QListView::ListMode);

        // Forzar relayout y repaint para que el delegado vuelva a pintar en el nuevo modo
        ui->DownloadedPictureList->doItemsLayout();
        ui->DownloadedPictureList->viewport()->update();

        emit viewModeToggled(newMode);  // Para sincronizar con DownloadWidget u otros
    });

    // Favoritos: el delegado emite favoriteToggled con el QModelIndex visual.
    // Aquí mapeamos/actuamos sobre el PictureManager por nombre (simplificación existente).
    connect(m_delegate, &ImageCardDelegate::favoriteToggled, this, [this](const QModelIndex &idx){
        // Nota: el índice del delegado viene en coordenadas del proxy. Mantenemos la lógica
        // original que alterna favorito por nombre (toggleFavoriteByName).
        m_pictureManager->toggleFavoriteByName(idx.data().toString());
        refreshList();
    });

    // Info: mostrar cuadro con información básica
    connect(m_delegate, &ImageCardDelegate::infoRequested, this, [this](const QModelIndex &idx){
        QModelIndex sourceIdx = m_downloadedProxy->mapToSource(idx);
        if (sourceIdx.isValid() && m_pictureManager) {
            const auto &pic = m_pictureManager->downloaded().at(sourceIdx.row());
            QMessageBox::information(this, tr("Info"), tr("Nombre: %1\nURL: %2").arg(pic.nombre(), pic.url()));
        }
    });

    // Doble clic abrir (con control de caducidad)
    connect(m_delegate, &ImageCardDelegate::doubleClicked, this, [this](const QModelIndex &idx){
        QModelIndex sourceIdx = m_downloadedProxy->mapToSource(idx);
        if (sourceIdx.isValid() && m_pictureManager) {
            bool expired = sourceIdx.data(ImageCardDelegate::ExpiredRole).toBool();
            if (expired) {
                QMessageBox::warning(this, tr("Caducada"), tr("Esta imagen ha caducado y no se puede abrir."));
                return;
            }
            const auto &pic = m_pictureManager->downloaded().at(sourceIdx.row());
            emit openPicture(pic);
        }
    });

    // Borrar: pedir a PictureManager que elimine (marque como no descargada)
    connect(m_delegate, &ImageCardDelegate::deleteRequested, this, [this](const QModelIndex &idx){
        QModelIndex sourceIdx = m_downloadedProxy->mapToSource(idx);
        if (sourceIdx.isValid() && m_pictureManager) {
            const auto &pic = m_pictureManager->downloaded().at(sourceIdx.row());
            m_pictureManager->removeDownloaded(pic);
        }
    });

    // Mostrar progreso de descarga/eliminación
    connect(m_pictureManager, &PictureManager::downloadProgress,
            this, &DownloadedWidget::onDownloadProgress);

    // Mostrar nombre de la imagen seleccionada en la etiqueta
    connect(ui->DownloadedPictureList->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex &current, const QModelIndex &previous){
                Q_UNUSED(previous);
                if (current.isValid() && m_downloadedProxy) {
                    QModelIndex sourceIdx = m_downloadedProxy->mapToSource(current);
                    ui->namelabel->setText(sourceIdx.data(Qt::DisplayRole).toString());
                } else {
                    ui->namelabel->setText(tr("Ninguna imagen seleccionada"));
                }
            });

    // Botón filtro de favoritos: al alternar invoca refreshList()
    connect(ui->btnFilterFavorites, &QPushButton::toggled, this, &DownloadedWidget::refreshList);
}

/**
 * @brief Refresca la lista visual a partir de PictureManager::downloaded().
 *
 * Aplica filtros: búsqueda por texto y flag de "solo favoritos". Además marca
 * visualmente si una imagen está caducada usando la cadena "(Caducada)".
 */
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

/**
 * @brief Actualiza la lista usada por el QCompleter a partir de los nombres descargados.
 *
 * Elimina duplicados y ordena alfabéticamente (case-insensitive).
 */
void DownloadedWidget::updateCompleterList() {
    if (!m_pictureManager) return;

    QStringList names;
    for (const auto &pic : m_pictureManager->downloaded())
        names << pic.nombre();

    names.removeDuplicates();
    names.sort(Qt::CaseInsensitive);
    m_completerModel->setStringList(names);
}

/**
 * @brief Establece el PictureManager asociado y reconecta señales relevantes.
 *
 * @param manager Puntero al PictureManager (puede ser nullptr para desconectar).
 */
void DownloadedWidget::setPictureManager(PictureManager *manager) {
    // Desconectar anterior (si existía)
    if (m_pictureManager) {
        disconnect(m_pictureManager, &PictureManager::downloadProgress,
                   this, &DownloadedWidget::onDownloadProgress);
    }

    m_pictureManager = manager;

    // Conectar la nueva instancia si existe
    if (m_pictureManager) {
        connect(m_pictureManager, &PictureManager::downloadProgress,
                this, &DownloadedWidget::onDownloadProgress);
    }

    refreshList();
}

/**
 * @brief Desactiva drag & drop en una vista para evitar reordenamiento manual.
 *
 * @param view Puntero a la QAbstractItemView a ajustar.
 */
void DownloadedWidget::disableDragDrop(QAbstractItemView* view) {
    if (!view) return;
    view->setDragEnabled(false);
    view->setAcceptDrops(false);
    view->setDragDropMode(QAbstractItemView::NoDragDrop);
}

/**
 * @brief Slot que recibe y procesa progreso de descarga/eliminación.
 *
 * - Si progress == -1 => significa "operación finalizada", refresca la lista tras un pequeño delay
 *   y emite pictureDeleted.
 * - En otro caso, busca el item cuyo texto contiene pictureName y actualiza su ProgressRole,
 *   forzando un repaint del viewport.
 *
 * @param progress Valor de progreso (0-100, o -1 para finalizado).
 * @param pictureName Nombre (o fragmento) de la imagen asociada al progreso.
 */
void DownloadedWidget::onDownloadProgress(int progress, const QString& pictureName) {
    if (progress == -1) {
        // Progreso terminado: refrescar con pequeño delay para dejar que otras operaciones finalicen
        QTimer::singleShot(150, this, [this]() {
            refreshList();
            emit pictureDeleted();
        });
        return;
    }

    // Buscar el ítem correspondiente en el modelo y actualizar su rol ProgressRole
    for (int i = 0; i < m_downloadedModel->rowCount(); ++i) {
        QStandardItem* item = m_downloadedModel->item(i);
        if (item && item->text().contains(pictureName)) {
            // Actualizar el progreso (rol personalizado)
            item->setData(progress, ImageCardDelegate::ProgressRole);
            // Forzar actualización visual del viewport
            ui->DownloadedPictureList->viewport()->update();
            break;
        }
    }
}

/**
 * @brief Destructor.
 *
 * Libera recursos de la interfaz (ui) — los objetos hijos creados con `new` y `this`
 * como padre se eliminarán automáticamente por QObject.
 */
DownloadedWidget::~DownloadedWidget() {
    delete ui;
}
