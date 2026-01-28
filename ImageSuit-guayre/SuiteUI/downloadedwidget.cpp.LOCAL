#include "DownloadedWidget.h"
#include "ImageCardDelegate.h"
#include "ui_DownloadedWidget.h"

#include <QMessageBox>
#include <QPixmap>
#include <QDebug>
#include <QCompleter>
#include <QLineEdit>
#include <QTimer>

DownloadedWidget::DownloadedWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DownloadedWidget)
    , m_model(new QStandardItemModel(this))
    , m_currentIndex(-1) // Inicializar siempre a -1
{
    ui->setupUi(this);

    // --- Configuración de Galería ---
    ui->DownloadedPictureList->setModel(m_model);
    ui->DownloadedPictureList->setItemDelegate(new ImageCardDelegate(this));
    ui->DownloadedPictureList->setViewMode(QListView::IconMode);
    ui->DownloadedPictureList->setResizeMode(QListView::Adjust);
    ui->DownloadedPictureList->setSpacing(10);
    ui->DownloadedPictureList->setMovement(QListView::Static); // Evita mover iconos por error

    // Conectar selección
    connect(ui->DownloadedPictureList->selectionModel(),&QItemSelectionModel::currentChanged, this, &DownloadedWidget::onSelectionChanged);

    // Botones inicialmente desactivados
    ui->OpenButton->setEnabled(false);
    ui->FavButton->setEnabled(false);
    ui->deleteButton->setEnabled(false);

    // Conectar botones
    connect(ui->OpenButton, &QPushButton::clicked, this, &DownloadedWidget::onOpenClicked);
    connect(ui->FavButton, &QPushButton::clicked, this, &DownloadedWidget::onFavClicked);
    connect(ui->InfoButton, &QPushButton::clicked, this, &DownloadedWidget::onInfoClicked);
    connect(ui->deleteButton, &QPushButton::clicked, this, &DownloadedWidget::onDeleteClicked);
    connect(ui->radioButton, &QRadioButton::toggled, this, &DownloadedWidget::refreshList);
    connect(ui->searchButton, &QPushButton::clicked, this, &DownloadedWidget::onSearchClicked);


}

DownloadedWidget::~DownloadedWidget() {
    delete ui;
}
void DownloadedWidget::refreshList() {
    if (!m_pictureManager) return;

    m_model->clear();
    m_visibleIndexes.clear();

    const QList<Picture>& pics = m_pictureManager->downloaded();

    for (int i = 0; i < pics.size(); ++i) {
        const Picture& pic = pics[i];
        if (ui->radioButton->isChecked() && !pic.favorito()) continue;

        QStandardItem* item = new QStandardItem(pic.nombre());
        QPixmap pix(pic.url());
        if (!pix.isNull()) {
            item->setData(QIcon(pix), Qt::DecorationRole);
        }

        m_model->appendRow(item);
        m_visibleIndexes.append(i);
    }

    // Forzar que los botones se apaguen después de refrescar
    ui->OpenButton->setEnabled(false);
    ui->deleteButton->setEnabled(false);
    ui->FavButton->setEnabled(false);
}

void DownloadedWidget::onSelectionChanged(const QModelIndex &current) {
    int viewRow = current.row();
    if (viewRow < 0 || viewRow >= m_visibleIndexes.size()) {
        m_currentIndex = -1;
        ui->OpenButton->setEnabled(false);
        ui->FavButton->setEnabled(false);
        ui->deleteButton->setEnabled(false);
        return;
    }

    // Traducimos el índice de la vista al índice real del Manager
    m_currentIndex = m_visibleIndexes[viewRow];

    // Obtenemos la referencia a la foto
    const Picture& pic = m_pictureManager->downloaded().at(m_currentIndex);

    // Actualizamos la UI lateral
    ui->namelabel->setText(pic.nombre());
    ui->OpenButton->setEnabled(true);
    ui->FavButton->setEnabled(true);
    ui->deleteButton->setEnabled(true);
}

void DownloadedWidget::onOpenClicked() {
    // Verificamos que haya una selección válida
    if (m_currentIndex < 0 || m_currentIndex >= m_pictureManager->downloaded().size()) return;

    // Obtenemos el objeto Picture completo usando el índice real
    const Picture& pic = m_pictureManager->downloaded().at(m_currentIndex);

    // EMITIMOS el objeto entero, no solo el string de la URL
    emit openPicture(pic);
}

void DownloadedWidget::onFavClicked() {
    if (m_currentIndex == -1) return;

    // Alternar favorito en el manager usando el índice real
    m_pictureManager->toggleFavorite(m_currentIndex);

    const Picture& updatedPic = m_pictureManager->downloaded()[m_currentIndex];

    ui->statusLabel->setText(updatedPic.favorito() ? "Marcado como favorito" : "Quitado de favoritos");
    QTimer::singleShot(2000, [this]() { ui->statusLabel->clear(); });

    refreshList(); // Redibujar para aplicar filtros si el radioButton está activo
}

void DownloadedWidget::onInfoClicked() {
    if (m_currentIndex == -1) return;
    const Picture& pic = m_pictureManager->downloaded()[m_currentIndex];
    QMessageBox::information(this, pic.nombre(), pic.descripcion());
}

void DownloadedWidget::onDeleteClicked() {
    if (m_currentIndex < 0) return;

    // 1. Obtenemos el objeto que el usuario está viendo en la lista filtrada
    const Picture& pic = m_pictureManager->downloaded().at(m_currentIndex);

    // 2. Mandamos el OBJETO al manager (él sabrá encontrarlo en la lista maestra)
    m_pictureManager->removeDownloaded(pic);

    // 3. Limpiar selección y refrescar de golpe
    ui->DownloadedPictureList->selectionModel()->clearSelection();
    m_currentIndex = -1;
    ui->namelabel->setText("Seleccione una imagen");

    refreshList(); // Esto actualiza la galería visualmente
    emit pictureDeleted(); // Avisa a otros widgets
}

void DownloadedWidget::onSearchClicked() {
    if (!m_pictureManager) return;

    QString searchText = ui->lineEdit->text().trimmed();

    m_model->clear();
    m_visibleIndexes.clear();
    m_currentIndex = -1;

    const QList<Picture>& pics = m_pictureManager->downloaded();

    for (int i = 0; i < pics.size(); ++i) {
        const Picture& pic = pics[i];

        // Filtros combinados (Favoritos + Texto)
        if (ui->radioButton->isChecked() && !pic.favorito()) continue;
        if (!searchText.isEmpty() && !pic.nombre().contains(searchText, Qt::CaseInsensitive)) continue;

        QStandardItem* item = new QStandardItem(pic.nombre());

        // Asignar icono si la imagen existe
        QPixmap pix(pic.url());
        if (!pix.isNull()) {
            // Escalamos la miniatura a 64x64 (puedes cambiar el tamaño)
            QIcon icon(pix.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            item->setData(icon, Qt::DecorationRole);
        }

        m_model->appendRow(item);
        m_visibleIndexes.append(i);
    }
}


void DownloadedWidget::setPictureManager(PictureManager* manager) {
    m_pictureManager = manager;

    // Construcción de autocompletado
    QStringList pictureNames;
    for (const Picture& pic : m_pictureManager->downloaded()) {
        pictureNames << pic.nombre();
    }

    QCompleter* completer = new QCompleter(pictureNames, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains); // coincidencia parcial
    ui->lineEdit->setCompleter(completer);

    // Conectar para filtrar la lista mientras escribes
    connect(ui->lineEdit, &QLineEdit::textChanged,this, &DownloadedWidget::onSearchClicked);

    refreshList();
}
