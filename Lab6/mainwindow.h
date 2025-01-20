#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void fetchCurrentTemperature();
    void fetchAverageTemperature();
    void fetchHistoryTemperature();
    void setupChart(const QJsonArray &temp);
    void handleNetworkReply(QNetworkReply *reply);

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *networkManager;
    QTimer *updateTimer;

    double currentTemperature;
    double averageTemperature;
};

#endif // MAINWINDOW_H
