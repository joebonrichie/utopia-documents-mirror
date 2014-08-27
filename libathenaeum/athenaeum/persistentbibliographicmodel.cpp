/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2014 Lost Island Labs
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

#include <athenaeum/persistentbibliographicmodel.h>
#include <athenaeum/persistentbibliographicmodel_p.h>
#include <athenaeum/bibliographicitem_p.h>
#include <athenaeum/bibliographicmimedata_p.h>
#include <athenaeum/cJSON.h>
#include <spine/Annotation.h>
#include <spine/Document.h>
//#include <papyro/annotator.h>
//#include <papyro/documentfactory.h>
//#include <papyro/papyrowindow.h>
//#include <papyro/utils.h>
#include <utopia2/fileformat.h>
#include <utopia2/networkaccessmanager.h>
#include <utopia2/node.h>
#include <utopia2/parser.h>
#include <utopia2/qt/uimanager.h>

#include <QMetaProperty>
#include <QMimeData>
#include <QNetworkReply>
#include <QThreadPool>
#include <QUrl>
#include <QDebug>

#define COLUMN_COUNT (AbstractBibliographicModel::PersistentRoleCount - Qt::UserRole)
#define _INTERNAL_MIMETYPE_PLAINTEXT "text/plain"
#define _INTERNAL_MIMETYPE_URILIST "text/uri-list"

namespace Athenaeum
{

    std::map< std::string, std::vector< std::string > > weightedProperties(const Spine::AnnotationSet & annotations, const std::map< std::string, std::string > & criteria)
    {
        typedef std::map< std::string, std::vector< std::string > > property_map;
        typedef std::map< std::string, std::string > string_map;

        std::map< std::string, std::map< int, Spine::AnnotationSet > > weighted;
        foreach (Spine::AnnotationHandle annotation, annotations) {
            bool matches = true;
            string_map::const_iterator c_iter(criteria.begin());
            string_map::const_iterator c_end(criteria.end());
            while (matches && c_iter != c_end) {
                if (c_iter->second.empty()) {
                    matches = matches && annotation->hasProperty(c_iter->first);
                } else {
                    matches = matches && annotation->hasProperty(c_iter->first, c_iter->second);
                }
                ++c_iter;
            }

            typedef std::pair< std::string, std::string > string_pair;
            if (matches) {
                bool ok;
                int weight = 0; // FIXME Papyro::qStringFromUnicode(annotation->getFirstProperty("session:weight")).toInt(&ok);
                if (!ok) {
                    weight = 0;
                }

                std::set< std::string > seen;
                foreach (const string_map::value_type & item, annotation->properties()) {
                    if (seen.find(item.first) == seen.end()) {
                        seen.insert(item.first);
                        weighted[item.first][weight].insert(annotation);
                    }
                }
            }
        }

        // Compile heaviest properties
        property_map properties;
        std::map< std::string, std::map< int, Spine::AnnotationSet > >::const_iterator iter(weighted.begin());
        std::map< std::string, std::map< int, Spine::AnnotationSet > >::const_iterator end(weighted.end());
        while (iter != end) {
            properties[iter->first] = (*--(--iter->second.end())->second.end())->getProperty(iter->first);
            ++iter;
        }

        return properties;
    }

