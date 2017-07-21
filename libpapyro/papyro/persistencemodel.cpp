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

#include <papyro/persistencemodel.h>

#include <papyro/abstractbibliography.h>
#include <papyro/collection.h>
#include <papyro/citation.h>
#include <papyro/cJSON.h>

#include <QAbstractItemModel>
#include <QDateTime>
#include <QDebug>
#include <QMetaProperty>
#include <QSet>
#include <QTextStream>

namespace Athenaeum
{

    static bool removeDir(QDir dir)
    {
        bool result = true;
        if (dir.exists()) {
            foreach (QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
                if (info.isDir()) {
                    result = removeDir(info.absoluteFilePath());
                } else {
                    result = QFile::remove(info.absoluteFilePath());
                }

                if (!result) {
                    return result;
                }
            }
            QString dirName(dir.dirName());
            result = dir.cdUp() && dir.rmdir(dirName);
        }
        return result;
    }




    PersistenceModel::PersistenceModel(QObject * parent)
        : QObject(parent)
    {}

    PersistenceModel::~PersistenceModel()
    {}

    bool PersistenceModel::isLoadable() const
    {
        return false;
    }

    bool PersistenceModel::isPurgeable() const
    {
        return false;
    }

    bool PersistenceModel::isSaveable() const
    {
        return false;
    }

    bool PersistenceModel::load(QAbstractItemModel * model) const
    {
        return false;
    }

    bool PersistenceModel::purge() const
    {
        return false;
    }

    bool PersistenceModel::save(QAbstractItemModel * model) const
    {
        return false;
    }




    class LocalPersistenceModelPrivate
    {
    public:
        LocalPersistenceModelPrivate()
            : purged(false)
        {}

        QDir path;
        bool purged;

        bool imprint() const
        {
            return path.mkpath("jsondb/.scratch") && path.mkpath("objects");
        }

    }; // class LocalPersistenceModelPrivate




    LocalPersistenceModel::LocalPersistenceModel(const QDir & path, QObject * parent)
        : PersistenceModel(parent), d(new LocalPersistenceModelPrivate)
    {
        d->path = path;
    }

    LocalPersistenceModel::~LocalPersistenceModel()
    {
        delete d;
    }

    bool LocalPersistenceModel::isLoadable() const
    {
        return true;
    }

    bool LocalPersistenceModel::isPurgeable() const
    {
        return true;
    }

    bool LocalPersistenceModel::isSaveable() const
    {
        return true;
    }

