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

#ifndef Utopia_CACHE_H
#define Utopia_CACHE_H

#include <utopia2/qt/cacheditem.h>

#include <QDataStream>
#include <QDateTime>
#include <QDir>
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QPair>
#include <QString>

#include <QtDebug>

namespace Utopia
{

    template< class Item >
    class CachePrivate
    {
    public:
        typedef CachePrivate< Item > CachePrivateClass;
        typedef CachedItem< Item > CachedItemClass;

        // Create a new cache backend with the specified path name
        CachePrivate(const QString & path = QString())
            : path(path), maximumSize(0), mutex(QMutex::Recursive)
        {}
        ~CachePrivate()
        {}

        // Path name of cache
        QString path;
        // Items stored in cache
        QMap< QString, QPair< CachedItemClass, bool > > items;
        // Maximum size
        int maximumSize;
        // Recently used items
        QList< QString > recentlyUsed;

        // Mutex for concurrent access to this cache backend
        QMutex mutex;

        // Check the size of this cache, removing elements if they cause it
        // to exceed its maximum size
        void resize()
        {
            // A zero maximum size means this cache is unrestricted
            if (maximumSize > 0 && recentlyUsed.size() > maximumSize) {
                // Remove excess items from the front of the list, removing
                // them from the cache
                while (recentlyUsed.size() > maximumSize) {
                    QString doomed = recentlyUsed.takeFirst();
                    items.remove(doomed);
                }
            }
        }

        // Get an existing cache by path name or create a new one
        static boost::shared_ptr< CachePrivateClass > getCache(const QString & path)
        {
            // Global map of caches of this type of Item
            static QMap< QString, boost::weak_ptr< CachePrivateClass > > caches;
            // Global mutex for concurrent access to static cache map
            static QMutex globalMutex(QMutex::Recursive);

            // Protect access to global cache map
            QMutexLocker l(&globalMutex);

            // Empty path name creates a new unnamed cache (do not store anywhere!)
            if (path.isEmpty()) {
                return boost::shared_ptr< CachePrivateClass >(new CachePrivateClass());
            }
            // Existing names fetch the cache from the global stored map
            else if (caches.contains(path) && !caches[path].expired()) {
                return caches[path].lock();
            }
            // Otherwise a new named cache is created
            else
            {
                // Path names beginning with a colon are volatile
                if (path.startsWith(":")) {
                    boost::shared_ptr< CachePrivateClass > cache(new CachePrivateClass(path));
                    caches[path] = cache;
                    return cache;
                }
                // Otherwise the path name represents the path of the persistent cache directory
                else {
                    QFileInfo info(QDir::cleanPath(path));

                    // Ensure existence of cache path
                    if (!info.exists()) {
                        if (!QDir().mkpath(info.path())) {
                            // Couldn't create path
                            return boost::shared_ptr< CachePrivateClass >();
                        }
                    } else if (!info.isDir() || !info.isReadable() || !info.isWritable()) {
                        // Not a directory, nor readable/writable
                        return boost::shared_ptr< CachePrivateClass >();
                    }

                    boost::shared_ptr< CachePrivateClass > cache(new CachePrivateClass(path));
                    caches[path] = cache;

                    // Load all persisted items
                    QDir dir(info.dir());
                    dir.setFilter(QDir::Files |
                                  QDir::NoSymLinks |
                                  QDir::NoDotAndDotDot |
                                  QDir::Readable |
                                  QDir::Writable |
                                  QDir::CaseSensitive);
                    QStringList filters;
                    filters << "*.cache";
                    dir.setNameFilters(filters);

                    QFileInfoList list(dir.entryInfoList());
                    for (int i = 0; i < list.size(); ++i) {
                        QFileInfo info(list.at(i));
                        QFile file(info.path());
                        file.open(QIODevice::ReadOnly);
                        QDataStream str(&file);
                        CachedItemClass item;
                        str >> item;
                        file.close();
                        cache->items[item.id()] = qMakePair(item, false);
                    }

                    return cache;
                }
            }
        }
    };

    template< class Item >
    class Cache
    {
    public:
        typedef CachePrivate< Item > CachePrivateClass;
        typedef CachedItem< Item > CachedItemClass;

        // Constructors
        Cache(const QString & path = QString())
            : m(QMutex::Recursive)
        {
            setPath(path);
        }

        // Destructor
        ~Cache()
        {}

        void clear()
        {
            // Lock access
            QMutexLocker lock(&m);
            QMutexLocker lockShared(&d->mutex);

            if (isValid()) {
                d->items.clear();
                d->recentlyUsed.clear();
            }
        }

