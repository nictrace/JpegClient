#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QFileDialog>
#include <QModelIndexList>
#include <QThreadPool>
#include <QStringList>
#include <QTimer>
#include "worker.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void onUpdate();
    void onWarn(QString w);

private:
    Ui::MainWindow *ui;
    QThreadPool *pool;
    QStringList files;
    QTimer *timer;
};

#endif // MAINWINDOW_H
