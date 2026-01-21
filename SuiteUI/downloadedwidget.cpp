#include "DownloadedWidget.h"
#include "ui_DownloadedWidget.h"
#include <QMessageBox>
#include <QItemSelectionModel>
#include <QCompleter>
#include <QStringListModel>

DownloadedWidget::DownloadedWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::DownloadedWidget), m_model(new QStandardItemModel(this))
{
    ui->setupUi(this);

    // Configurar el autocompletado
    m_completer = new QCompleter(this);
    m_completerModel = new QStringListModel(this);
    m_completer->setModel(m_completerModel);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setFilterMode(Qt::MatchContains); // Busca en cualquier parte del texto
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    ui->searchLineEdit->setCompleter(m_completer);

    m_delegate = new ImageCardDelegate(this);
    ui->DownloadedPictureList->setItemDelegate(m_delegate);
    ui->DownloadedPictureList->setModel(m_model);
    ui->DownloadedPictureList->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // SELECCIÓN SIMPLE: Cambiar el texto del nameLabel al pinchar
    connect(ui->DownloadedPictureList->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [this](const QItemSelection &selected) {
                if (!selected.indexes().isEmpty()) {
                    QString name = selected.indexes().first().data(Qt::DisplayRole).toString();
                    ui->namelabel->setText(name);
                }
            });

    // FILTROS DINÁMICOS
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &DownloadedWidget::refreshList);
    connect(ui->radioButton, &QRadioButton::toggled, this, &DownloadedWidget::refreshList);
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &DownloadedWidget::searchTextChanged);

    // DOBLE CLICK: Abrir visor
    connect(m_delegate, &ImageCardDelegate::doubleClicked, this, [this](const QModelIndex &idx){
        auto downloaded = m_pictureManager->downloaded();
        if (idx.row() < downloaded.size())
            emit openPicture(downloaded.at(idx.row()));
    });

    // CLIC EN 'i': Info
    connect(m_delegate, &ImageCardDelegate::infoRequested, this, [this](const QModelIndex &idx){
        const auto &pic = m_pictureManager->downloaded().at(idx.row());
        QString info = QString("Nombre: %1\nRuta: %2").arg(pic.nombre(), pic.url());
        QMessageBox::information(this, "Info", info);
    });

    // FAVORITOS Y BORRAR
    connect(m_delegate, &ImageCardDelegate::favoriteToggled, this, [this](const QModelIndex &idx){
        m_pictureManager->toggleFavorite(idx.row());
        refreshList();
    });

    connect(m_delegate, &ImageCardDelegate::deleteRequested, this, [this](const QModelIndex &idx){
        auto downloaded = m_pictureManager->downloaded();
        if (idx.row() < downloaded.size()) {
            m_pictureManager->removeDownloaded(downloaded.at(idx.row()));
            refreshList();
            emit pictureDeleted();
        }
    });

    // CAMBIO DE VISTA (Cuadrícula / Lista)
    connect(ui->toggleViewButton, &QPushButton::clicked, this, [this](){
        auto mode = (m_delegate->viewMode() == ImageCardDelegate::Grid) ? ImageCardDelegate::List : ImageCardDelegate::Grid;
        m_delegate->setViewMode(mode);
        ui->DownloadedPictureList->setViewMode(mode == ImageCardDelegate::Grid ? QListView::IconMode : QListView::ListMode);
        ui->DownloadedPictureList->doItemsLayout();

         emit viewModeToggled(mode);
    });
}

void DownloadedWidget::refreshList() {
    m_model->clear();
    if (!m_pictureManager) return;

    QString searchText = ui->searchLineEdit->text().toLower();
    bool onlyFavorites = ui->radioButton->isChecked();

    for(const auto &pic : m_pictureManager->downloaded()) {
        if(onlyFavorites && !pic.favorito()) continue;
        if(!searchText.isEmpty() && !pic.nombre().toLower().contains(searchText)) continue;

        QStandardItem *item = new QStandardItem(pic.nombre());
        item->setData(QIcon(pic.url()), Qt::DecorationRole);
        item->setData(pic.favorito(), Qt::UserRole + 1);
        item->setData(true, Qt::UserRole + 2);
        m_model->appendRow(item);
    }

    // Actualizar la lista de autocompletado
    updateCompleterList();
}

void DownloadedWidget::updateCompleterList() {
    if (!m_pictureManager) return;

    QStringList names;
    for(const auto &pic : m_pictureManager->downloaded()) {
        names << pic.nombre();
    }

    // Eliminar duplicados (por si acaso)
    names.removeDuplicates();
    names.sort(Qt::CaseInsensitive);

    m_completerModel->setStringList(names);
}

void DownloadedWidget::setPictureManager(PictureManager *manager) {
    m_pictureManager = manager;
    refreshList();
}

DownloadedWidget::~DownloadedWidget() {
    delete ui;
}
