#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , imageViewer(new ImageViewer(this))

{
    // Establece el título de la ventana principal (traducible)
    setWindowTitle("Suite");
    // Establece el icono de la aplicación desde los recursos Qt
    setWindowIcon(QIcon(":/icons/logo.png"));


    ui->setupUi(this);

    // 1. Obtener ruta genérica a Documentos/ImageSuite
    QString docsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString projectPath = QDir(docsPath).filePath("ImageSuite");

    // 2. Crear carpetas si no existen
    QDir dir(projectPath);
    if (!dir.exists()) dir.mkpath(".");
    if (!dir.exists("images")) dir.mkdir("images");

    // 3. Configurar Manager con la ruta base antes de cargar nada
    m_pictureManager.setBasePath(projectPath);

    QString catalogPath = QDir(projectPath).filePath("download.json");
    QString downloadedPath = QDir(projectPath).filePath("downloaded.json");

    qDebug() << "Ruta de trabajo:" << projectPath;

    // 4. Cargar datos
    m_pictureManager.loadCatalog(catalogPath);
    m_pictureManager.loadDownloaded(downloadedPath);

    // 5. Pasar Manager a los widgets hijos
    ui->downloadWidget->setPictureManager(&m_pictureManager);
    ui->downloadedWidget->setPictureManager(&m_pictureManager);

    // 6. Conexiones de Sincronización
    connect(ui->downloadWidget, &DownloadWidget::pictureDownloaded, ui->downloadedWidget, &DownloadedWidget::refreshList);
    connect(ui->downloadedWidget, &DownloadedWidget::pictureDeleted, ui->downloadWidget, &DownloadWidget::refreshList);
    connect(ui->downloadedWidget, &DownloadedWidget::openPicture, imageViewer, &ImageViewer::showPicture);
    connect(ui->downloadedWidget, &DownloadedWidget::viewModeToggled, ui->downloadWidget, &DownloadWidget::setViewMode);
    connect(ui->downloadedWidget, &DownloadedWidget::searchTextChanged,ui->downloadWidget, &DownloadWidget::refreshWithSearch);

    // 7. Carga inicial de UI
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
