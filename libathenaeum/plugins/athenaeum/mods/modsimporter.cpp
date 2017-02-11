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

#include "modsimporter.h"

#include <QDebug>
#include <QDir>
#include <QVariant>
#include <QXmlInputSource>
#include <QXmlSimpleReader>


bool MODSImporter::import(Athenaeum::LibraryModel * model, QIODevice * io)
{
    // Set the model for future use
	this->_db = model;

	QXmlSimpleReader xmlReader;
	QXmlInputSource source(io);
	xmlReader.setContentHandler(this);
	xmlReader.parse(&source);

    // FIXME
    return true;
}

QStringList MODSImporter::extensions() const
{
    QStringList exts;
    exts << "mods.xml";
    return exts;
}

QString MODSImporter::name() const
{
    return "MODS XML";
}

bool MODSImporter::supports(QIODevice * io) const
{
    return false;
}







MODSImporter::MODSImporter()
{}

void MODSImporter::pushState(QString state, QXmlAttributes atts)
{
	this->_stateStackName.append(state);
	this->_stateStackAtts.append(atts);
}

void MODSImporter::popState()
{
	this->_stateStackAtts.removeLast();
	this->_stateStackName.removeLast();
}

QString MODSImporter::stateName(int indexOffset)
{
	if (this->_stateStackName.isEmpty())
	{
		return "";
	}
	else
	{
		int index = this->_stateStackName.count() - 1 + indexOffset;
		if (index < 0)
		{
			return QString();
		}
		else
		{
			QString string = this->_stateStackName.at(index);
			return string;
		}
	}
}

QXmlAttributes MODSImporter::stateAtts(int indexOffset)
{
	if (this->_stateStackAtts.isEmpty())
	{
		return QXmlAttributes();
	}
	else
	{
		int index = this->_stateStackAtts.count() - 1 + indexOffset;
		if (index < 0)
		{
			return QXmlAttributes();
		}
		else
		{
			QXmlAttributes atts = this->_stateStackAtts.at(index);
			return atts;
		}
	}
}

int count = 0;

/*
void MODSImporter::writeMODSRecord()
{
	if (this->_db->addRecord(this->_record))
	{
		// all is well, and the record was imported correctly
	}
	else
	{
		//qDebug() << "Failed to add record";
	}

	delete this->_record;
	this->_record = NULL;
}
*/


	// XML Handler Functions

bool MODSImporter::startElement ( const QString & /* namespaceURI */, const QString & localName, const QString & /* qName */, const QXmlAttributes & atts )
{
	this->pushState(localName,atts);

	if (this->stateName() == "mods" && this->stateName(-1) == "modsCollection")
	{
	    int index = this->_db->rowCount();
	    this->_db->insertRow(index);
		this->_currentIndex = this->_db->index(index, 0);
		this->_currentAuthors.clear();
		this->_currentIdentifiers.clear();
	}
	else if (this->stateName() == "name" && this->stateName(-1) == "mods")
	{
		this->_currentAuthors.append(qMakePair(QStringList(), QStringList()));
	}

	return true;
}

bool MODSImporter::endElement( const QString & /* namespaceURI */, const QString & localName, const QString & /* qName */ )
{
	QString elementName = localName;

	if (this->stateName() == elementName)
	{
		if (elementName == "mods")
		{
			this->_db->setAuthors(this->_currentIndex, this->_currentAuthors);
			this->_db->setIdentifiers(this->_currentIndex, this->_currentIdentifiers);
            // FIXME : Scan for duplicates

			this->_currentIndex = QModelIndex();
//			this->writeMODSRecord();
//			delete this->_record;
//			this->_record= NULL;
		}
		this->popState();
	}
	else
	{
		qDebug() << "Badly formed xml";
	}
	return true;
}

/*
bool MODSImporter::importToDatabase(QString sourceFile)
{
    QXmlSimpleReader xmlReader;
    QXmlInputSource *source = new QXmlInputSource(&file);

    MODSImporter *handler = new MODSImporter(database);
    xmlReader.setContentHandler(handler);
    xmlReader.parse(source);
}
*/

