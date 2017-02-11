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

#ifndef ATHENAEUM_PERSISTENCEMODEL_H
#define ATHENAEUM_PERSISTENCEMODEL_H

#include <QDir>
#include <QObject>

class QAbstractItemModel;

namespace Athenaeum
{

    class AbstractBibliography;

    // Abstract super class of persistence models
    class PersistenceModel : public QObject
    {
        Q_OBJECT

    public:
        PersistenceModel(QObject * parent = 0);
        ~PersistenceModel();

        virtual bool isLoadable() const;
        virtual bool isPurgeable() const;
        virtual bool isSaveable() const;

        virtual bool load(QAbstractItemModel * model) const;
        virtual bool purge() const;
        virtual bool save(QAbstractItemModel * model) const;

    }; // class PersistenceModel




    // Default action means there's no persistence at all
    typedef PersistenceModel NoPersistenceModel;




    // Local persistence
    class LocalPersistenceModelPrivate;
    class LocalPersistenceModel : public PersistenceModel
    {
        Q_OBJECT

    public:
        LocalPersistenceModel(const QDir & path, QObject * parent = 0);
        ~LocalPersistenceModel();

        virtual bool isLoadable() const;
        virtual bool isPurgeable() const;
        virtual bool isSaveable() const;

        virtual bool load(QAbstractItemModel * model) const;
        virtual bool purge() const;
        virtual bool save(QAbstractItemModel * model) const;

    private:
        LocalPersistenceModelPrivate * d;
    }; // class LocalPersistenceModelPrivate




    // Collection persistence
    class CollectionPersistenceModelPrivate;
    class CollectionPersistenceModel : public PersistenceModel
    {
        Q_OBJECT

    public:
        CollectionPersistenceModel(const QDir & path, QObject * parent = 0);
        ~CollectionPersistenceModel();

        virtual bool isLoadable() const;
        virtual bool isPurgeable() const;
        virtual bool isSaveable() const;

        virtual bool load(QAbstractItemModel * model) const;
        virtual bool purge() const;
        virtual bool save(QAbstractItemModel * model) const;

    private:
        CollectionPersistenceModelPrivate * d;
    }; // class CollectionPersistenceModelPrivate

} // namespace Athenaeum

#endif // ATHENAEUM_PERSISTENCEMODEL_H
