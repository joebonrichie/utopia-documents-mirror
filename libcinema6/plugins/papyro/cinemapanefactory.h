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

#ifndef CINEMAPANEFACTORY_H
#define CINEMAPANEFACTORY_H

#include <papyro/embeddedpanefactory.h>
#include <papyro/embeddedpane.h>
#include <papyro/utils.h>
#include <cinema6/alignmentview.h>
#include <cinema6/annotationcomponent.h>
#include <cinema6/controlaspect.h>
#include <cinema6/groupaspect.h>
#include <cinema6/keycomponent.h>
#include <cinema6/sequence.h>
#include <cinema6/sequencecomponent.h>
#include <cinema6/titleaspect.h>
#include <utopia2/networkaccessmanager.h>
#include <utopia2/fileformat.h>
#include <utopia2/parser.h>
#include <utopia2/qt/webview.h>
#include <string>

#include <cmath>

#include <QBuffer>
#include <QNetworkReply>
#include <QObject>
#include <QPainter>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>
#include <QWebFrame>

class CinemaPane : public Papyro::EmbeddedPane
{
    Q_OBJECT

public:
    CinemaPane(QByteArray bytes, Utopia::FileFormat * format, QWidget * parent = 0)
        : Papyro::EmbeddedPane(Papyro::EmbeddedPane::DefaultFlags, parent), _format(format), _cinemaWidget(0), _model(0)
    {
        init();
        QVariantMap map;
        map["bytes"] = bytes;
        setData(map);
    }

    CinemaPane(QUrl url, Utopia::FileFormat * format, QWidget * parent = 0)
        : Papyro::EmbeddedPane(Papyro::EmbeddedPane::DefaultFlags, parent), _format(format), _cinemaWidget(0), _model(0)
    {
        init();
        QVariantMap map;
        map["url"] = url;
        setData(map);
    }

    void init()
    {
        _layout = new QVBoxLayout(this);
        _layout->setContentsMargins(0, 0, 0, 0);
        _layout->setSpacing(0);

        // Widget stuff
        setMouseTracking(true);
        resize(600, 150);
    }

    virtual ~CinemaPane()
    {
    }

protected:
    void download()
    {
        QVariantMap conf(data().toMap());
        // Don't bother with the download step if we already have the data
        if (!conf.contains("bytes") && conf.contains("url")) {
            // Otherwise construct a URL...
            QUrl url = conf.value("url").toUrl();

            // ... and download it
            if (url.isValid()) {
                startDownload(url);
            }
        } else {
            skipDownload();
        }
    }

    void load()
    {
        if (_model == 0 && _cinemaWidget == 0)
        {
            QByteArray bytes(data().toMap().value("bytes").toByteArray());
            QBuffer buffer(&bytes);
            buffer.open(QIODevice::ReadOnly);
            Utopia::Parser::Context ctx = Utopia::parse(buffer, _format);
            if (ctx.errorCode() == Utopia::Parser::None)
            {
                _model = ctx.model();
            }

            // load model
            if (_model && _cinemaWidget == 0)
            {
                int authCount = _model->relations(Utopia::UtopiaSystem.hasPart).size();
                Utopia::Node::relation::iterator seq = _model->relations(Utopia::UtopiaSystem.hasPart).begin();
                Utopia::Node::relation::iterator end = _model->relations(Utopia::UtopiaSystem.hasPart).end();
                if (seq != end)
                {
                    // Widget
                    static Utopia::Node* p_title = Utopia::UtopiaDomain.term("title");
                    _cinemaWidget = new CINEMA6::AlignmentView;
                    //_cinemaWidget->setInteractionMode(CINEMA6::AlignmentView::GapMode);

                    _cinemaWidget->appendComponent(CINEMA6::AlignmentView::Top, new CINEMA6::KeyComponent());
                    _cinemaWidget->appendComponent(CINEMA6::AlignmentView::Bottom, new CINEMA6::KeyComponent());
                    for (; seq != end; ++seq)
                    {
                        Utopia::Node::relation::iterator seq2 = authCount == 1 ? seq : (*seq)->relations(Utopia::UtopiaSystem.hasPart).begin();
                        Utopia::Node::relation::iterator end2 = authCount == 1 ? end : (*seq)->relations(Utopia::UtopiaSystem.hasPart).end();
                        if (seq2 != end2)
                        {
                            _cinemaWidget->appendComponent(CINEMA6::AlignmentView::Center, new CINEMA6::SequenceComponent(new CINEMA6::Sequence(*seq2)));
                            //qDebug() << "++++" << (*seq)->attributes.get(p_title).toString().mid(3);
                        }
                    }

                    _cinemaWidget->appendAspect(CINEMA6::AlignmentView::Left, new CINEMA6::TitleAspect("Names"));
                    _cinemaWidget->appendAspect(CINEMA6::AlignmentView::Right, new CINEMA6::ControlAspect("Control"));

                    _cinemaWidget->setInteractionMode(CINEMA6::AlignmentView::GapMode);
                    _cinemaWidget->show();
                    _layout->addWidget(_cinemaWidget);
                }
                else
                {
                    qDebug() << "No sequence found in model!";
                }
            }
        }
    }

