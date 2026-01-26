#include "mainwindow.h"
#include "qevent.h"
#include "ui_mainwindow.h"
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

/**
 * @file mainwindow.cpp
 * @brief Ventana principal que orquesta los widgets de descarga y descargadas.
 *
 * MainWindow se encarga de:
 * - localizar la ruta base del proyecto (getProjectPath),
 * - inicializar PictureManager y cargar catálogo / estado descargado,
 * - conectar DownloadWidget <-> DownloadedWidget <-> ImageViewer,
 * - persistir el estado de descargadas en el cierre de la aplicación.
 *
 * Comentarios en español estilo Doxygen para facilitar lectura y mantenimiento.
 */

/**
 * @brief Constructor.
 *
 * - Inicializa la UI,
 * - crea la carpeta "images" si no existe en la ruta del proyecto,
 * - carga los JSON (catalog y downloaded),
 * - asigna el PictureManager a los widgets correspondientes y conecta señales entre ellos,
 * - realiza un refresco inicial de las listas.
 *
 * @param parent Widget padre (por defecto nullptr).
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), imageViewer(new ImageViewer(this))
{
    ui->setupUi(this);

    // Obtener la ruta del proyecto (se busca hacia arriba desde el ejecutable)
    QString projectPath = getProjectPath();

    // Crear carpeta "images" si no existe (donde se guardarán las imágenes descargadas)
    QDir dir(projectPath);
    if (!dir.exists("images")) {
        dir.mkdir("images");
    }

    // Rutas a los ficheros JSON (catálogo y descargadas)
    QString catalogPath = QDir(projectPath).filePath("download.json");
    QString downloadedPath = QDir(projectPath).filePath("downloaded.json");

    // Inicializar PictureManager con la ruta base y cargar datos
    m_pictureManager.setBasePath(projectPath);
    m_pictureManager.loadCatalog(catalogPath);
    m_pictureManager.loadDownloaded(downloadedPath);

    // Asignar el manager a los widgets de la UI
    ui->downloadWidget->setPictureManager(&m_pictureManager);
    ui->downloadedWidget->setPictureManager(&m_pictureManager);

    // Conexiones entre widgets:
    // - Cuando se descarga una picture, refrescar la lista de descargadas.
    connect(ui->downloadWidget, &DownloadWidget::pictureDownloaded, ui->downloadedWidget, &DownloadedWidget::refreshList);

    // - Cuando se borra una picture desde descargadas, refrescar la lista de disponibles.
    connect(ui->downloadedWidget, &DownloadedWidget::pictureDeleted, ui->downloadWidget, &DownloadWidget::refreshList);

    // - Abrir el visor al solicitarlo desde descargadas.
    connect(ui->downloadedWidget, &DownloadedWidget::openPicture, imageViewer, &ImageViewer::showPicture);

    // - Sincronizar filtros y modo de vista entre widgets.
    connect(ui->downloadedWidget, &DownloadedWidget::searchTextChanged, ui->downloadWidget, &DownloadWidget::applyExternalFilter);
    connect(ui->downloadedWidget, &DownloadedWidget::viewModeToggled, ui->downloadWidget, &DownloadWidget::applyExternalViewMode);

    // Refrescos iniciales para poblar las vistas.
    ui->downloadWidget->refreshList();
    ui->downloadedWidget->refreshList();
}

/**
 * @brief Busca la ruta raíz del proyecto subiendo desde el directorio del ejecutable.
 *
 * La función recorre hasta 10 niveles hacia arriba buscando indicadores de proyecto:
 * - existencia de SuiteCore/SuiteCore.pro o SuiteCore/picturemanager.h
 * - existencia de SuiteUI/SuiteUI.pro o SuiteUI/mainwindow.h
 *
 * Si encuentra ambos, devuelve la ruta donde residen; si no, devuelve el directorio
 * del ejecutable como fallback.
 *
 * @return QString Ruta al directorio raíz del proyecto o directorio del ejecutable.
 */
QString MainWindow::getProjectPath()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QDir searchDir(appDir);

    // Subir niveles hasta encontrar la raíz del proyecto (máx 10 niveles)
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

    // Si no se encuentra la estructura del repo, usar el directorio del ejecutable
    return appDir;
}

/**
 * @brief Maneja el evento de cierre de la ventana.
 *
 * Antes de cerrar persiste el estado actual de las imágenes descargadas llamando
 * a PictureManager::saveDownloaded(...).
 *
 * @param event Evento de cierre (permitir/ignorar según lógica).
 */
void MainWindow::closeEvent(QCloseEvent *event) {
    QString projectPath = getProjectPath();
    QString downloadedPath = QDir(projectPath).filePath("downloaded.json");
    m_pictureManager.saveDownloaded(downloadedPath);
    event->accept();
}

/**
 * @brief Destructor.
 *
 * Libera la UI. imageViewer tiene a MainWindow como padre, por lo que Qt lo
 * eliminará automáticamente; no obstante, la interfaz ui (generada) se elimina.
 */
MainWindow::~MainWindow() {
    delete ui;
}
