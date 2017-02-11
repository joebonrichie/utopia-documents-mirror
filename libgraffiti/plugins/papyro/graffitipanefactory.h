/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2017 Lost Island Labs
 *           <info@utopiadocs.com>
 *   
 *   Utopia Documents is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU GENERAL PUBLIC LICENSE VERSION 3 as
 *   published by the Free Software Foundation.
 *   
 *   Utopia Documents is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 *   Public License for more details.
 *   
 *   In addition, as a special exception, the copyright holders give
 *   permission to link the code of portions of this program with the OpenSSL
 *   library under certain conditions as described in each individual source
 *   file, and distribute linked combinations including the two.
 *   
 *   You must obey the GNU General Public License in all respects for all of
 *   the code used other than OpenSSL. If you modify file(s) with this
 *   exception, you may extend this exception to your version of the file(s),
 *   but you are not obligated to do so. If you do not wish to do so, delete
 *   this exception statement from your version.
 *   
 *   You should have received a copy of the GNU General Public License
 *   along with Utopia Documents. If not, see <http://www.gnu.org/licenses/>
 *  
 *****************************************************************************/

#ifndef GRAFFITIPANEFACTORY_H
#define GRAFFITIPANEFACTORY_H

#include <graffiti/view.h>
#include <graffiti/tablewidget.h>
#include <graffiti/graphswidget.h>
#include <graffiti/graphicsFlipWidget.h>
#include <papyro/embeddedpanefactory.h>
#include <papyro/utils.h>
#include <utopia2/networkaccessmanager.h>
#include <string>

#include <QBuffer>
#include <QEventLoop>
#include <QFileDialog>
#include <QObject>
#include <QPainter>
#include <QPen>
#include <QPointer>
#include <QPushButton>
#include <QRunnable>
#include <QStandardItemModel>
#include <QStringList>
#include <QThreadPool>
#include <QTime>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>

#define SCALE_FACTOR 1.0

class GraffitiPane : public QWidget, public Utopia::NetworkAccessManagerMixin
{
    Q_OBJECT

public:
    GraffitiPane(QString dataUrl, QString format, QString tableId, QString linkTemplate, QWidget * parent = 0)
        : QWidget(parent), _dataUrl(dataUrl), _format(format), _tableId(tableId), _linkTemplate(linkTemplate), _progress(-1.0), _downloaded(false), _retries(3), _redirects(0)
    {
        _layout = new QVBoxLayout(this);
        _layout->setContentsMargins(0, 0, 0, 0);
        _layout->setSpacing(0);

        _checker.setInterval(1000);
        connect(&_checker, SIGNAL(timeout()), this, SLOT(check()));

        QPalette palette = this->palette();
        palette.setColor(QPalette::Active, QPalette::Window, Qt::white);
        this->setPalette(palette);

        // Widget stuff
        setMouseTracking(true);
        setAutoFillBackground(true);
        setContentsMargins(0, 0, 0, 0);


        qDebug()<<"Before restart";
        restart();
        qDebug()<<"After restart";



    }

    virtual ~GraffitiPane()
    {}

public Q_SLOTS:
    void flip()
    {
        _flipWidget->flip();
    }

    void exportCSV()
    {
        if (QStandardItemModel * model = _tableWidget->model()) {
            QString csvFileName = QFileDialog::getSaveFileName(this, "Choose where to save CSV...", QString(), "CSV Files (*.csv)");
            if (!csvFileName.isEmpty()) {
                QString csv;
                for (int r = 0; r < model->rowCount(); ++r) {
                    for (int c = 0; c < model->columnCount(); ++c) {
                        if (c != 0) { csv += ","; }
                        QModelIndex i = model->index(r, c);
                        QString cell = model->data(i).toString().replace('"', "\"\"");
                        csv += "\"" + cell + "\"";
                    }
                    csv += "\n";
                }

                // Save file
                if (!csv.isEmpty()) {
                    QFile csvFile(csvFileName);
                    if (csvFile.open(QIODevice::WriteOnly)) {
                        csvFile.write(csv.toUtf8());
                    } else {
                        // FIXME what if I can't open the file?
                    }
                } else {
                    // FIXME what if empty?
                }
            }
        }
    }

protected Q_SLOTS:

