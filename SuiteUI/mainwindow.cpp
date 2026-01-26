#include "mainwindow.h"
#include "qevent.h"
#include "ui_mainwindow.h"
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), imageViewer(new ImageViewer(this))
{
    ui->setupUi(this);

    // Obtener la ruta del proyecto
    QString projectPath = getProjectPath();

    // Crear carpeta images si no existe
    QDir dir(projectPath);
    if (!dir.exists("images")) {
        dir.mkdir("images");
    }

    // Configurar rutas de archivos JSON
    QString catalogPath = QDir(projectPath).filePath("download.json");
    QString downloadedPath = QDir(projectPath).filePath("downloaded.json");

    m_pictureManager.setBasePath(projectPath);
    m_pictureManager.loadCatalog(catalogPath);
    m_pictureManager.loadDownloaded(downloadedPath);

    // ASIGNAR MANAGER
    ui->downloadWidget->setPictureManager(&m_pictureManager);
    ui->downloadedWidget->setPictureManager(&m_pictureManager);

    // --- CONEXIONES ENTRE WIDGETS ---
    connect(ui->downloadWidget, &DownloadWidget::pictureDownloaded,
            ui->downloadedWidget, &DownloadedWidget::refreshList);

    connect(ui->downloadedWidget, &DownloadedWidget::pictureDeleted,
            ui->downloadWidget, &DownloadWidget::refreshList);

    connect(ui->downloadedWidget, &DownloadedWidget::openPicture,
            imageViewer, &ImageViewer::showPicture);

    connect(ui->downloadedWidget, &DownloadedWidget::searchTextChanged,
            ui->downloadWidget, &DownloadWidget::applyExternalFilter);

    connect(ui->downloadedWidget, &DownloadedWidget::viewModeToggled,
            ui->downloadWidget, &DownloadWidget::applyExternalViewMode);

    // Refresco inicial
    ui->downloadWidget->refreshList();
    ui->downloadedWidget->refreshList();
}

QString MainWindow::getProjectPath()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QDir searchDir(appDir);

    // Subir niveles hasta encontrar la raíz del proyecto
    for (int i = 0; i < 10; ++i) {
        bool hasSuiteCore = searchDir.exists("SuiteCore/SuiteCore.pro") ||
                            searchDir.exists("SuiteCore/picturemanager.h");
        bool hasSuiteUI = searchDir.exists("SuiteUI/SuiteUI.pro") ||
                          searchDir.exists("SuiteUI/mainwindow.h");

        if (hasSuiteCore && hasSuiteUI) {
            return searchDir.absolutePath();
        }

        if (!searchDir.cdUp()) break;
    }

    // Fallback al directorio del ejecutable
    return appDir;
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    QString projectPath = getProjectPath();
    QString downloadedPath = QDir(projectPath).filePath("downloaded.json");
    m_pictureManager.saveDownloaded(downloadedPath);
    event->accept();
}
