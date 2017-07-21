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

#ifndef PAPYRO_RESULTSVIEW_P_H
#define PAPYRO_RESULTSVIEW_P_H

#include <papyro/resolver.h>
#include <papyro/resultitem.h>
#include <papyro/cslengine.h>
#include <utopia2/busagent.h>

#include <QEventLoop>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QTimer>
#include <QWebElement>

namespace Papyro
{

    class ResultsViewPrivate;
    class DownloadCapability;




    class ResultItemControl : public QObject, public Utopia::BusAgent
    {
        Q_OBJECT
        Q_PROPERTY(bool openByDefault READ openByDefault)
        Q_PROPERTY(int weight READ weight)
        Q_PROPERTY(QString authorUri READ authorUri)
        Q_PROPERTY(QString title READ title)
        Q_PROPERTY(QString description READ description)
        Q_PROPERTY(QString sourceDatabase READ sourceDatabase)
        Q_PROPERTY(QString sourceIcon READ sourceIcon)
        Q_PROPERTY(QWebElement element
                   READ element
                   WRITE setElement)
        Q_PROPERTY(QString cssId READ cssId)
        Q_PROPERTY(bool headless READ headless)
        Q_PROPERTY(QString highlight READ highlight)
        Q_PROPERTY(QVariantMap context READ context)

    public:
        ResultItemControl(ResultsViewPrivate * viewPrivate, ResultItem * item);

        QString authorUri() const;
        QVariantMap context() const;
        QString cssId() const;
        QString description() const;
        QWebElement element() const;
        bool headless() const;
        QString highlight() const;
        ResultItem * item() const;
        bool openByDefault() const;
        void setElement(QWebElement element);
        QString sourceDatabase() const;
        QString sourceIcon() const;
        QWebElement thumbnailElement(const QString & thumbnailClass) const;
        QString title() const;
        Q_INVOKABLE QString value(const QString & key) const;
        Q_INVOKABLE QStringList values(const QString & key) const;
        int weight() const;

    signals:
        void insertContent(QWebElement element, const QVariant & content);

    public slots:
        void toggleContent();
        void postMessage(const QVariant & data);
        void postMessage(const QString & recipient, const QVariant & data);

    protected slots:
        void contentChanged(const QString & key);
        void stateChanged(Papyro::ResultItem::State state);

    public: // Members
        struct
        {
            ResultsViewPrivate * viewPrivate;

            ResultItem * item;
            struct {
                QList< boost::shared_ptr< DownloadCapability > > downloads;
            } capabilities;
            QWebElement resultElement;

            QStringList contentKeys;
        } d;
    };




    class MetadataResolutionFuture : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(bool ready READ isReady);
        Q_PROPERTY(QVariantMap results READ results);

    public:
        MetadataResolutionFuture(const QVariantMap & metadata, const QString & purpose);

        bool isReady() const;
        QVariantMap results() const;

        Q_SCRIPTABLE void doom();
        Q_SCRIPTABLE void lock();
        Q_SCRIPTABLE void unlock();

    signals:
        void completed(const QVariantMap & metadata);

    protected:
        bool _ready;
        QMutex _mutex;
        QVariantMap _metadata;

    protected slots:
        void onResolverRunnableCompleted(Athenaeum::CitationHandle citation);
    };




    class ResultsViewControl : public QObject, public Utopia::BusAgent
    {
        Q_OBJECT

    public:
        ResultsViewControl(ResultsViewPrivate * resultsViewPrivate);

        Q_SCRIPTABLE QVariantMap availableCitationStyles();
        Q_SCRIPTABLE QString defaultCitationStyle();
        Q_SCRIPTABLE QString formatCitation(const QVariantMap & metadata, const QString & style = QString());
        Q_SCRIPTABLE QObject * resolveMetadata(const QVariantMap & metadata, const QString & purpose);

    public slots:
        void activateCitation(const QVariantMap & citation, const QString & target);
        void activateCitations(const QVariantList & citation, const QString & target);
        void activateLink(const QString & href, const QString & target);
        void activateSource(QObject * obj);
        void explore(const QString & term);
        void onLoadComplete();
        void searchRemote(const QString & term);

    signals:
        void citationsActivated(const QVariantList & citation, const QString & target);
        void linkClicked(const QUrl & href, const QString & target);
        void resultAdded(QObject * result);
        void termExplored(const QString & term);

    protected:
        ResultsViewPrivate * d;
    };




    class ResultsView;
    class ResultsViewPrivate : public QObject
    {
        Q_OBJECT

    public:
        ResultsViewPrivate(ResultsView * sidebar);

        ResultsView * view;
        ResultsViewControl * control;
        QList< ResultItemControl * > results;
        QStringList classes;

        QQueue< ResultItem * > resultQueue;
        QTimer resultQueueTimer;

        boost::shared_ptr< CSLEngine > cslengine;

        bool ready;
        QEventLoop wait;

        QStringList terms;
        bool exploreTerms;

    public slots:
        void setupJavaScriptWindowObject();
        void addResult();

    signals:
        void linkClicked(const QUrl & href, const QString & target);
        void runningChanged(bool running);
        void resultAdded(QObject * result);
    };

}

#endif // PAPYRO_RESULTSVIEW_P_H