    QVariant parseDownload(QNetworkReply * reply)
    {
        QVariantMap map(data().toMap());
        map["bytes"] = reply->readAll();
        return map;
    }

private:
    QVBoxLayout * _layout;

    Utopia::FileFormat * _format;
    CINEMA6::AlignmentView * _cinemaWidget;
    Utopia::Node * _model;
};



class CinemaPaneFactory : public Papyro::EmbeddedPaneFactory
{

public:
    QSet< Utopia::FileFormat * > formats;

    // Constructor
    CinemaPaneFactory()
    : Papyro::EmbeddedPaneFactory()
    {
        formats = Utopia::FileFormat::get(Utopia::SequenceFormat);
    }

    // Destructor
    virtual ~CinemaPaneFactory()
    {}

    virtual QWidget * create(Spine::AnnotationHandle annotation, QWidget * parent = 0)
    {
        CinemaPane * pane = 0;
        std::string concept = annotation->getFirstProperty("concept");
        std::string dataUrl = annotation->getFirstProperty("property:dataUrl");
        QString url, name;
        std::vector< std::string > media = annotation->getProperty("session:media");

        if (!dataUrl.empty() && concept == "Alignment")
        {
            url = Papyro::qStringFromUnicode(dataUrl);
            name = QUrl(url).path();
        }
        else if (concept == "DataLink" && media.size() > 0)
        {
            if (url.isEmpty())
            {
                foreach(std::string media_link, media)
                {
                    QString encoded(Papyro::qStringFromUnicode(media_link));
                    QStringList pairs(encoded.split("&"));
                    foreach(QString pair, pairs)
                    {
                        QString key(QUrl::fromPercentEncoding(pair.section('=', 0, 0).toUtf8()));
                        QString value(QUrl::fromPercentEncoding(pair.section('=', 1, 1).toUtf8()));
                        if (key == "src") url = value;
                        else if (key == "name") name = value;
                    }
                }
            }
        }

        if (!url.isEmpty() && !name.isEmpty())
        {
            foreach (Utopia::FileFormat * format, formats)
            {
                if (format->contains(name.section(".", -1)))
                {
                    pane = new CinemaPane(url, format, parent);
                    break;
                }
            }
        }
        return pane;
    }

    virtual QString title()
    {
        return "Sequence Alignment";
    }

}; // class CinemaPaneFactory

#endif // CINEMAPANEFACTORY_H


















