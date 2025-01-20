#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QGraphicsScene>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , networkManager(new QNetworkAccessManager(this))
    , updateTimer(new QTimer(this))
    , currentTemperature(0.0)
    , averageTemperature(0.0) {
    ui->setupUi(this);

    connect(updateTimer, &QTimer::timeout, this, &MainWindow::fetchCurrentTemperature);
    connect(ui->refreshButton, &QPushButton::clicked, this, &MainWindow::fetchHistoryTemperature);

    updateTimer->start(5000);

    connect(networkManager, &QNetworkAccessManager::finished, this, &MainWindow::handleNetworkReply);

    fetchCurrentTemperature();
    fetchAverageTemperature();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupChart(const QJsonArray &temp) {
    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->setSceneRect(ui->graphicsView->rect());

    QChart *chart = new QChart();
    chart->setBackgroundBrush(QBrush(QColor(255, 255, 255)));


    QLineSeries *series = new QLineSeries();
    QJsonObject firstTempObject = temp.first().toObject();
    QString firstTimestamp = firstTempObject["timestamp"].toString();
    QDateTime firstDateTime = QDateTime::fromString(firstTimestamp, "yyyy-MM-dd HH:mm:ss");

    qint64 maxTime = 0;
    double minTemp = std::numeric_limits<double>::max();
    double maxTemp = std::numeric_limits<double>::lowest();

    for (const QJsonValue &value : temp) {
        QJsonObject tempObject = value.toObject();
        double temperature = tempObject["temperature"].toDouble();
        QString timestamp = tempObject["timestamp"].toString();
        QDateTime dateTime = QDateTime::fromString(timestamp, "yyyy-MM-dd HH:mm:ss");

        qint64 timeDiff = (dateTime.toMSecsSinceEpoch() - firstDateTime.toMSecsSinceEpoch()) / 1000;
        series->append(timeDiff, temperature);

        maxTime = qMax(maxTime, timeDiff);
        minTemp = qMin(minTemp, temperature);
        maxTemp = qMax(maxTemp, temperature);
    }

    chart->addSeries(series);
    chart->setTitle("Температура за период");
    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Время (с)");
    axisX->setRange(0, maxTime);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Температура (°C)");
    axisY->setRange(minTemp - 5, maxTemp + 5);

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    series->attachAxis(axisX);
    series->attachAxis(axisY);

    scene->addItem(chart);

    ui->graphicsView->setScene(scene);
    ui->graphicsView->fitInView(scene->sceneRect(), Qt::IgnoreAspectRatio);
}

void MainWindow::fetchCurrentTemperature() {
    QUrl url("http://localhost:8080/temperature");
    QNetworkRequest request(url);
    networkManager->get(request);
}

void MainWindow::fetchAverageTemperature() {
    QUrl url("http://localhost:8080/stats");
    QNetworkRequest request(url);
    networkManager->get(request);
}

void MainWindow::fetchHistoryTemperature() {
    QString startDate = ui->startDatetimeEdit->text();
    QString endDate = ui->endDatetimeEdit->text();

    if (startDate.isEmpty() || endDate.isEmpty()) {
        qDebug() << "Both start and end dates must be provided!";
        return;
    }

    QUrl url("http://localhost:8080/history");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QByteArray body;
    body.append("start_datetime=" + startDate.toStdString() + "&");
    body.append("end_datetime=" + endDate.toStdString());

    networkManager->sendCustomRequest(request, "GET", body);
}


void MainWindow::handleNetworkReply(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

        if (reply->url().path() == "/temperature") {
            QJsonObject jsonObj = jsonDoc.object();
            currentTemperature = jsonObj.value("temperature").toDouble();
            ui->currentTemperatureLabel->setText(QString::number(currentTemperature, 'f', 2) + " °C");
        } else if (reply->url().path() == "/stats") {
            QJsonObject jsonObj = jsonDoc.object();
            averageTemperature = jsonObj.value("average_temperature").toDouble();
            ui->averageTemperatureLabel->setText(QString::number(averageTemperature, 'f', 2) + " °C");
        } else if (reply->url().path() == "/history") {
            QVector<QPointF> historicalData;
            QJsonArray historyArray = jsonDoc.array();
            setupChart(historyArray);
        }
    } else {
        qDebug() << "Network error:" << reply->errorString();
    }
    reply->deleteLater();
}
