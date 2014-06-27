/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2014 Lost Island Labs
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

#ifndef ATHENAEUM_PERSISTENTBIBLIOGRAPHICMODEL_P_H
#define ATHENAEUM_PERSISTENTBIBLIOGRAPHICMODEL_P_H

#include <athenaeum/abstractbibliographicmodel.h>

#include <QDir>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QRunnable>
#include <QString>
#include <QVector>
#include <QUrl>

namespace Athenaeum
{

    class UrlImporter : public QObject, public QRunnable
    {
        Q_OBJECT

    public:
        UrlImporter(const QUrl & url, QObject * parent = 0);
        ~UrlImporter();

        void run();

    signals:
        void finished(Athenaeum::BibliographicItem * item);

    private:
        QUrl url;
    }; // class UrlImporter




    class BibliographicItem;
    class PersistentBibliographicModel;
    class PersistentBibliographicModelPrivate : public QObject
    {
        Q_OBJECT

    public:
        PersistentBibliographicModelPrivate(PersistentBibliographicModel * m, const QDir & path);

        PersistentBibliographicModel * m;

        QString title;
        bool readOnly;
        QMutex mutex;
        QVector< BibliographicItem * > items;
        QDir path;
        AbstractBibliographicModel::State state;

        QMutex importMutex;
        QList< QUrl > importQueue;
        int importing;

        // Manage the on-disk database
        void dispatchImporter(const QUrl & url);
        void queueUrlForImport(const QUrl & url);
        bool imprint();
        bool isIdle();
        bool load(QString * errorMsg = 0);
        bool save(bool incremental = true, QString * errorMsg = 0);

    public slots:
        void onUrlImporterFinished(Athenaeum::BibliographicItem * item);

    }; // class AbstractBibliographicModelPrivate

} // namespace Athenaeum

#endif // ATHENAEUM_PERSISTENTBIBLIOGRAPHICMODEL_P_H
