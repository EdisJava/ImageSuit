#include "DownloadedWidget.h"
#include "qtimer.h"
#include "ui_DownloadedWidget.h"

#include <QMessageBox>
#include <QPixmap>
#include <QDebug>

DownloadedWidget::DownloadedWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DownloadedWidget)
    , m_model(new QStandardItemModel(this))
{
    ui->setupUi(this);

    // 🔹 Configurar QListView como galería con iconos grandes
    ui->DownloadedPictureList->setModel(m_model);
    ui->DownloadedPictureList->setViewMode(QListView::IconMode);
    ui->DownloadedPictureList->setIconSize(QSize(150, 150));
    ui->DownloadedPictureList->setResizeMode(QListView::Adjust);
    ui->DownloadedPictureList->setSpacing(10);

    // Conectar selección
    connect(ui->DownloadedPictureList->selectionModel(),
            &QItemSelectionModel::currentChanged,
            this, &DownloadedWidget::onSelectionChanged);

    // Botones inicialmente desactivados
    ui->OpenButton->setEnabled(false);
    ui->FavButton->setEnabled(false);
    ui->deleteButton->setEnabled(false);

    // Conectar botones
    connect(ui->OpenButton, &QPushButton::clicked, this, &DownloadedWidget::onOpenClicked);
    connect(ui->FavButton, &QPushButton::clicked, this, &DownloadedWidget::onFavClicked);
    connect(ui->InfoButton, &QPushButton::clicked, this, &DownloadedWidget::onInfoClicked);
    connect(ui->deleteButton, &QPushButton::clicked, this, &DownloadedWidget::onDeleteClicked);
    connect(ui->radioButton, &QRadioButton::toggled,this, &DownloadedWidget::refreshList);
    connect(ui->searchButton, &QPushButton::clicked,this, &DownloadedWidget::onSearchClicked);


}

DownloadedWidget::~DownloadedWidget()
{
    delete ui;
}

void DownloadedWidget::refreshList()
{
    if (!m_pictureManager) return;

    m_model->clear(); // limpiamos la lista
    m_visibleIndexes.clear();

    const QList<Picture>& pics = m_pictureManager->downloaded();

    for (int i = 0; i < pics.size(); ++i) {
        const Picture& pic = pics[i];

        // Aplica filtro de favoritos
        if (ui->radioButton->isChecked() && !pic.favorito()) continue;

        // Creamos un item con texto y icono
        QStandardItem* item = new QStandardItem();
        item->setText(pic.nombre());
        QPixmap pix(pic.url());
        if (!pix.isNull())
            item->setIcon(QIcon(pix.scaled(64,64, Qt::KeepAspectRatio, Qt::SmoothTransformation)));

        m_model->appendRow(item);       // añadimos al modelo
        m_visibleIndexes.append(i);     // índice real
    }

    ui->DownloadedPictureList->setModel(m_model);

    ui->namelabel->clear();
    ui->OpenButton->setEnabled(false);
    ui->deleteButton->setEnabled(false);
    m_currentIndex = -1;
}

void DownloadedWidget::onSelectionChanged(const QModelIndex &current)
{
    int row = current.row();
    if (row < 0 || row >= m_visibleIndexes.size()) return;

    m_currentIndex = m_visibleIndexes[row];
    const Picture& pic = m_pictureManager->downloaded()[m_currentIndex];

    ui->namelabel->setText(pic.nombre());
    ui->OpenButton->setEnabled(true);
    ui->FavButton->setEnabled(true);
    ui->deleteButton->setEnabled(true);
}

void DownloadedWidget::onOpenClicked()
{
    if (m_currentIndex < 0 || m_currentIndex >= m_visibleIndexes.size()) return;

    int realIndex = m_visibleIndexes[m_currentIndex];
    const Picture& pic = m_pictureManager->pictures().at(realIndex);

    emit openPicture(pic);
}


void DownloadedWidget::onFavClicked()
{
    if (m_currentIndex < 0) return;

    Picture& pic = m_pictureManager->downloaded()[m_currentIndex];

    // Alternar favorito
    m_pictureManager->toggleFavorite(m_currentIndex);

    // Consultar el estado actualizado
    const Picture& updatedPic = m_pictureManager->downloaded()[m_currentIndex];

    // Mostrar mensaje
    if (updatedPic.favorito())
        ui->statusLabel->setText("Marcado como favorito");
    else
        ui->statusLabel->setText("Quitado de favoritos");

    // Opcional: borrar mensaje tras 2 segundos
    QTimer::singleShot(2000, [this]() { ui->statusLabel->clear(); });

    refreshList();
}

void DownloadedWidget::onInfoClicked()
{
    if (m_currentIndex < 0) return;
    const Picture& pic = m_pictureManager->downloaded()[m_currentIndex];
    QMessageBox::information(this, pic.nombre(), pic.descripcion());
}

void DownloadedWidget::onDeleteClicked()
{
    if (m_currentIndex < 0) return;

    // fila y columna para QStandardItemModel
    const QString name = m_model->data(m_model->index(m_currentIndex, 0), Qt::DisplayRole).toString();

    int realIndex = m_pictureManager->indexOf(name);
    if (realIndex != -1) {
        m_pictureManager->removeDownloaded(realIndex);
    }

    refreshList();
    emit pictureDeleted();
}


void DownloadedWidget::onSearchClicked()
{
    if (!m_pictureManager) return;

    QString searchText = ui->textEdit->toPlainText().trimmed(); // obtener texto

    m_model->clear();
    m_visibleIndexes.clear();
    m_currentIndex = -1;

    const QList<Picture>& pics = m_pictureManager->downloaded();
    bool filterFav = ui->radioButton->isChecked();

    for (int i = 0; i < pics.size(); ++i) {
        const Picture& pic = pics[i];

        // Filtrar por favoritos
        if (filterFav && !pic.favorito()) continue;

        // Filtrar por nombre (si hay texto)
        if (!searchText.isEmpty() && !pic.nombre().contains(searchText, Qt::CaseInsensitive))
            continue;

        QStandardItem* item = new QStandardItem();
        QPixmap pix(pic.url());
        if (!pix.isNull()) {
            item->setIcon(QIcon(pix.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
        }
        item->setText(pic.nombre());
        item->setToolTip(pic.descripcion());

        m_model->appendRow(item);
        m_visibleIndexes.append(i);
    }

    // Reset botones
    ui->OpenButton->setEnabled(false);
    ui->FavButton->setEnabled(false);
    ui->deleteButton->setEnabled(false);
}

void DownloadedWidget::setPictureManager(PictureManager* manager)
{
    m_pictureManager = manager;

    // Conectamos las señales
    connect(m_pictureManager, &PictureManager::pictureDownloaded,
            this, &DownloadedWidget::refreshList);

    connect(m_pictureManager, &PictureManager::pictureRemoved,
            this, &DownloadedWidget::refreshList);

    refreshList();
}