    bool LocalPersistenceModel::load(QAbstractItemModel * model) const
    {
        bool success = true;
        QString * errorMsg = 0;

        if (AbstractBibliography * bibliography = qobject_cast< AbstractBibliography * >(model)) {

            static QRegExp metadataRegExp("(\\w[\\w_\\d]+)\\s*=\\s*(\\S.*)?");
            static QRegExp dataFileRegExp("[a-f0-9]{2}");

            // Only existing paths can be loaded
            if (d->imprint()) {
                QDir jsonDir(d->path);
                jsonDir.cd("jsondb");
                QDir scratchDir(jsonDir);
                scratchDir.cd(".scratch");

                /////////////////////////////////////////////////////////////////////////////
                // Read metadata

                QFile metadataFile(d->path.absoluteFilePath("metadata"));
                if (metadataFile.exists() && metadataFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    const QMetaObject * metaObject(model->metaObject());
                    QString line;
                    while (!(line = QString::fromUtf8(metadataFile.readLine())).isEmpty()) {
                        metadataRegExp.exactMatch(line.trimmed());
                        QStringList captures = metadataRegExp.capturedTexts();
                        if (captures.size() == 3) {
                            int propertyIndex = metaObject->indexOfProperty(captures[1].toUtf8().constData());
                            if (propertyIndex >= 0) {
                                QMetaProperty metaProperty(metaObject->property(propertyIndex));
                                if (metaProperty.isWritable() && metaProperty.isStored() && metaProperty.name()[0] != '_') {
                                    metaProperty.write(model, captures[2]);
                                }
                            } else {
                                model->setProperty(captures[1].toUtf8().constData(), captures[2]);
                            }
                        }
                    }
                    metadataFile.close();

                    /////////////////////////////////////////////////////////////////////////
                    // For purposes of working out which files should be read or discarded,
                    // check when the scratch manifest was written (if one exists)

                    bool hasScratch = scratchDir.exists(".manifest");
                    QDateTime scratchModified;
                    if (hasScratch) { scratchModified = QFileInfo(scratchDir.absoluteFilePath(".manifest")).lastModified(); }

                    /////////////////////////////////////////////////////////////////////////
                    // Collect a list of all possible DB item files (union of jsondb and
                    // scratch dirs) that have the correct modified times.

                    QMap< QString, QFileInfo > manifest;
                    if (hasScratch) {
                        foreach (QFileInfo fileInfo, scratchDir.entryInfoList(QDir::Files)) {
                            QDateTime fileLastModified = fileInfo.lastModified();
                            QString baseName = fileInfo.baseName();
                            //qDebug() << "found scratch file" << baseName << fileLastModified << scratchModified << (fileLastModified >= scratchModified);
                            if (fileLastModified >= scratchModified && dataFileRegExp.exactMatch(baseName)) {
                                manifest[baseName] = fileInfo;
                            }
                        }
                    }
                    foreach (QFileInfo fileInfo, jsonDir.entryInfoList(QDir::Files)) {
                        QDateTime fileLastModified = fileInfo.lastModified();
                        QString baseName = fileInfo.baseName();
                        //qDebug() << "found jsondb file" << baseName << fileLastModified << scratchModified << (fileLastModified >= scratchModified);
                        if (dataFileRegExp.exactMatch(baseName)) {
                            if (!manifest.contains(baseName) || (hasScratch && fileLastModified >= scratchModified)) {
                                manifest[baseName] = fileInfo;
                            }
                        }
                    }

                    /////////////////////////////////////////////////////////////////////////
                    // Load each appropriate file from disk, adding its items to the model.

                    QMapIterator< QString, QFileInfo > iter(manifest);
                    while (iter.hasNext()) {
                        iter.next();
                        QFile dataFile(iter.value().absoluteFilePath());
                        if (dataFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                            QByteArray data(dataFile.readAll());
                            if (cJSON * json = cJSON_Parse(data.constData())) {
                                QVector< CitationHandle > newItems;
                                int count = cJSON_GetArraySize(json);
                                for (int i = 0; i < count; ++i) {
                                    if (cJSON * item = cJSON_GetArrayItem(json, i)) {
                                        newItems << Citation::fromJson(item);
                                    }
                                }
                                if (!newItems.isEmpty()) {
                                    // Add items to model
                                    bibliography->prependItems(newItems);
                                }
                                cJSON_Delete(json);
                            } else {
                                if (errorMsg) { *errorMsg = "Failed to parse one or more of the data files."; }
                                success = false;
                            }
                            dataFile.close();
                        } else {
                            if (errorMsg) { *errorMsg = "Failed to read one or more of the data files."; }
                            success = false;
                        }
                    }

                } else {
                    if (errorMsg) { *errorMsg = "Cannot read from metadata file."; }
                    success = false;
                }
            } else {
                if (errorMsg) { *errorMsg = "Cannot create database directories."; }
                success = false;
            }

            if (!success) {
                bibliography->setState(AbstractBibliography::CorruptState);
            }
        } else {
            if (errorMsg) { *errorMsg = "Not a bibliography."; }
            success = false;
        }

        return success;
    }

    bool LocalPersistenceModel::purge() const
    {
        bool success = true;

        // If this is a purged model, completely remove it from the filesystem
        if (d->path.exists()) {
            if (!removeDir(d->path)) {
                //if (errorMsg) { *errorMsg = "Unable to remove the collection's directory."; }
                success = false;
            }
        }

        return success;
    }

