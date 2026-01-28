#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "PictureManager.h"
#include "imageviewer.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;


private:
    Ui::MainWindow *ui;
    PictureManager m_pictureManager;
    ImageViewer* imageViewer;
     QString getProjectPath();
};

#endif // MAINWINDOW_H