    void abort()
    {
        qDebug() << "abort";
        // Abort because of timeout
        _reply->abort();
        _checker.stop();
    }

    void check()
    {
        qDebug() << "check" << _lastUpdate.elapsed();
        if (_lastUpdate.elapsed() > 15000)
        {
            abort();
        }
    }

    void getCompleted()
    {
        qDebug() << "getCompleted";

        QGraphicsScene *scene = 0;
        QStandardItemModel *model = 0;

        if (_reply)
        {
            _reply->deleteLater();

            // If this is a redirect, follow it transparently, but only to a maximum of (arbitrarily)
            // four redirections
            QUrl redirectedUrl = _reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
            if (redirectedUrl.isValid()) {
                if (redirectedUrl.isRelative()) {
                    QString redirectedAuthority = redirectedUrl.authority();
                    redirectedUrl = _reply->url().resolved(redirectedUrl);
                    if (!redirectedAuthority.isEmpty()) {
                        redirectedUrl.setAuthority(redirectedAuthority);
                    }
                }
                if (_redirects++ < 4) {
                    QNetworkRequest request = _reply->request();
                    request.setUrl(redirectedUrl);
                    _reply = networkAccessManager()->get(request);
                    connect(_reply.data(), SIGNAL(finished()), this, SLOT(getCompleted()));
                    connect(_reply.data(), SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(getFailed(QNetworkReply::NetworkError)));
                    connect(_reply.data(), SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(getProgressed(qint64, qint64)));
                    return;
                }
            }

            // Reset redirect count and make sure those who care deal with this reply finishing
            _redirects = 0;
            _progress = 1.0;
            _checker.stop();

            _downloaded = true;

            QIODevice *result = _reply.data();
            QByteArray res = result->readAll();
            QString str(QString::fromUtf8(res.constData(), res.size()));

            scene = new QGraphicsScene();
            model = new QStandardItemModel();

            qDebug() << _format << str;
            if (_format == "nlm")
            {
//                Graffiti::TableWidget::initModelFromNLMXMLData(str, _tableId, model);
                Graffiti::TableWidget::initModelFromXML(model, str, _tableId);
            }
            else
            {
                Graffiti::TableWidget::initModelFromElsevierXMLData(str, _tableId, model);
            }

//                Graffiti::TableWidget::initModelFromNLMXML("/Users/srp/Desktop/NLMs/BJ427(1)_XML/bj4270143.xml", "T1", model);
//                Graffiti::TableWidget::initModelFromNLMXML(":nlmxml/bj4290261_T1.xml", model);
//                Graffiti::TableWidget::initModelFromNLMXML(":nlmxml/bj4110161.xml", model);
//                Graffiti::TableWidget::initModelFromCortiData(model);
        }
        else
        {
            Graffiti::TableWidget::initModelFromCortiData(model);
        }


        _tableWidget = new Graffiti::TableWidget(model);
        _graphWidget = new Graffiti::GraphsWidget(0,0,model);
        _graphWidget->setLinkTemplate(_linkTemplate);
        connect(_graphWidget, SIGNAL(resized()), this, SLOT(resizeContents()));

        // current implementation of GraphsWidget assumes the following things are set
        // doing anything else may result in undefined behaviour
        _tableWidget->setRowHeader(true);
        _tableWidget->setColumnHeader(true);
        _tableWidget->setDataSeriesOrientation(Graffiti::TableWidget::ColumnSeries);
        _tableWidget->setGraphType(Graffiti::TableWidget::ScatterPlot);

        QObject::connect(_tableWidget, SIGNAL(tableChanged(Graffiti::TableWidget::DataSeriesOrientation, Graffiti::TableWidget::GraphType, int)),
                         _graphWidget, SLOT(dataChanged(Graffiti::TableWidget::DataSeriesOrientation, Graffiti::TableWidget::GraphType, int)));

        _tableWidget->setColumnSeriesType(0, Graffiti::GraphTableView::Label);

        int column = 1;
        bool foundXAxis = false;
        while (column < _tableWidget->columns() && !foundXAxis)
        {
            if (_tableWidget->columnIsNumeric(column))
            {
                _tableWidget->setColumnSeriesType(column, Graffiti::GraphTableView::XAxis);
                qDebug() << "Setting col " << column << " to xaxis";
                foundXAxis = true;
            }
            ++column;

        }

        bool foundYAxis = false;
        while (column < _tableWidget->columns() && !foundYAxis)
        {
            if (_tableWidget->columnIsNumeric(column))
            {
                _tableWidget->setColumnSeriesType(column, Graffiti::GraphTableView::YAxis);
                qDebug() << "Setting col " << column << " to yaxis";
                foundYAxis = true;
            }
            ++column;
        }

        _graffitiView = new Graffiti::View();
        _graffitiView->setMouseTracking(true);
        _graffitiView->setFrameStyle(QFrame::NoFrame);
        _layout->addWidget(_graffitiView);

        _layout->invalidate();

        QRect newRect = _graffitiView->viewport()->rect();
        newRect.setWidth(newRect.width() / SCALE_FACTOR);
        newRect.setHeight(newRect.height() / SCALE_FACTOR);
        _tableWidget->setGeometry(newRect);
        _graphWidget->setGeometry(newRect);

        _flipWidget = new Graffiti::GraphicsFlipWidget(_tableWidget, _graphWidget);

        scene->addItem(_flipWidget);

        connect(_graffitiView, SIGNAL(resized()), this, SLOT(resizeContents()));

        _flipWidget->setScale(SCALE_FACTOR);

        if (this->parent() == 0) {
            QHBoxLayout * hLay = new QHBoxLayout;
            _layout->addLayout(hLay);
            hLay->addStretch(1);

            QPushButton *exportButton = new QPushButton("Export as CSV...");
            QObject::connect(exportButton, SIGNAL(clicked()), this, SLOT(exportCSV()));
            hLay->addWidget(exportButton);

            QPushButton *flipButton = new QPushButton("Toggle Table/Graph");
            QObject::connect(flipButton, SIGNAL(clicked()), _flipWidget, SLOT(flip()));
            hLay->addWidget(flipButton);
        }

        _graffitiView->setScene(scene);
        _graffitiView->scene()->setSceneRect(_graffitiView->viewport()->rect());

        update();
    }