    std::map< std::string, std::vector< std::string > > weightedProperties(const Spine::AnnotationSet & annotations, const std::vector< std::string > & keys, const std::map< std::string, std::string > & criteria)
    {
        typedef std::map< std::string, std::vector< std::string > > property_map;
        typedef std::map< std::string, std::string > string_map;

        std::map< std::string, std::map< int, Spine::AnnotationSet > > weighted;
        foreach (Spine::AnnotationHandle annotation, annotations) {
            bool matches = true;
            string_map::const_iterator c_iter(criteria.begin());
            string_map::const_iterator c_end(criteria.end());
            while (matches && c_iter != c_end) {
                if (c_iter->second.empty()) {
                    matches = matches && annotation->hasProperty(c_iter->first);
                } else {
                    matches = matches && annotation->hasProperty(c_iter->first, c_iter->second);
                }
                ++c_iter;
            }

            foreach (const std::string & key, keys) {
                if (matches && annotation->hasProperty(key)) {
                    bool ok;
                    int weight = 0; // FIXME Papyro::qStringFromUnicode(annotation->getFirstProperty("session:weight")).toInt(&ok);
                    if (!ok) {
                        weight = 0;
                    }
                    weighted[key][weight].insert(annotation);
                }
            }
        }

        // Compile heaviest properties
        property_map properties;
        std::map< std::string, std::map< int, Spine::AnnotationSet > >::const_iterator iter(weighted.begin());
        std::map< std::string, std::map< int, Spine::AnnotationSet > >::const_iterator end(weighted.end());
        while (iter != end) {
            properties[iter->first] = (*--(--iter->second.end())->second.end())->getProperty(iter->first);
            ++iter;
        }

        return properties;
    }

    std::vector< std::string > weightedProperty(const Spine::AnnotationSet & annotations, const std::string & key, const std::map< std::string, std::string > & criteria)
    {
        typedef std::map< std::string, std::string > string_map;

        std::map< int, Spine::AnnotationSet > weighted;
        foreach (Spine::AnnotationHandle annotation, annotations) {
            bool matches = true;
            string_map::const_iterator c_iter(criteria.begin());
            string_map::const_iterator c_end(criteria.end());
            while (matches && c_iter != c_end) {
                if (c_iter->second.empty()) {
                    matches = matches && annotation->hasProperty(c_iter->first);
                } else {
                    matches = matches && annotation->hasProperty(c_iter->first, c_iter->second);
                }
                ++c_iter;
            }
            if (matches && annotation->hasProperty(key)) {
                int weight = 0; // FIXME Papyro::qStringFromUnicode(annotation->getFirstProperty("session:weight")).toInt();
                weighted[weight].insert(annotation);
            }
        }

        // Set new authors
        if (!weighted.empty()) {
            return (*--(--weighted.end())->second.end())->getProperty(key);
        } else {
            return std::vector< std::string >();
        }
    }

    std::string weightedFirstProperty(const Spine::AnnotationSet & annotations, const std::string & key, const std::map< std::string, std::string > & criteria)
    {
        std::vector< std::string > values(weightedProperty(annotations, key, criteria));
        if (values.empty()) {
            return std::string();
        } else {
            return values[0];
        }
    }

    UrlImporter::UrlImporter(const QUrl & url, QObject * parent)
        : QObject(parent), QRunnable(), url(url)
    {
        setAutoDelete(false);
    }

    UrlImporter::~UrlImporter()
    {}

