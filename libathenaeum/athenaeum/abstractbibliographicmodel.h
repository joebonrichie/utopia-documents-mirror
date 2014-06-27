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

#ifndef ATHENAEUM_ABSTRACTBIBLIOGRAPHICMODEL_H
#define ATHENAEUM_ABSTRACTBIBLIOGRAPHICMODEL_H

#include <athenaeum/abstractbibliographiccollection.h>

#include <QAbstractItemModel>
#include <QDateTime>
#include <QList>
#include <QMap>
#include <QMimeData>
#include <QString>
#include <QStringList>

namespace Athenaeum
{

    /////////////////////////////////////////////////////////////////////////////////////
    // AbstractBibliographicModel is the Qt model that represents a library of
    // bibliographic items

    class BibliographicItem;
    class AbstractBibliographicModel : public QAbstractItemModel, public AbstractBibliographicCollection
    {
        Q_OBJECT
        Q_PROPERTY(bool persistent READ isPersistent STORED false CONSTANT)
        Q_PROPERTY(bool readOnly READ isReadOnly STORED false)
        Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
        Q_INTERFACES(Athenaeum::AbstractBibliographicCollection)

    public:
        using AbstractBibliographicCollection::State;
        using AbstractBibliographicCollection::ItemFlags;

        // General constructor for a new library
        AbstractBibliographicModel(QObject * parent = 0) : QAbstractItemModel(parent) {}
        // Destructor
        virtual ~AbstractBibliographicModel() {}

        /////////////////////////////////////////////////////////////////////////////////
        // Standard Qt helpers

        virtual bool acceptsDrop(const QMimeData * mimeData) const;
        virtual QMimeData * mimeData(const QModelIndexList & indexes) const;
        virtual QStringList mimeTypes() const;
        virtual Qt::DropActions supportedDropActions() const { return isReadOnly() ? Qt::CopyAction : (Qt::CopyAction | Qt::MoveAction); }

    signals:
        void stateChanged(Athenaeum::AbstractBibliographicCollection::State state);
        void titleChanged(const QString & title);

    }; // class AbstractBibliographicModel

} // namespace Athenaeum

// typedefs for Qt's metatype system (to allow QVariant access)
typedef QList< QPair< QStringList, QStringList > > _AuthorList;
typedef QMap< QString, QString > _IdentifierMap;

Q_DECLARE_METATYPE(_AuthorList)
Q_DECLARE_METATYPE(_IdentifierMap)

#endif // ATHENAEUM_ABSTRACTBIBLIOGRAPHICMODEL_H