bool MODSImporter::characters ( const QString & ch )
{
	if (this->_currentIndex.isValid())
	{
		QString text = ch;

		// FIXME Does this method collate characters?

		QString element = this->stateName();
		QString parent = this->stateName(-1);
		QString grandParent = this->stateName(-2);
	//	QString greatGrandParent = this->stateName(-3);

		if (element == "title" && parent == "titleInfo" && grandParent == "mods")
		{
			// occasionally, MODS files converted from endnote contain an additional 'title'
			// which has got the journal name in by mistake... this always seems to come
			// after the real title, so if we've already found a better title, ignore the dodgy one
			if (this->_db->title(this->_currentIndex) == "")
			{
				this->_db->setTitle(this->_currentIndex, text);
			}
		}
		else if (element == "subTitle" && parent == "titleInfo" && grandParent == "mods")
		{
			this->_db->setSubTitle(this->_currentIndex, text);
		}
		else if (element == "namePart" && parent == "name" && grandParent == "mods")
		{
			QXmlAttributes attrs = this->stateAtts();
			if (attrs.value("type") == "given")
			{
				this->_currentAuthors.last().first << text;
			}
			else if (attrs.value("type") == "family")
			{
				this->_currentAuthors.last().second << text;
			}
		}
		else if (element == "url" && parent == "location" && grandParent == "mods")
		{
			this->_db->setUrl(this->_currentIndex, text);
		}
		else if (element == "genre" && parent == "relatedItem" && grandParent == "mods")
		{
			this->_db->setType(this->_currentIndex, text);
		}
		else if (element == "abstract" && parent == "mods")
		{
			this->_db->setAbstract(this->_currentIndex, text);
		}
		else if (element == "title" && parent == "titleInfo" && grandParent == "relatedItem")
		{
			this->_db->setPublicationTitle(this->_currentIndex, text);
		}
		else if (element == "publisher" && parent == "originInfo" && grandParent == "relatedItem")
		{
			this->_db->setPublisher(this->_currentIndex, text);
		}
		else if (element == "topic" && parent == "subject")
		{

			text.remove("*");

			if (text.contains(QRegExp("[,/]")))
			{
				QStringList keywords = text.split(QRegExp("[,/]"));
				for (int i = 0 ; i < keywords.count(); ++i)
				{
					this->_db->addKeyword(this->_currentIndex, keywords[i].trimmed());
				}
			}
			else
			{
				this->_db->addKeyword(this->_currentIndex, text.trimmed());
			}
		}
		else if (element == "date" && parent == "part")
		{
			this->_db->setYear(this->_currentIndex, text);
		}
		else if (element == "number" && parent == "detail" && grandParent == "part")
		{
			QXmlAttributes atts = this->stateAtts(-1);
			if (atts.value("type") == "issue")
			{
				this->_db->setIssue(this->_currentIndex, text);
			}
			else if (atts.value("type") == "volume")
			{
				this->_db->setVolume(this->_currentIndex, text);
			}
		}
		else if (element == "start" && parent == "extent" && grandParent == "part")
		{
			this->_db->setStartPage(this->_currentIndex, text);
		}
		else if (element == "end" && parent == "extent" && grandParent == "part")
		{
			this->_db->setEndPage(this->_currentIndex, text);
		}
		else if (element == "identifier" && parent == "mods")
		{
			QXmlAttributes atts = this->stateAtts();

			if (atts.value("type") == "doi")
			{
				this->_currentIdentifiers.insert("doi", text);
			}
			else if (atts.value("type") == "accessnum")
			{
				if (text.startsWith("ISI:"))
				{
					text.remove("ISI:");

					// I think ISI identifiers can only be numbers and letters....
					if (text.contains(QRegExp("[^0-9A-Za-Z]")))
					{
						qDebug() << "WARNING : Failed to convert an apparent ISI number, text was " << text;
					}
					else
					{
						this->_currentIdentifiers.insert("isi", text);
					}
				}
				else
				{
					if (text.contains(QRegExp("[^0-9]")))
					{
						// if any characters other than 0-9 then it can't be a valid
						// pmid
					}
					else
					{
						this->_currentIdentifiers.insert("pmid", text);
					}
				}
			}
			else if (atts.value("type") == "citekey")
			{
				this->_currentIdentifiers.insert("citekey", text);
			}
			else if (atts.value("type") == "issn")
			{
				this->_currentIdentifiers.insert("issn", text);
			}
			else if (atts.value("type") == "isbn")
			{
				this->_currentIdentifiers.insert("isbn", text);
			}
			else
			{
				qDebug() << "WARNING : unknown identifier type :" << atts.value("type");
			}
		}
	}

	return true;


}