    void UrlImporter::run()
    {
        BibliographicItem * item = 0;

        if (Utopia::FileFormat * format = Utopia::FileFormat::get("PDF")) {
            //qDebug() << "importing" << url;

            // Resolve data from URL
            boost::shared_ptr< Utopia::NetworkAccessManager > netAccess(Utopia::NetworkAccessManagerMixin().networkAccessManager());
            QNetworkReply * reply = netAccess->getAndBlock(QNetworkRequest(url));

            // Attempt to load the data using Crackle
            Spine::DocumentHandle document;
            QByteArray bytes(reply->readAll());
            //foreach (Papyro::DocumentFactory * factory, Utopia::instantiateAllExtensions< Papyro::DocumentFactory >()) {
            //    if (!document) {
            //        document = factory->create(bytes);
            //    }
            //    delete factory;
            //}
            reply->deleteLater();

            if (!document) {
                return; // FIXME deal with errors here
            }

            // Run the metadata resolver over the document
            //Papyro::Annotator * annotator = Utopia::instantiateExtension< Papyro::Annotator >("metadata.Metadata");
            //if (document && annotator && annotator->prepare(document)) {
            //    delete annotator;
            //}

            // Get the metadata from the document's annotations
            QStringList id_names;
            id_names << "doi" << "pmid" << "arxivid" << "pmcid" << "pii" << "issn";
            QVariantMap metadata;

            typedef std::map< std::string, std::string > string_map;
            string_map criteria;
            criteria["concept"] = "DocumentIdentifier";

            std::vector< std::string > fields;
            fields.push_back("property:title");
            fields.push_back("property:authors");
            fields.push_back("property:identifiers");
            fields.push_back("property:keywords");
            fields.push_back("property:publication-title");
            fields.push_back("property:volume");
            fields.push_back("property:issue");
            fields.push_back("property:page-from");
            fields.push_back("property:page-to");
            fields.push_back("property:year");

            std::map< std::string, std::vector< std::string > > properties(weightedProperties(document->annotations("Document Metadata"), criteria));
            std::map< std::string, std::vector< std::string > >::const_iterator p_iter(properties.begin());
            std::map< std::string, std::vector< std::string > >::const_iterator p_end(properties.end());
            while (p_iter != p_end) {
                const std::string & full_key = p_iter->first;
                if (full_key.compare(0, 9, "property:") == 0) {
                    QString key = ""; // FIXME Papyro::qStringFromUnicode(full_key.substr(9));
                    const std::vector< std::string > & values = p_iter->second;
                    QString value = ""; // FIXME Papyro::qStringFromUnicode(*values.begin());

                    if (key == "authors" || key == "keywords") { // QStringList
                        QStringList list;
                        foreach (const std::string & item, values) {
                            list << ""; // FIXME Papyro::qStringFromUnicode(item);
                        }
                        metadata[key] = list;
                    } else if (id_names.contains(key)) { // Add to QVariantMap
                        QVariantMap identifiers(metadata.value("identifiers").toMap());
                        identifiers[key] = value;
                        metadata["identifiers"] = identifiers;
                    } else { // Normal QVariant
                        metadata[key] = value;
                    }
                }

                ++p_iter;
            }

            if (!metadata.isEmpty()) {
                if (url.isLocalFile()) {
                    metadata["pdf"] = url;
                } else {
                    metadata["url"] = url;
                }

                item = BibliographicItem::fromMap(metadata);
            }

            //qDebug() << "imported" << metadata;
        }
        emit finished(item);
    }




    PersistentBibliographicModelPrivate::PersistentBibliographicModelPrivate(PersistentBibliographicModel * m, const QDir & path)
        : m(m), readOnly(false), path(path.absolutePath()), state(AbstractBibliographicModel::IdleState), importMutex(QMutex::Recursive), importing(0)
    {}

    void PersistentBibliographicModelPrivate::dispatchImporter(const QUrl & url)
    {
        QMutexLocker guard(&importMutex);
        m->setState(AbstractBibliographicCollection::BusyState);
        ++importing;
        UrlImporter * importer = new UrlImporter(url, this);
        connect(importer, SIGNAL(finished(Athenaeum::BibliographicItem *)),
                this, SLOT(onUrlImporterFinished(Athenaeum::BibliographicItem *)));
        QThreadPool::globalInstance()->start(importer);
    }

    bool PersistentBibliographicModelPrivate::imprint()
    {
        return path.mkpath("jsondb/.scratch") && path.mkpath("objects");
    }

    bool PersistentBibliographicModelPrivate::isIdle()
    {
        return state == AbstractBibliographicCollection::IdleState;
    }

