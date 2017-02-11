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

#ifndef Utopia_CACHEDITEM_H
#define Utopia_CACHEDITEM_H

#include <utopia2/qt/config.h>

#include <boost/shared_ptr.hpp>

#include <QDataStream>
#include <QDateTime>
#include <QString>

namespace Utopia
{

    template< class Item >
    class CachedItemPrivate
    {
    public:
        CachedItemPrivate()
            : item(0)
        {}
        CachedItemPrivate(const Item & item, const QString & id, const QDateTime & accessed, const QDateTime & modified)
            : id(id), item(new Item(item)), accessed(accessed), modified(modified)
        {}
        ~CachedItemPrivate()
        {
            if (item) {
                delete item;
            }
        }

        QString id;
        Item * item;
        QDateTime accessed;
        QDateTime modified;
    };

    template< class Item >
    class CachedItem
    {
    public:
        typedef CachedItem< Item > CachedItemClass;
        typedef CachedItemPrivate< Item > CachedItemPrivateClass;

        // Constructors
        CachedItem()
            : d(new CachedItemPrivateClass)
        {}

        CachedItem(const Item & item, const QString & id, const QDateTime & accessed, const QDateTime & modified)
            : d(new CachedItemPrivateClass(item, id, accessed, modified))
        {}

        CachedItem(const CachedItemClass & rhs)
        {
            d = rhs.d;
        }

        // Destructor
        ~CachedItem()
        {}

        QDateTime accessed() const
        {
            return d->accessed;
        }

        QString id() const
        {
            return d->id;
        }

        bool isValid() const
        {
            return d->item;
        }

        const Item & item() const
        {
            return *d->item;
        }

        QDateTime modified() const
        {
            return d->modified;
        }

        void touch()
        {
            d->accessed = QDateTime::currentDateTime();
        }

        operator Item ()
        {
            return item();
        }

    private:
        // Item
        boost::shared_ptr< CachedItemPrivateClass > d;
    };

    template< class Item >
    LIBUTOPIA_QT_EXPORT QDataStream & operator << (QDataStream & str, const CachedItem< Item > & cachedItem)
    {
        str << cachedItem.id() << cachedItem.accessed() << cachedItem.modified() << cachedItem.item();
        return str;
    }

    template< class Item >
    LIBUTOPIA_QT_EXPORT QDataStream & operator >> (QDataStream & str, CachedItem< Item > & cachedItem)
    {
        QString id;
        QDateTime accessed;
        QDateTime modified;
        Item item;
        str >> id >> accessed >> modified >> item;
        cachedItem = CachedItem< Item >(item, id, accessed, modified);
        return str;
    }

} // namespace Utopia

#endif // Utopia_CACHEDITEM_H
