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

#include <athenaeum/importer.h>
#include <athenaeum/librarymodel.h>

#include <QXmlDefaultHandler>
#include <QList>
#include <QXmlAttributes>
#include <QMap>
#include <QModelIndex>
#include <QStringList>

class MODSImporter : public QXmlDefaultHandler, public Athenaeum::Importer
{
public:
    bool import(Athenaeum::LibraryModel * model, QIODevice * io);
    QStringList extensions() const;
    QString name() const;
    bool supports(QIODevice * io) const;

public:
	MODSImporter();
	void pushState(QString state, QXmlAttributes atts);
	void popState();
	QString stateName(int indexOffset = 0);
	QXmlAttributes stateAtts(int indexOffset = 0);
    bool startElement ( const QString & namespaceURI, const QString & localName, const QString & qName, const QXmlAttributes & atts );
    bool endElement ( const QString & namespaceURI, const QString & localName, const QString & qName );
 	bool characters ( const QString & ch );
 	void writeMODSRecord();

 	bool importToDatabase(QString sourceFile);

protected:
	QString text;
	bool collectingText;

	QXmlAttributes _elementAttributes;

	QStringList _stateStackName;
	QList<QXmlAttributes> _stateStackAtts;

	QString _abstract;

	QString _articleTitle;
	QString _articleSubTitle;


	QString _publicationTitle;
	QString _publisher;

	Athenaeum::LibraryModel *_db;

	QModelIndex _currentIndex;
	QList< QPair< QStringList, QStringList > > _currentAuthors;
	QMap< QString, QString > _currentIdentifiers;
};
