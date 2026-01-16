#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "downloadwidget.h"
#include "downloadedwidget.h"
#include <QDir>
#include "imageviewer.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , imageViewer(new ImageViewer(this))

{
    ui->setupUi(this);

    QString projectPath = QDir("C:/Users/Edgar/Documents/ImageSuite").absolutePath();
    QString catalogPath = QDir(projectPath).filePath("download.json");
    QString downloadedPath = QDir(projectPath).filePath("downloaded.json");

    qDebug() << "Ruta catalogo:" << catalogPath
             << "Existe?" << QFileInfo(catalogPath).exists();

    m_pictureManager.loadCatalog(catalogPath);
    m_pictureManager.loadDownloaded(downloadedPath);

    // 🔹 Cargar catálogo y descargadas
    m_pictureManager.loadCatalog(catalogPath);
    m_pictureManager.loadDownloaded(downloadedPath);

    // 🔹 Pasamos el core a ambos widgets
    ui->downloadWidget->setPictureManager(&m_pictureManager);
    ui->downloadedWidget->setPictureManager(&m_pictureManager);

    // 🔹 Sincronización entre widgets
    connect(ui->downloadWidget, &DownloadWidget::pictureDownloaded,ui->downloadedWidget, &DownloadedWidget::refreshList);
    connect(ui->downloadedWidget, &DownloadedWidget::pictureDeleted, ui->downloadWidget, &DownloadWidget::refreshList);
    connect(ui->downloadedWidget, &DownloadedWidget::pictureDeleted,ui->downloadWidget, &DownloadWidget::refreshList);
    connect(ui->downloadedWidget, &DownloadedWidget::openPicture,imageViewer, &ImageViewer::showPicture);

    // 🔹 Carga inicial
    ui->downloadWidget->refreshList();
    ui->downloadedWidget->refreshList();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_pictureManager.saveDownloaded("C:/Users/Edgar/Documents/ImageSuite/downloaded.json");
    QMainWindow::closeEvent(event);
}
