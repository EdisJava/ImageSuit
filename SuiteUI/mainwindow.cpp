#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), imageViewer(new ImageViewer(this))
{
    ui->setupUi(this);

    // ... (Tu código de rutas y PictureManager igual que antes) ...
    QString docsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString projectPath = QDir(docsPath).filePath("ImageSuite");
    QDir dir(projectPath);
    if (!dir.exists()) dir.mkpath(".");
    if (!dir.exists("images")) dir.mkdir("images");

    m_pictureManager.setBasePath(projectPath);
    m_pictureManager.loadCatalog(QDir(projectPath).filePath("download.json"));
    m_pictureManager.loadDownloaded(QDir(projectPath).filePath("downloaded.json"));

    // ASIGNAR MANAGER
    ui->downloadWidget->setPictureManager(&m_pictureManager);
    ui->downloadedWidget->setPictureManager(&m_pictureManager);

    // --- CONEXIONES ENTRE WIDGETS ---

    // 1. Si descarga termina -> refrescar lista de descargados
    connect(ui->downloadWidget, &DownloadWidget::pictureDownloaded, ui->downloadedWidget, &DownloadedWidget::refreshList);

    // 2. Si se borra una foto -> refrescar lista de descargas (por si vuelve a aparecer)
    connect(ui->downloadedWidget, &DownloadedWidget::pictureDeleted, ui->downloadWidget, &DownloadWidget::refreshList);

    // 3. Abrir visor
    connect(ui->downloadedWidget, &DownloadedWidget::openPicture, imageViewer, &ImageViewer::showPicture);

    // 4. SINCRONIZACIÓN BUSCADOR
    connect(ui->downloadedWidget, &DownloadedWidget::searchChanged,
            ui->downloadWidget, &DownloadWidget::applyExternalFilter);

    // 5. SINCRONIZACIÓN VISTA (GRID/LIST)
    connect(ui->downloadedWidget, &DownloadedWidget::viewModeChanged,
            ui->downloadWidget, &DownloadWidget::applyExternalViewMode);

    // Refresco inicial
    ui->downloadWidget->refreshList();
    ui->downloadedWidget->refreshList();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // Guarda automáticamente al cerrar usando la ruta genérica
    m_pictureManager.saveDownloaded(m_pictureManager.getDownloadedJsonPath());
    event->accept();
}
