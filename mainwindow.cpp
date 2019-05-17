#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    pool = new QThreadPool(this);
    pool->setMaxThreadCount(10);
    timer = new QTimer(this);
          connect(timer, SIGNAL(timeout()), this, SLOT(onUpdate()));
          timer->start(500);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    qDebug() << "Select files...";
    files = QFileDialog::getOpenFileNames(
                            this,
                            "Select one or more files to open",
                            "",
                            "Images (*.jpg *.jpeg)");
    ui->listWidget->clear();
    ui->listWidget->addItems(files);
}

void MainWindow::on_pushButton_2_clicked()
{
    qDebug() << "Send data...";
    for(QString fn: files){
        QString to_write = ui->lineEdit->text();
        Worker *task = new Worker(this, fn, to_write, ui->ipAddrEdit->toPlainText(), ui->plainTextEdit_2->toPlainText().toUShort());
        task->setAutoDelete(true);
        pool->start(task);
    }

}
void MainWindow::onUpdate(){
    int works = pool->activeThreadCount();
    ui->thread_lbl->setText(QString::number(works));
}

void MainWindow::onWarn(const QString w){
    QMessageBox::warning(this, "Error:", w);
}