    bool LocalPersistenceModel::save(QAbstractItemModel * model) const
    {
        bool success = true;
        bool incremental = true;
        QString * errorMsg = 0;

        if (AbstractBibliography * bibliography = qobject_cast< AbstractBibliography * >(model)) {

            // If this is a purged model, completely remove it from the filesystem
            if (bibliography->state() == AbstractBibliography::PurgedState) {
                if (d->path.exists()) {
                    if (!removeDir(d->path)) {
                        if (errorMsg) { *errorMsg = "Unable to remove the collection's directory."; }
                        success = false;
                    }
                }
            } else if (d->imprint()) { // Ensure DB exists and is writable
                QDir jsonDir(d->path);
                jsonDir.cd("jsondb");
                QDir scratchDir(jsonDir);
                scratchDir.cd(".scratch");

                /////////////////////////////////////////////////////////////////////////////
                // Write metadata

                QFile metadataFile(d->path.absoluteFilePath("metadata"));
                if (metadataFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
                    QTextStream stream(&metadataFile);
                    const QMetaObject * metaObject(model->metaObject());
                    for (int index = 0; index < metaObject->propertyCount(); ++index) {
                        QMetaProperty metaProperty(metaObject->property(index));
                        // Only save properties I can/should write back later
                        if (metaProperty.isWritable() && metaProperty.isStored() && metaProperty.name()[0] != '_') {
                            QVariant value(metaProperty.read(model));
                            if (!value.isNull()) {
                                stream << metaProperty.name() << " = " << value.toString() << endl;
                            }
                        }
                    }
                    foreach (const QByteArray & key, model->dynamicPropertyNames()) {
                        stream << key.constData() << " = " << model->property(key).toString().toUtf8().constData() << endl;
                    }
                    metadataFile.close();

                    /////////////////////////////////////////////////////////////////////////
                    // Compile list of all items that need to be written to the database.

                    QMap< QString, QList< CitationHandle > > dirty;
                    QSet< QString > clean;
                    QVectorIterator< CitationHandle > iter(bibliography->items());
                    while (iter.hasNext()) {
                        CitationHandle item = iter.next();
                        if (!incremental || item->isDirty()) {
                            dirty[item->field(Citation::KeyRole).toString().mid(0, 2)].append(item);
                        } else {
                            clean.insert(item->field(Citation::KeyRole).toString().mid(0, 2));
                        }
                    }
                    clean.subtract(dirty.keys().toSet());

                    /////////////////////////////////////////////////////////////////////////
                    // Mark for removal any file that is no longer needed

                    foreach (QFileInfo fileInfo, jsonDir.entryInfoList(QDir::Files)) {
                        QString baseName = fileInfo.baseName();
                        if (!clean.contains(baseName) && !dirty.contains(baseName)) {
                            dirty[baseName]; // Ensure an empty dirty list, to be deleted later
                        }
                    }

                    /////////////////////////////////////////////////////////////////////////
                    // Write into the scratch directory a manifest of all the files that need
                    // to be written. If this fails, the database cannot be saved

                    QFile scratchManifestFile(scratchDir.filePath(".manifest"));
                    if (scratchManifestFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
                        scratchManifestFile.write(QStringList(dirty.keys()).join("\n").toUtf8());
                        scratchManifestFile.close();

                        /////////////////////////////////////////////////////////////////////
                        // Now write all the items that need saving into the scratch directory

                        QMapIterator< QString, QList< CitationHandle > > toWrite(dirty);
                        while (toWrite.hasNext()) {
                            toWrite.next();
                            QFile scratchFile(scratchDir.filePath(toWrite.key()));
                            QFile destinationFile(jsonDir.filePath(toWrite.key()));
                            if (!scratchFile.exists() || scratchFile.remove()) {
                                QList< cJSON * > itemsToWrite;
                                foreach (CitationHandle item, toWrite.value()) {
                                    itemsToWrite << item->toJson();
                                }
                                if (itemsToWrite.isEmpty()) {
                                    // Remove the file, as it is empty and therefore no longer needed
                                    destinationFile.remove();
                                } else if (scratchFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
                                    cJSON * json = cJSON_CreateArray();
                                    foreach (cJSON * item, itemsToWrite) {
                                        cJSON_AddItemToArray(json, item);
                                    }
                                    char * str = cJSON_PrintUnformatted(json);
                                    scratchFile.write(str);
                                    free(str);
                                    cJSON_Delete(json);
                                    scratchFile.close();

                                    if (true) { // FIXME check for item->write() above returning NULL
                                        // Relatively atomic move, ish
                                        if (!(destinationFile.exists() && !destinationFile.remove()) &&
                                            QFile::copy(scratchFile.fileName(), destinationFile.fileName())) {
                                            // Success!
                                            foreach (CitationHandle item, toWrite.value()) {
                                                item->setClean();
                                            }
                                        } else {
                                            if (errorMsg) { *errorMsg = "Unable to copy the scratch file into place."; }
                                            success = false;
                                            break;
                                        }
                                    } else {
                                        if (errorMsg) { *errorMsg = "Unable to generate scratch data for " + toWrite.key() + "."; }
                                        success = false;
                                        break;
                                    }
                                } else {
                                    if (errorMsg) { *errorMsg = "Unable to open the scratch file."; }
                                    success = false;
                                    break;
                                }
                            } else {
                                if (errorMsg) { *errorMsg = "Unable to remove old scratch file."; }
                                success = false;
                                break;
                            }
                        }
                    } else {
                        if (errorMsg) { *errorMsg = "Unable to write the scratch manifest file."; }
                        success = false;
                    }
                } else {
                    if (errorMsg) { *errorMsg = "Cannot write to metadata file."; }
                    success = false;
                }
            } else {
                if (errorMsg) { *errorMsg = "Cannot create database directories."; }
                success = false;
            }
        } else {
            if (errorMsg) { *errorMsg = "Not a bibliography."; }
            success = false;
        }

        return success;
    }