    bool PersistentBibliographicModelPrivate::load(QString * errorMsg)
    {
        static QRegExp metadataRegExp("(\\w[\\w_\\d]+)\\s*=\\s*(\\S.*)?");
        static QRegExp dataFileRegExp("[a-f0-9]{2}");
        QMutexLocker guard(&mutex);
        bool success = true;

        // Only existing paths can be loaded
        if (imprint()) {
            QDir jsonDir(path);
            jsonDir.cd("jsondb");
            QDir scratchDir(jsonDir);
            scratchDir.cd(".scratch");

            /////////////////////////////////////////////////////////////////////////////
            // Read metadata

            QFile metadataFile(path.absoluteFilePath("metadata"));
            if (metadataFile.exists() && metadataFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                const QMetaObject * metaObject(m->metaObject());
                QString line;
                while (!(line = QString::fromUtf8(metadataFile.readLine())).isEmpty()) {
                    metadataRegExp.exactMatch(line.trimmed());
                    QStringList captures = metadataRegExp.capturedTexts();
                    if (captures.size() == 3) {
                        int propertyIndex = metaObject->indexOfProperty(captures[1].toAscii().constData());
                        if (propertyIndex >= 0) {
                            QMetaProperty metaProperty(metaObject->property(propertyIndex));
                            if (metaProperty.isWritable() && metaProperty.isStored() && metaProperty.name()[0] != '_') {
                                metaProperty.write(m, captures[2]);
                            }
                        } else {
                            m->setProperty(captures[1].toAscii().constData(), captures[2]);
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
                            QVector< BibliographicItem * > newItems;
                            int count = cJSON_GetArraySize(json);
                            for (int i = 0; i < count; ++i) {
                                if (cJSON * item = cJSON_GetArrayItem(json, i)) {
                                    newItems << BibliographicItem::fromJson(item);
                                }
                            }
                            if (!newItems.isEmpty()) {
                                // Add items to model
                                m->beginInsertRows(QModelIndex(), items.size(), items.size() + newItems.size() - 1);
                                items += newItems;
                                m->endInsertRows();
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
            m->setState(AbstractBibliographicModel::CorruptState);
        }
        return success;
    }

    void PersistentBibliographicModelPrivate::onUrlImporterFinished(BibliographicItem * item)
    {
        // Deal with incoming item
        if (item) {
            m->appendItem(item);
        }

        // Set the next importer off
        QMutexLocker guard(&importMutex);
        --importing;
        if (importQueue.isEmpty()) {
            if (importing == 0) {
                //qDebug() << "import finished";
                m->setState(AbstractBibliographicCollection::IdleState);
            }
        } else {
            dispatchImporter(importQueue.takeFirst());
        }
    }

    void PersistentBibliographicModelPrivate::queueUrlForImport(const QUrl & url)
    {
        QMutexLocker guard(&importMutex);
        importQueue << url;
        if (importing < 4) {
            dispatchImporter(importQueue.takeFirst());
        }
    }

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

    bool PersistentBibliographicModelPrivate::save(bool incremental, QString * errorMsg)
    {
        QMutexLocker guard(&mutex);
        bool success = true;

        // If this is a purged model, completely remove it from the filesystem
        if (m->state() == AbstractBibliographicModel::PurgedState) {
            if (path.exists()) {
                if (!removeDir(path)) {
                    if (errorMsg) { *errorMsg = "Unable to remove the collection's directory."; }
                    success = false;
                }
            }
        } else if (imprint()) { // Ensure DB exists and is writable
            QDir jsonDir(path);
            jsonDir.cd("jsondb");
            QDir scratchDir(jsonDir);
            scratchDir.cd(".scratch");

            /////////////////////////////////////////////////////////////////////////////
            // Write metadata

            QFile metadataFile(path.absoluteFilePath("metadata"));
            if (metadataFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
                QTextStream stream(&metadataFile);
                const QMetaObject * metaObject(m->metaObject());
                for (int index = 0; index < metaObject->propertyCount(); ++index) {
                    QMetaProperty metaProperty(metaObject->property(index));
                    // Only save properties I can/should write back later
                    if (metaProperty.isWritable() && metaProperty.isStored() && metaProperty.name()[0] != '_') {
                        QVariant value(metaProperty.read(m));
                        if (!value.isNull()) {
                            stream << metaProperty.name() << " = " << value.toString() << endl;
                        }
                    }
                }
                foreach (const QByteArray & key, m->dynamicPropertyNames()) {
                    stream << key.constData() << " = " << m->property(key).toString().toUtf8().constData() << endl;
                }
                metadataFile.close();

                /////////////////////////////////////////////////////////////////////////
                // Compile list of all items that need to be written to the database.

                QMap< QString, QList< BibliographicItem * > > dirty;
                QSet< QString > clean;
                QVectorIterator< BibliographicItem * > iter(items);
                while (iter.hasNext()) {
                    BibliographicItem * item = iter.next();
                    if (!incremental || item->isDirty()) {
                        dirty[item->field(AbstractBibliographicModel::KeyRole).toString().mid(0, 2)].append(item);
                    } else {
                        clean.insert(item->field(AbstractBibliographicModel::KeyRole).toString().mid(0, 2));
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

                    QMapIterator< QString, QList< BibliographicItem * > > toWrite(dirty);
                    while (toWrite.hasNext()) {
                        toWrite.next();
                        QFile scratchFile(scratchDir.filePath(toWrite.key()));
                        QFile destinationFile(jsonDir.filePath(toWrite.key()));
                        if (!scratchFile.exists() || scratchFile.remove()) {
                            QList< cJSON * > itemsToWrite;
                            foreach (BibliographicItem * item, toWrite.value()) {
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
                                        foreach (BibliographicItem * item, toWrite.value()) {
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

        return success;
    }




    PersistentBibliographicModel::PersistentBibliographicModel(const QDir & path, QObject * parent)
        : AbstractBibliographicModel(parent), d(new PersistentBibliographicModelPrivate(this, path))
    {
        d->load();
    }

    PersistentBibliographicModel::~PersistentBibliographicModel()
    {
        QString msg;
        d->save(true, &msg);
        delete d;
    }

    void PersistentBibliographicModel::appendItem(BibliographicItem * item)
    {
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        d->items.append(item);
        item->setDirty();
        endInsertRows();
    }

    void PersistentBibliographicModel::clear()
    {
        // FIXME
    }

    int PersistentBibliographicModel::columnCount(const QModelIndex & index) const
    {
        return index.isValid() ? 0 : COLUMN_COUNT;
    }

    int PersistentBibliographicModel::count(ItemFlags flags) const
    {
        if (flags == AllFlags) {
            return rowCount();
        } else {
            return 0;
        }
    }

    QVariant PersistentBibliographicModel::data(const QModelIndex & index, int role) const
    {
        if (BibliographicItem * item = static_cast< BibliographicItem * >(index.internalPointer())) {
            switch (role) {
            case AbstractBibliographicModel::ItemRole:
                return QVariant::fromValue(item);
            case AbstractBibliographicModel::FullTextSearchRole:
                // Ignore some fields for searching purposes
                switch (index.column() + Qt::UserRole) {
                case AbstractBibliographicModel::KeyRole:
                case AbstractBibliographicModel::TypeRole:
                case AbstractBibliographicModel::DocumentUriRole:
                case AbstractBibliographicModel::UrlRole:
                case AbstractBibliographicModel::PdfRole:
                case AbstractBibliographicModel::ItemFlagsRole:
                case AbstractBibliographicModel::ItemStateRole:
                    return QVariant();
                default:
                    break;
                }
            case Qt::EditRole:
            case Qt::DisplayRole:
                // Post processing of values
                switch (index.column() + Qt::UserRole) {
                case AbstractBibliographicModel::IdentifiersRole: {
                    QVariantMap identifiers(item->field(AbstractBibliographicModel::IdentifiersRole).toMap());
                    QMapIterator< QString, QVariant > iter(identifiers);
                    QStringList idText;
                    while (iter.hasNext()) {
                        iter.next();
                        idText << (iter.key() + ":" + iter.value().toString());
                    }
                    return idText.join("\n");
                }
                case AbstractBibliographicModel::AuthorsRole: {
                    QStringList authors(item->field(AbstractBibliographicModel::AuthorsRole).toStringList());
                    QStringList authorStrings;
                    foreach (const QString & author, authors) {
                        QString authorString;
                        foreach (const QString & forename, author.section(", ", 1, 1).split(" ")) {
                            authorString += forename.left(1).toUpper() + ". ";
                        }
                        authorString += author.section(", ", 0, 0);
                        authorString = authorString.trimmed();
                        if (!authorString.isEmpty()) {
                            authorStrings << authorString;
                        }
                    }
                    if (!authorStrings.isEmpty()) {
                        QString authorString;
                        if (authorStrings.size() == 1) {
                            authorString = authorStrings.at(0) + ".";
                        } else {
                            if (authorStrings.size() > 2) {
                                authorString = QStringList(authorStrings.mid(0, authorStrings.size() - 2)).join(", ") + ", ";
                            }
                            authorString += authorStrings.at(authorStrings.size() - 2) + " and " + authorStrings.at(authorStrings.size() - 1);
                        }
                        return authorString;
                    }
                    break;
                }
                case AbstractBibliographicModel::KeywordsRole:
                    return item->field(AbstractBibliographicModel::KeywordsRole).toStringList().join(", ");
                case AbstractBibliographicModel::DateModifiedRole:
                    return item->field(AbstractBibliographicModel::DateModifiedRole).toDateTime().toString(Qt::ISODate);
                case AbstractBibliographicModel::KeyRole:
                case AbstractBibliographicModel::TitleRole:
                case AbstractBibliographicModel::SubtitleRole:
                case AbstractBibliographicModel::UrlRole:
                case AbstractBibliographicModel::VolumeRole:
                case AbstractBibliographicModel::IssueRole:
                case AbstractBibliographicModel::YearRole:
                case AbstractBibliographicModel::PageFromRole:
                case AbstractBibliographicModel::PageToRole:
                case AbstractBibliographicModel::AbstractRole:
                case AbstractBibliographicModel::PublicationTitleRole:
                case AbstractBibliographicModel::PublisherRole:
                case AbstractBibliographicModel::TypeRole:
                case AbstractBibliographicModel::DocumentUriRole:
                case AbstractBibliographicModel::PdfRole:
                    return item->field(index.column() + Qt::UserRole);
                default:
                    // Should never happen
                    qWarning("data: invalid display value column %d", index.column());
                    break;
                }
                break;
            case AbstractBibliographicModel::KeyRole:
            case AbstractBibliographicModel::TitleRole:
            case AbstractBibliographicModel::SubtitleRole:
            case AbstractBibliographicModel::AuthorsRole:
            case AbstractBibliographicModel::UrlRole:
            case AbstractBibliographicModel::VolumeRole:
            case AbstractBibliographicModel::IssueRole:
            case AbstractBibliographicModel::YearRole:
            case AbstractBibliographicModel::PageFromRole:
            case AbstractBibliographicModel::PageToRole:
            case AbstractBibliographicModel::AbstractRole:
            case AbstractBibliographicModel::PublicationTitleRole:
            case AbstractBibliographicModel::PublisherRole:
            case AbstractBibliographicModel::DateModifiedRole:
            case AbstractBibliographicModel::KeywordsRole:
            case AbstractBibliographicModel::TypeRole:
            case AbstractBibliographicModel::IdentifiersRole:
            case AbstractBibliographicModel::DocumentUriRole:
            case AbstractBibliographicModel::PdfRole:
            case AbstractBibliographicModel::ItemFlagsRole:
            case AbstractBibliographicModel::ItemStateRole:
                return item->field(role);
            default:
                break;
            }
        }

        return QVariant();
    }

    bool PersistentBibliographicModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
    {
        //qDebug() << "== PersistentBibliographicModel::dropMimeData" << data << action << row << column << parent;
        if (data->hasUrls()) { // Dragging PDFs and other files into a library
            foreach (QUrl url, data->urls()) {
                d->queueUrlForImport(url);
            }
        } else if (data->hasFormat(_INTERNAL_MIMETYPE_BIBLIOGRAPHICITEMS)) {
            if (const BibliographicMimeData * bibData = qobject_cast< const BibliographicMimeData * >(data)) {
                QMap< int, QModelIndex > ordered;
                foreach (const QModelIndex & index, bibData->indexes()) {
                    ordered[index.row()] = index;
                }
                QMapIterator< int, QModelIndex > iter(ordered);
                iter.toBack();
                while (iter.hasPrevious()) {
                    iter.previous();
                    QModelIndex index = iter.value();
                    if (const AbstractBibliographicModel * model = qobject_cast< const AbstractBibliographicModel * >(index.model())) {
                        BibliographicItem * copy = new BibliographicItem(model->itemAt(index.row()));
                        appendItem(copy);
                    }
                }
            }
        }

        return true;
    }

    Qt::ItemFlags PersistentBibliographicModel::flags(const QModelIndex & index) const
    {
        Qt::ItemFlags flags(AbstractBibliographicModel::flags(index));
        if (index.isValid()) {
            flags |= Qt::ItemIsDragEnabled;
        } else {
            flags |= Qt::ItemIsDropEnabled;
        }
        return flags;
    }

    QVariant PersistentBibliographicModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role == Qt::DisplayRole && section >= 0) {
            if (orientation == Qt::Horizontal && section < COLUMN_COUNT) {
                switch (section + Qt::UserRole) {
                case AbstractBibliographicModel::KeyRole: return QString("Key");
                case AbstractBibliographicModel::TitleRole: return QString("Title");
                case AbstractBibliographicModel::SubtitleRole: return QString("Subtitle");
                case AbstractBibliographicModel::AuthorsRole: return QString("Authors");
                case AbstractBibliographicModel::UrlRole: return QString("Url");
                case AbstractBibliographicModel::VolumeRole: return QString("Volume");
                case AbstractBibliographicModel::IssueRole: return QString("Issue");
                case AbstractBibliographicModel::YearRole: return QString("Year");
                case AbstractBibliographicModel::PageFromRole: return QString("Start Page");
                case AbstractBibliographicModel::PageToRole: return QString("End Page");
                case AbstractBibliographicModel::AbstractRole: return QString("Abstract");
                case AbstractBibliographicModel::PublicationTitleRole: return QString("Publication Title");
                case AbstractBibliographicModel::PublisherRole: return QString("Publisher");
                case AbstractBibliographicModel::DateModifiedRole: return QString("Date Modified");
                case AbstractBibliographicModel::KeywordsRole: return QString("Keywords");
                case AbstractBibliographicModel::TypeRole: return QString("Type");
                case AbstractBibliographicModel::IdentifiersRole: return QString("Identifiers");
                case AbstractBibliographicModel::DocumentUriRole: return QString("Document URI");
                case AbstractBibliographicModel::PdfRole: return QString("Filename");
                case AbstractBibliographicModel::ItemFlagsRole: return QString("Flags");
                default: break;
                }
            } else if (orientation == Qt::Vertical && section < d->items.size()) {
                return QString::number(section + 1);
            }
        }
        return QVariant();
    }

    QModelIndex PersistentBibliographicModel::index(int row, int column, const QModelIndex & parent) const
    {
        // Only top-level indices can be created
        if (parent.isValid() || !hasIndex(row, column, parent)) {
            return QModelIndex();
        } else {
            return createIndex(row, column, (void *) d->items.at(row));
        }
    }

    void PersistentBibliographicModel::insertItem(BibliographicItem * before, BibliographicItem * item)
    {
        int idx = 0;
        QVector< BibliographicItem * >::iterator where(d->items.begin());
        while (where != d->items.end() && *where != before) { ++where; ++idx; }
        beginInsertRows(QModelIndex(), idx, idx);
        d->items.insert(where, item);
        item->setDirty();
        endInsertRows();
    }

    bool PersistentBibliographicModel::insertRows(int row, int count, const QModelIndex & parent)
    {
        QMutexLocker guard(&d->mutex);

        if (parent.isValid() || row < 0 || row > d->items.size()) {
            return false;
        }

        beginInsertRows(parent, row, row + count - 1);
        d->items.insert(row, count, (BibliographicItem *) 0);
        for (int i = row; i < row + count; ++i) {
            d->items[i] = new BibliographicItem();
        }
        endInsertRows();

        return true;
    }

    BibliographicItem * PersistentBibliographicModel::itemAt(int idx) const
    {
        return d->items.at(idx);
    }

    QList< BibliographicItem * > PersistentBibliographicModel::items() const
    {
        return d->items.toList();
    }

    bool PersistentBibliographicModel::isReadOnly() const
    {
        return d->readOnly;
    }

    bool PersistentBibliographicModel::isPersistent() const
    {
        return true;
    }

    QMimeData * PersistentBibliographicModel::mimeData(const QModelIndexList & indexes) const
    {
        return AbstractBibliographicModel::mimeData(indexes);
    }

    QStringList PersistentBibliographicModel::mimeTypes() const
    {
        QStringList mimeTypes(AbstractBibliographicModel::mimeTypes());
        mimeTypes << "text/plain" << "text/uri-list";
        return mimeTypes;
    }

    QModelIndex PersistentBibliographicModel::parent(const QModelIndex & /*index*/) const
    {
        // No parent, as there is no hierarchy
        return QModelIndex();
    }

    QDir PersistentBibliographicModel::path() const
    {
        return d->path;
    }

    qreal PersistentBibliographicModel::progress() const
    {
        return -1.0;
    }

    void PersistentBibliographicModel::purge()
    {
        clear();
        setState(AbstractBibliographicModel::PurgedState);
    }

    bool PersistentBibliographicModel::removeItem(BibliographicItem * item)
    {
        int row = d->items.indexOf(item);
        if (row >= 0) {
            // Remove item
            return removeRow(row);
        } else {
            // Not found
            return false;
        }
    }

    bool PersistentBibliographicModel::removeRows(int row, int count, const QModelIndex & parent)
    {
        QMutexLocker guard(&d->mutex);

        if (parent.isValid() || row < 0 || (count - row) > d->items.size()) {
            return false;
        } else {
            beginRemoveRows(parent, row, row + count - 1);
            for (int i = row; i < row + count; ++i) {
                delete d->items[i];
            }
            d->items.remove(row, count);
            endRemoveRows();
            return true;
        }
    }

    int PersistentBibliographicModel::rowCount(const QModelIndex & index) const
    {
        // Only the root item has children
        return index.isValid() ? 0 : d->items.size();
    }

    bool PersistentBibliographicModel::setData(const QModelIndex & index, const QVariant & value, int role)
    {
        QMutexLocker guard(&d->mutex);

        if (index.model() != this)
            return false;

        // Only top level items can be set
        if (BibliographicItem * item = static_cast< BibliographicItem * >(index.internalPointer())) {
            if (role == Qt::DisplayRole) {
                item->setField(index.column() + Qt::UserRole, value);
                return true;
            } else if (role >= Qt::UserRole && role < RoleCount) {
                item->setField(role, value);
                return true;
            }
        }

        return false;
    }

	void PersistentBibliographicModel::setProgress(qreal /*progress*/)
	{
	    /* no-op */
	}

    void PersistentBibliographicModel::setReadOnly(bool readOnly)
    {
        d->readOnly = readOnly;
    }

    void PersistentBibliographicModel::setState(AbstractBibliographicModel::State state)
    {
	    if (d->state != state) {
	        d->state = state;
	        emit stateChanged(state);
	    }
    }

    void PersistentBibliographicModel::setTitle(const QString & title)
    {
        if (d->title != title) {
            d->title = title;
            emit titleChanged(title);
        }
    }

    AbstractBibliographicModel::State PersistentBibliographicModel::state() const
    {
        // By default persistent models are idle, though they can be corrupt
        return d->state;
    }

    BibliographicItem * PersistentBibliographicModel::takeItemAt(int idx)
    {
        QMutexLocker guard(&d->mutex);
        BibliographicItem * taken = 0;

        if (idx >= 0 && idx < d->items.size()) {
            beginRemoveRows(QModelIndex(), idx, idx);
            taken = d->items.at(idx);
            d->items.remove(idx);
            endRemoveRows();
        }

        return taken;
    }

    QString PersistentBibliographicModel::title() const
    {
        return d->title;
    }

} // namespace Athenaeum