    void getFailed(QNetworkReply::NetworkError code)
    {
        qDebug() << "get failed";

        switch (code)
        {
        case QNetworkReply::ContentNotFoundError:
            _errorMessage = "Requested data not available";
            break;
        case QNetworkReply::HostNotFoundError:
            _errorMessage = "Host not found (utopia.cs.manchester.ac.uk)";
            break;
        case QNetworkReply::ConnectionRefusedError:
            _errorMessage = "Connection refused (utopia.cs.manchester.ac.uk)";
            break;
        case QNetworkReply::RemoteHostClosedError:
            _errorMessage = "Unexpected disconnection (utopia.cs.manchester.ac.uk)";
            break;
        case QNetworkReply::ProtocolFailure:
            _errorMessage = "Malformed response (utopia.cs.manchester.ac.uk)";
            break;
        case QNetworkReply::ProxyAuthenticationRequiredError:
        case QNetworkReply::AuthenticationRequiredError:
            _errorMessage = "Authentication failed (utopia.cs.manchester.ac.uk)";
            break;
        case QNetworkReply::OperationCanceledError:
        case QNetworkReply::TimeoutError:
            _errorMessage = "Network timeout occurred";
            break;
        default:
            _errorMessage = "Unknown data download error";
            break;
        }

        if (isHidden() && --_retries > 0)
        {
            QTimer::singleShot(0, this, SLOT(restart()));
        }
    }

    void getProgressed(qint64 progress, qint64 total)
    {
        qDebug() << "getprogressed";

        if (total > 0)
        {
            _progress = qBound(0.0, progress / (double) total, 1.0);
        }
        _lastUpdate.restart();

        update();
    }

    void load()
    {
        qDebug() << "Got as a far as load";
    }

    void restart()
    {
        if (_dataUrl.isEmpty() && _tableId.isEmpty() && _linkTemplate.isEmpty())
        {
            getCompleted();
            return;
        }

        _errorMessage = QString();
        _progress = -1.0;
        _checker.start();
        _lastUpdate.start();
        _started.start();
        _downloaded = false;

        _reply = networkAccessManager()->get(QNetworkRequest(_dataUrl));
        connect(_reply.data(), SIGNAL(finished()), this, SLOT(getCompleted()));
        connect(_reply.data(), SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(getFailed(QNetworkReply::NetworkError)));
        connect(_reply.data(), SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(getProgressed(qint64, qint64)));

        qDebug() <<"finished restart!!";
        update();
    }