    class CollectionPersistenceModelPrivate
    {
    public:
        CollectionPersistenceModelPrivate()
            : purged(false)
        {}

        QDir path;
        bool purged;

    }; // class CollectionPersistenceModelPrivate




    CollectionPersistenceModel::CollectionPersistenceModel(const QDir & path, QObject * parent)
        : PersistenceModel(parent), d(new CollectionPersistenceModelPrivate)
    {
        d->path = path;
    }

    CollectionPersistenceModel::~CollectionPersistenceModel()
    {
        delete d;
    }

    bool CollectionPersistenceModel::isLoadable() const
    {
        return true;
    }

    bool CollectionPersistenceModel::isPurgeable() const
    {
        return true;
    }

    bool CollectionPersistenceModel::isSaveable() const
    {
        return true;
    }

    bool CollectionPersistenceModel::load(QAbstractItemModel * model) const
    {
        bool success = true;
        QString * errorMsg = 0;
        if (Collection * collection = qobject_cast< Collection * >(model)) {

            static QRegExp metadataRegExp("(\\w[\\w_\\d]+)\\s*=\\s*(\\S.*)?");
            static QRegExp dataFileRegExp("[a-f0-9]{2}");

            /////////////////////////////////////////////////////////////////////////////
            // Read metadata
            QFile metadataFile(d->path.absoluteFilePath("metadata"));
            if (metadataFile.exists() && metadataFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                const QMetaObject * metaObject(model->metaObject());
                QString line;
                while (!(line = QString::fromUtf8(metadataFile.readLine())).isEmpty()) {
                    metadataRegExp.exactMatch(line.trimmed());
                    QStringList captures = metadataRegExp.capturedTexts();
                    if (captures.size() == 3) {
                        int propertyIndex = metaObject->indexOfProperty(captures[1].toUtf8().constData());
                        if (propertyIndex >= 0) {
                            QMetaProperty metaProperty(metaObject->property(propertyIndex));
                            if (metaProperty.isWritable() && metaProperty.isStored() && metaProperty.name()[0] != '_') {
                                metaProperty.write(model, captures[2]);
                            }
                        } else {
                            model->setProperty(captures[1].toUtf8().constData(), captures[2]);
                        }
                    }
                }
                metadataFile.close();

                /////////////////////////////////////////////////////////////////////////
                // Read ids straight from file, separated by newlines

                QFile dataFile(d->path.absoluteFilePath("data"));
                if (dataFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream stream(&dataFile);
                    QVector< CitationHandle > items;
                    while (true) {
                        QString line = stream.readLine();
                        if (line.isNull()) {
                            break;
                        } else {
                            if (CitationHandle item = collection->sourceBibliography()->itemForKey(line.trimmed())) {
                                items << item;
                            }
                        }
                    }
                    collection->appendItems(items);
                    dataFile.close();
                } else {
                    if (errorMsg) { *errorMsg = "Cannot read from data file."; }
                    success = false;
                }

            } else {
                if (errorMsg) { *errorMsg = "Cannot read from metadata file."; }
                success = false;
            }

            if (!success) {
                collection->setState(AbstractBibliography::CorruptState);
            }
        } else {
            if (errorMsg) { *errorMsg = "Not a bibliography."; }
            success = false;
        }

        return success;
    }