        bool exists(const QString & id) const
        {
            // Lock access
            QMutexLocker lock(&m);
            QMutexLocker lockShared(&d->mutex);

            return isValid() && d->items.contains(id);
        }

        void flush()
        {
            // Lock access
            QMutexLocker lock(&m);
            QMutexLocker lockShared(&d->mutex);

            // Flush only works on persistent caches
            if (isValid() && isPersistent()) {
                // FIXME
            }
        }

        Item get(const QString & id) const
        {
            // Lock access
            QMutexLocker lock(&m);
            QMutexLocker lockShared(&d->mutex);

            Q_ASSERT_X(isValid(), "d->items", "Cannot get item from Null cache");

            // Touch and return
            QPair< CachedItemClass, bool > & item = d->items[id];
            d->recentlyUsed.removeAll(id);
            d->recentlyUsed.append(id);
            item.first.touch();
            item.second = true;
            return item.first.isValid() ? item.first : Item();
        }

        CachedItemClass getMeta(const QString & id) const
        {
            // Lock access
            QMutexLocker lock(&m);
            QMutexLocker lockShared(&d->mutex);

            Q_ASSERT_X(isValid(), "d->items", "Cannot get item from Null cache");

            // Touch and return
            QPair< CachedItemClass, bool > & item = d->items[id];
            item.first.touch();
            item.second = true;
            return item.first;
        }

        bool isPersistent() const
        {
            // Lock access
            QMutexLocker lock(&m);
            QMutexLocker lockShared(&d->mutex);

            return isValid() && !d->path.isEmpty() && !d->path.startsWith(":");
        }

        bool isValid() const
        {
            // Lock access
            QMutexLocker lock(&m);
            QMutexLocker lockShared(&d->mutex);

            return (bool) d;
        }

        int maximumSize() const
        {
            // Lock access
            QMutexLocker lock(&m);
            QMutexLocker lockShared(&d->mutex);

            return isValid() ? d->maximumSize : 0;
        }

        QString path() const
        {
            // Lock access
            QMutexLocker lock(&m);
            QMutexLocker lockShared(&d->mutex);

            return isValid() ? d->path : QString();
        }

        void put(const Item & item, const QString & id)
        {
            // Lock access
            QMutexLocker lock(&m);
            QMutexLocker lockShared(&d->mutex);

            Q_ASSERT_X(isValid(), "d->items", "Cannot put item into Null cache");

            // Remove if present
            remove(id);

            // Check whether this now means we should remove an old item from
            // the cache
            d->resize();

            // Create new item
            d->items[id] = qMakePair(CachedItemClass(item, id, QDateTime::currentDateTime(), QDateTime::currentDateTime()), true);
            d->recentlyUsed.push_back(id);
        }

        void remove(const QString & id)
        {
            // Lock access
            QMutexLocker lock(&m);
            QMutexLocker lockShared(&d->mutex);

            if (isValid() && exists(id)) {
                d->items.remove(id);
                d->recentlyUsed.removeAll(id);
            }
        }

        void setMaximumSize(int maximumSize)
        {
            // Lock access
            QMutexLocker lock(&m);
            QMutexLocker lockShared(&d->mutex);

            d->maximumSize = maximumSize;
            d->resize();
        }

        bool setPath(const QString & path, bool create = false)
        {
            // Lock access
            QMutexLocker lock(&m);
            bool first = !d;
            if (!first) d->mutex.lock(); // Lock old shared mutex
            boost::shared_ptr< CachePrivateClass > oldCache(d);
            boost::shared_ptr< CachePrivateClass > newCache(CachePrivateClass::getCache(path));
            bool success = newCache;
            if (success) newCache->mutex.lock(); // Lock new shared mutex
            d = newCache;
            if (!first) oldCache->mutex.unlock(); // Unlock old shared mutex
            if (success) newCache->mutex.unlock(); // Unlock new shared mutex
            return true;
        }

    protected:
        QString filePathOf(const QString & id) const
        {
            QString result;
            QString filePath = path();
            if (true) {
                QString tmp(id);
                result=filePath + "/" + tmp.replace("/", "\\/") + ".cache";
            }
	    return result;
        }

    private:
        // Item
        boost::shared_ptr< CachePrivateClass > d;
        // Concurrent access to this cache object
        mutable QMutex m;
    };

} // namespace Utopia

#endif // Utopia_CACHE_H