/*

#ifndef CINEMAPANEFACTORY_H
#define CINEMAPANEFACTORY_H

#include <cinema6/alignmentview.h>
#include <cinema6/annotationcomponent.h>
#include <cinema6/controlaspect.h>
#include <cinema6/groupaspect.h>
#include <cinema6/keycomponent.h>
#include <cinema6/sequence.h>
#include <cinema6/sequencecomponent.h>
#include <cinema6/titleaspect.h>
#if !defined(Q_MOC_RUN) || QT_VERSION >= 0x050000
#  include <spine/Annotation.h>
#endif
#include <papyro/embeddedpanefactory.h>
#include <papyro/utils.h>
#include <utopia2/networkaccessmanager.h>
#include <utopia2/fileformat.h>
#include <utopia2/parser.h>
#include <math.h>

#include <QBuffer>
#include <QEventLoop>
#include <QFileInfo>
#include <QMouseEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QPainter>
#include <QPointer>
#include <QPen>
#include <QRunnable>
#include <QThreadPool>
#include <QTime>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

class CinemaPane : public QWidget, public Utopia::NetworkAccessManagerMixin
{
    Q_OBJECT

    public:
        CinemaPane(QString url, Utopia::FileFormat * format, QWidget * parent = 0)
        : QWidget(parent), _format(format), _url(url), _cinemaWidget(0), _model(0), _progress(-1.0), _retryHover(false), _retryPressed(false), _downloaded(false), _retries(3), _redirects(0)
        {
            _layout = new QVBoxLayout(this);
            _layout->setContentsMargins(0, 0, 0, 0);
            _layout->setSpacing(0);

            // QNetworkAccessManager stuff for getting update information
            _checker.setInterval(1000);
            connect(&_checker, SIGNAL(timeout()), this, SLOT(check()));

            // Widget stuff
            setMouseTracking(true);
            resize(600, 150);

            // Start download
            restart();
        }

        virtual ~CinemaPane()
        {
            if (_model)
            {
                delete _model;
            }
        }

    protected slots:
        void abort()
        {
            // Abort because of timeout
            _reply->abort();
            _checker.stop();
        }

        void check()
        {
            if (_lastUpdate.elapsed() > 15000)
            {
                abort();
            }
        }

        void getCompleted()
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

            _replyData = _reply->readAll();

            if (isVisible())
            {
                load();
            }

            update();
        }

        void getFailed(QNetworkReply::NetworkError code)
        {
            switch (code)
            {
            case QNetworkReply::ContentNotFoundError:
                _errorMessage = "Requested data not available";
                break;
            case QNetworkReply::HostNotFoundError:
                _errorMessage = "Host not found";
                break;
            case QNetworkReply::ConnectionRefusedError:
                _errorMessage = "Connection refused";
                break;
            case QNetworkReply::RemoteHostClosedError:
                _errorMessage = "Unexpected disconnection";
                break;
            case QNetworkReply::ProtocolFailure:
                _errorMessage = "Malformed response";
                break;
            case QNetworkReply::ProxyAuthenticationRequiredError:
            case QNetworkReply::AuthenticationRequiredError:
                _errorMessage = "Authentication failed";
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
            if (total > 0)
            {
                _progress = qBound(0.0, progress / (double) total, 1.0);
            }
            _lastUpdate.restart();

            update();
        }

        void load()
        {
            if (_model == 0 && _cinemaWidget == 0)
            {
                QBuffer buffer(&_replyData);
                buffer.open(QIODevice::ReadOnly);
                Utopia::Parser::Context ctx = Utopia::parse(buffer, _format);
                if (ctx.errorCode() == Utopia::Parser::None)
                {
                    _model = ctx.model();
                }

                // load model
                if (_model && _cinemaWidget == 0)
                {
                    int authCount = _model->relations(Utopia::UtopiaSystem.hasPart).size();
                    Utopia::Node::relation::iterator seq = _model->relations(Utopia::UtopiaSystem.hasPart).begin();
                    Utopia::Node::relation::iterator end = _model->relations(Utopia::UtopiaSystem.hasPart).end();
                    if (seq != end)
                    {
                        // Widget
                        static Utopia::Node* p_title = Utopia::UtopiaDomain.term("title");
                        _cinemaWidget = new CINEMA6::AlignmentView;
                        //_cinemaWidget->setInteractionMode(CINEMA6::AlignmentView::GapMode);

                        _cinemaWidget->appendComponent(CINEMA6::AlignmentView::Top, new CINEMA6::KeyComponent());
                        _cinemaWidget->appendComponent(CINEMA6::AlignmentView::Bottom, new CINEMA6::KeyComponent());
                        for (; seq != end; ++seq)
                        {
                            Utopia::Node::relation::iterator seq2 = authCount == 1 ? seq : (*seq)->relations(Utopia::UtopiaSystem.hasPart).begin();
                            Utopia::Node::relation::iterator end2 = authCount == 1 ? end : (*seq)->relations(Utopia::UtopiaSystem.hasPart).end();
                            if (seq2 != end2)
                            {
                                _cinemaWidget->appendComponent(CINEMA6::AlignmentView::Center, new CINEMA6::SequenceComponent(new CINEMA6::Sequence(*seq2)));
                                qDebug() << "++++" << (*seq)->attributes.get(p_title).toString().mid(3);
                            }
                        }

                        _cinemaWidget->appendAspect(CINEMA6::AlignmentView::Left, new CINEMA6::TitleAspect("Names"));
                        _cinemaWidget->appendAspect(CINEMA6::AlignmentView::Right, new CINEMA6::ControlAspect("Control"));

                        _cinemaWidget->setInteractionMode(CINEMA6::AlignmentView::GapMode);
                        _cinemaWidget->show();
                        _layout->addWidget(_cinemaWidget);
                    }
                    else
                    {
                        qDebug() << "No sequence found in model!";
                    }
                }
                else if (_errorMessage.isEmpty())
                {
                    _errorMessage = "Cannot load data";
                }
            }

            update();
        }

        void restart()
        {
            _errorMessage = QString();
            _progress = -1.0;
            _checker.start();
            _lastUpdate.start();
            _started.start();
            _downloaded = false;

            _reply = networkAccessManager()->get(QNetworkRequest(_url));
            connect(_reply.data(), SIGNAL(finished()), this, SLOT(getCompleted()));
            connect(_reply.data(), SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(getFailed(QNetworkReply::NetworkError)));
            connect(_reply.data(), SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(getProgressed(qint64, qint64)));

            update();
        }

    protected:
        QRect retryButtonGeometry()
        {
            int radius = 20;
            QRect spinnerRect(0, 0, 2*radius, 2*radius);
            spinnerRect.moveCenter(rect().center() + QPoint(0, -11) + QPoint(-1, -1));
            QRect messageRect(0, spinnerRect.bottom() + 10, width(), 12);
            QString text("Retry");
            int textWidth = fontMetrics().width(text);
            int textHeight = fontMetrics().height();
            QRect retryRect(0, 0, 12 + 6 + textWidth, qMin(12, textHeight));
            retryRect.moveCenter(rect().center());
            retryRect.moveTop(messageRect.bottom() + 20);
            return retryRect;
        }

        void mouseMoveEvent(QMouseEvent * event)
        {
            bool old = _retryHover;
            _retryHover = retryButtonGeometry().contains(event->pos());
            if (old != _retryHover)
            {
                update();
            }
        }

        void mousePressEvent(QMouseEvent * event)
        {
            bool old = _retryPressed;
            _retryPressed = event->buttons(); // & Qt::LeftButton && retryButtonGeometry().contains(event->pos());
            if (old != _retryPressed)
            {
                update();
            }
        }

        void mouseReleaseEvent(QMouseEvent * event)
        {
            bool old = _retryPressed;
            _retryPressed = false;
            if (old) // && retryButtonGeometry().contains(event->pos()))
            {
                restart();
            }
            else if (old != _retryPressed)
            {
                update();
            }
        }

        void paintEvent(QPaintEvent * event)
        {
            if (_cinemaWidget != 0) return;

            QString message;
            int radius = 20;

            QPainter p(this);
            p.setRenderHint(QPainter::Antialiasing);
            p.setRenderHint(QPainter::TextAntialiasing);

            // Background
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(230, 230, 230));
            p.drawRect(rect());
            p.setPen(QColor(140, 140, 140));

            // Spinner
            QRect spinnerRect(0, 0, 2*radius, 2*radius);
            spinnerRect.moveCenter(rect().center() + QPoint(0, -11) + QPoint(-1, -1));
            if (!_errorMessage.isEmpty())
            {
                message = _errorMessage;

                QPen pen(p.pen());
                pen.setColor(QColor(180, 140, 140));
                pen.setWidth(4.0);
                p.setBrush(Qt::NoBrush);
                p.setPen(pen);
                p.drawEllipse(spinnerRect);
                QRect lineRect(0, 0, 2*radius/sqrt(2), 2*radius/sqrt(2));
                lineRect.moveCenter(rect().center() + QPoint(0, -11));
                p.drawLine(lineRect.bottomLeft(), lineRect.topRight());

                // Retry!
                QRect retryRect = retryButtonGeometry();
                if (_retryHover || _retryPressed)
                {
                    pen = p.pen();
                    pen.setWidth(1.0);
                    if (_retryPressed)
                    {
                        p.setBrush(QColor(230, 200, 200));
                    }
                    else
                    {
                        p.setBrush(Qt::NoBrush);
                    }
                    p.setPen(pen);
                    p.drawRect(retryRect.adjusted(-3, -3, 3, 3));
                }
                p.drawText(retryRect.adjusted(12 + 6, 0, 0, 0), Qt::AlignVCenter, "Retry");
                pen = p.pen();
                pen.setWidth(2.0);
                p.setBrush(Qt::NoBrush);
                p.setPen(pen);
                QRect iconRect(retryRect.topLeft(), QSize(12, 12));
                p.drawArc(iconRect, 16*90.0, -16*135.0);
                p.drawLine((iconRect.left() + iconRect.right()) / 2, iconRect.top(), 2 + (iconRect.left() + iconRect.right()) / 2, 2 + iconRect.top());
                p.drawArc(iconRect, 16*270.0, -16*135.0);
                p.drawLine((iconRect.left() + iconRect.right()) / 2, iconRect.bottom(), -1 + (iconRect.left() + iconRect.right()) / 2, -1 + iconRect.bottom());
            }
            else if (_progress >= 0 && _progress < 1.0)
            {
                message = "Downloading data...";
                QPen pen(p.pen());
                pen.setWidth(1.0);
                p.setPen(pen);
                p.setBrush(QColor(140, 140, 140));
                p.drawPie(spinnerRect, 16*90.0, -16*360.0*_progress);
                p.setBrush(Qt::NoBrush);
                p.drawEllipse(spinnerRect);
            }
            else if (_progress == 1.0 || _progress == -1.0)
            {
                message = _progress == 1.0 ? "Parsing data..." : "Downloading data...";
                int startAngle = _started.elapsed() * 5;
                int spanAngle = 120*16*2;
                QPen pen(p.pen());
                pen.setWidth(4.0);
                p.setBrush(Qt::NoBrush);
                p.setPen(pen);
                p.drawArc(spinnerRect.adjusted(2, 2, -2, -2), -startAngle, spanAngle);
                QTimer::singleShot(40, this, SLOT(update()));
            }
            else
            {
                message = "Initialising visualisation...";
            }

            QRect messageRect(0, spinnerRect.bottom() + 10, width(), 12);
            p.drawText(messageRect, Qt::AlignCenter, message);
        }

        void showEvent(QShowEvent * event)
        {
            if (_downloaded && _cinemaWidget == 0)
            {
                QTimer::singleShot(0, this, SLOT(load()));
            }
        }

    private:
        QString _url;
        QString _errorMessage;
        QVBoxLayout * _layout;
        Utopia::FileFormat * _format;
        CINEMA6::AlignmentView * _cinemaWidget;
        Utopia::Node * _model;

        QTimer _checker;
        QPointer< QNetworkReply > _reply;
        QByteArray _replyData;
        double _progress;
        QTime _lastUpdate;
        QTime _started;
        bool _retryHover;
        bool _retryPressed;
        bool _downloaded;
        int _retries;
        int _redirects;
};

class CinemaPaneFactory : public Papyro::EmbeddedPaneFactory
{

    public:
        QSet< Utopia::FileFormat * > formats;


        // Constructor
        CinemaPaneFactory()
        : Papyro::EmbeddedPaneFactory()
        {
            formats = Utopia::FileFormat::get(Utopia::SequenceFormat);
        }

        // Destructor
        virtual ~CinemaPaneFactory()
        {}

        virtual QWidget * create(Spine::AnnotationHandle annotation, QWidget * parent = 0)
        {
            CinemaPane * pane = 0;
            std::string concept = annotation->getFirstProperty("concept");
            std::string dataUrl = annotation->getFirstProperty("property:dataUrl");
            QString url, name;
            std::vector< std::string > media = annotation->getProperty("session:media");

            if (!dataUrl.empty() && concept == "Alignment")
            {
                url = Papyro::qStringFromUnicode(dataUrl);
                name = QUrl(url).path();
            }
            else if (concept == "DataLink" && media.size() > 0)
            {
                if (url.isEmpty())
                {
                    foreach(std::string media_link, media)
                    {
                        QString encoded(Papyro::qStringFromUnicode(media_link));
                        QStringList pairs(encoded.split("&"));
                        foreach(QString pair, pairs)
                        {
                            QString key(QUrl::fromPercentEncoding(pair.section('=', 0, 0).toUtf8()));
                            QString value(QUrl::fromPercentEncoding(pair.section('=', 1, 1).toUtf8()));
                            if (key == "src") url = value;
                            else if (key == "name") name = value;
                        }
                    }
                }
            }

            if (!url.isEmpty() && !name.isEmpty())
            {
                foreach (Utopia::FileFormat * format, formats)
                {
                    if (format->contains(name.section(".", -1)) && !url.isEmpty())
                    {
                        pane = new CinemaPane(url, format, parent);
                        break;
                    }
                }
            }
            return pane;
        }

        virtual QString title()
        {
            return "Sequence Alignment";
        }

}; // class CinemaPaneFactory

#endif // CINEMAPANEFACTORY_H

*/