    void resizeContents()
    {
        QRect newRect = _graffitiView->viewport()->rect();
        newRect.setWidth(newRect.width() / SCALE_FACTOR);
        newRect.setHeight(newRect.height() / SCALE_FACTOR);
        _tableWidget->setGeometry(newRect);
        _graphWidget->setGeometry(newRect);
    }


private:
    QString _dataUrl;
    QString _format;
    QString _tableId;
    QString _linkTemplate;
    QVBoxLayout * _layout;
    Graffiti::View * _graffitiView;
    Graffiti::TableWidget * _tableWidget;
    Graffiti::GraphsWidget * _graphWidget;
    Graffiti::GraphicsFlipWidget * _flipWidget;

    QTimer _checker;
    QPointer< QNetworkReply > _reply;
    double _progress;
    QTime _lastUpdate;
    QTime _started;
    bool _downloaded;
    int _retries;
    int _redirects;
    QString _errorMessage;
    QString _code;


};

class GraffitiPaneFactory : public Papyro::EmbeddedPaneFactory
{

public:
    // Constructor
    GraffitiPaneFactory()
        : Papyro::EmbeddedPaneFactory()
        {}

    // Destructor
    virtual ~GraffitiPaneFactory()
        {}

    virtual QWidget * create(Spine::AnnotationHandle annotation, QWidget * parent = 0)
        {
            GraffitiPane * pane = 0;
            std::string concept = annotation->getFirstProperty("concept");
            std::string dataUrl = annotation->getFirstProperty("property:dataUrl");
            std::string tableId = annotation->getFirstProperty("property:tableId");
            std::string linkTemplate = annotation->getFirstProperty("property:linkTemplate");
            std::vector< std::string > media = annotation->getProperty("session:media");
            std::vector< std::string > upload_files = annotation->getProperty("session:upload_files");
            if (concept == "http://utopia.cs.man.ac.uk/utopia/annotation#graph" ||
                concept == "ElsevierTable" ||
                concept == "Table")
            {
                QString format = (concept == "ElsevierTable") ? "elsevier" : "nlm";

                QString qDataUrl = dataUrl.empty() ? QString() : Papyro::qStringFromUnicode(dataUrl);
                QString qTableId = tableId.empty() ? QString() : Papyro::qStringFromUnicode(tableId);
                QString qLinkTemplate = linkTemplate.empty() ? QString() : Papyro::qStringFromUnicode(linkTemplate);
                if (!qTableId.isEmpty() && !qDataUrl.isEmpty()) {
                    pane = new GraffitiPane(qDataUrl, format, qTableId, qLinkTemplate, parent);
                } else {
                    QString mime_type, url, name;
                    if (!media.empty()) {
                        foreach (std::string media_link, media) {
                            QString encoded(Papyro::qStringFromUnicode(media_link));
                            QStringList pairs(encoded.split("&"));
                            foreach (QString pair, pairs) {
                                QString key(QUrl::fromPercentEncoding(pair.section('=', 0, 0).toUtf8()));
                                QString value(QUrl::fromPercentEncoding(pair.section('=', 1, 1).toUtf8()));
                                if (key == "type") mime_type = value;
                                else if (key == "src") url = value;
                                else if (key == "name") name = value;
                            }
                        }
                    } else {
                        foreach(std::string upload_file, upload_files) {
                            QString dataUrl(Papyro::qStringFromUnicode(upload_file));
                            if (!dataUrl.startsWith("data:")) break;
                            mime_type = dataUrl.section(';', 0, 0).mid(5);
                            name = dataUrl.section(';', 1, 1).mid(5);
                            url = dataUrl;
                            break;
                        }
                    }
                    if (!url.isEmpty() && name == "data.xml") {
                        pane = new GraffitiPane(url, format, QString(), qLinkTemplate, parent);
                    }
                }
            }
            return pane;
        }

    virtual QString title()
    {
        return "Graph Plot";
    }

}; // class GraffitiPaneFactory

#endif // GRAFFITIPANEFACTORY_H