    bool CollectionPersistenceModel::purge() const
    {
        bool success = true;

        // If this is a purged model, completely remove it from the filesystem
        if (d->path.exists()) {
            if (!removeDir(d->path)) {
                //if (errorMsg) { *errorMsg = "Unable to remove the collection's directory."; }
                success = false;
            }
        }

        return success;
    }

    bool CollectionPersistenceModel::save(QAbstractItemModel * model) const
    {
        bool success = true;
        //bool incremental = true;
        QString * errorMsg = 0;

        if (Collection * collection = qobject_cast< Collection * >(model)) {
            // If this is a purged model, completely remove it from the filesystem
            if (collection->state() != AbstractBibliography::PurgedState) {

                /////////////////////////////////////////////////////////////////////////////
                // Write metadata

                QFile metadataFile(d->path.absoluteFilePath("metadata"));
                if (metadataFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
                    QTextStream stream(&metadataFile);
                    const QMetaObject * metaObject(model->metaObject());
                    for (int index = 0; index < metaObject->propertyCount(); ++index) {
                        QMetaProperty metaProperty(metaObject->property(index));
                        // Only save properties I can/should write back later
                        if (metaProperty.isWritable() && metaProperty.isStored() && metaProperty.name()[0] != '_') {
                            QVariant value(metaProperty.read(model));
                            if (!value.isNull()) {
                                stream << metaProperty.name() << " = " << value.toString() << endl;
                            }
                        }
                    }
                    foreach (const QByteArray & key, model->dynamicPropertyNames()) {
                        stream << key.constData() << " = " << model->property(key).toString().toUtf8().constData() << endl;
                    }
                    metadataFile.close();

                    /////////////////////////////////////////////////////////////////////////
                    // Compile list of all items that need to be written to the database.

                    QFile dataFile(d->path.absoluteFilePath("data"));
                    if (dataFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
                        QMap< QString, QList< CitationHandle > > dirty;
                        QSet< QString > clean;
                        QVectorIterator< CitationHandle > iter(collection->items());
                        while (iter.hasNext()) {
                            CitationHandle item = iter.next();
                            QString key = item->field(Citation::KeyRole).toString() + "\n";
                            dataFile.write(key.toUtf8());
                        }
                        dataFile.close();
                    } else {
                        if (errorMsg) { *errorMsg = "Cannot write to data file."; }
                        success = false;
                    }
                } else {
                    if (errorMsg) { *errorMsg = "Cannot write to metadata file."; }
                    success = false;
                }
            }
        } else {
            if (errorMsg) { *errorMsg = "Not a collection."; }
            success = false;
        }

        return success;
    }

} // namespace Athenaeum
