#include "mainwindow.h"
#include <QTranslator>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    /**
     * =========================
     * DETECCIÓN DE IDIOMA
     * =========================
     */

    // Obtiene la configuración regional del sistema
    QLocale locale;

    // Extrae el idioma base ("es", "en", etc.)
    // Ejemplo: "es_ES" -> "es"
    QString language = locale.name().section('_', 0, 0);
    qDebug() << "Idioma del sistema:" << language;

    /**
     * =========================
     * CARGA DE TRADUCCIONES
     * =========================
     */

    // Objeto encargado de traducir cadenas usando archivos .qm
    QTranslator translator;

    // Si el idioma del sistema es español, intenta cargar la traducción
    if(language == "es") {
        // Carga el archivo de traducción desde los recursos Qt
        if(translator.load(":/translations/translations/app_es.qm")) {
            // Instala el traductor en la aplicación
            a.installTranslator(&translator);
            qDebug() << "Traducción española cargada.";
        } else {
            // Debug en caso de error de carga
            qDebug() << "No se pudo cargar app_es.qm";
        }
    }


    MainWindow w;
    w.show();
    return a.exec();
}
