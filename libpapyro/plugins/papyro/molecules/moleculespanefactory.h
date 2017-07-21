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

#ifndef MOLECULESPANEFACTORY_H
#define MOLECULESPANEFACTORY_H

#include <papyro/embeddedpanefactory.h>
#include <papyro/embeddedpane.h>
#include <papyro/utils.h>
#include <utopia2/networkaccessmanager.h>
#include <utopia2/fileformat.h>
#include <utopia2/qt/webview.h>
#include <string>

#include <cmath>

#include <QNetworkReply>
#include <QObject>
#include <QPainter>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>
#include <QWebFrame>

class MoleculesPane : public Papyro::EmbeddedPane
{
    Q_OBJECT

public:
    MoleculesPane(QString code, QWidget * parent = 0)
        : Papyro::EmbeddedPane(Papyro::EmbeddedPane::DefaultFlags, parent), _webView(0)
    {
        init();
        QVariantMap map;
        map["id"] = code;
        setData(map);
    }

    MoleculesPane(QByteArray bytes, QWidget * parent = 0)
        : Papyro::EmbeddedPane(Papyro::EmbeddedPane::DefaultFlags, parent), _webView(0)
    {
        init();
        QVariantMap map;
        map["bytes"] = bytes;
        setData(map);
    }

    MoleculesPane(QUrl url, QWidget * parent = 0)
        : Papyro::EmbeddedPane(Papyro::EmbeddedPane::DefaultFlags, parent), _webView(0)
    {
        init();
        QVariantMap map;
        map["url"] = url;
        setData(map);
    }

    void init()
    {
        _layout = new QVBoxLayout(this);
        _layout->setContentsMargins(0, 0, 0, 0);
        _layout->setSpacing(0);

        // Widget stuff
        setMouseTracking(true);
        setFixedSize(400, 400);
    }

    virtual ~MoleculesPane()
    {
        delete _webView;
    }

public slots:
    void molify()
    {
        QString command;
        QVariantMap map = data().toMap();
        if (!map.value("bytes").isNull()) {
            command = QString("jQuery(function () { molify('#molecule-viewer', {'bytes': '%1'}); });").arg(map.value("bytes").toString().replace("\\", "\\\\").replace("'", "\'"));
        } else if (!map.value("url").isNull()) {
            command = QString("jQuery(function () { molify('#molecule-viewer', {'url': '%1'}); });").arg(map.value("url").toString().replace("\\", "\\\\").replace("'", "\'"));
        } else if (!map.value("id").isNull()) {
            command = QString("jQuery(function () { molify('#molecule-viewer', {'id': 'pdb:%1'}); });").arg(map.value("id").toString().replace("\\", "\\\\").replace("'", "\'"));
        }
        _webView->page()->mainFrame()->evaluateJavaScript(command);
    }

protected:
    void download()
    {
        // Skip this code, as it's duplicated inside the Javascript
#if 0
        QVariantMap conf(data().toMap());
        // Don't bother with the download step if we already have the data
        if (!conf.contains("bytes")) {
            // Otherwise construct a URL...
            QUrl url;
            if (conf.contains("url")) {
                url = conf.value("url").toUrl();
            } else if (conf.contains("id")) {
                QString urlStr = "http://www.rcsb.org/pdb/download/downloadFile.do?fileFormat=pdb&compression=NO&structureId=";
                urlStr += conf.value("id").toString();
                url = QUrl::fromEncoded(urlStr.toUtf8());
            }
            // ... and download it
            if (url.isValid()) {
                startDownload(url);
            }
        }
#endif

        skipDownload();
        update();
    }

    void load()
    {
        if (_webView == 0) {
            _webView = new Utopia::WebView();

            _webView->setRenderHint(QPainter::Antialiasing, true);
            _webView->setRenderHint(QPainter::TextAntialiasing, true);
            _webView->setRenderHint(QPainter::SmoothPixmapTransform, true);

            _webView->page()->mainFrame()->addToJavaScriptWindowObject("control", this);

            _webView->load(QUrl("qrc:/papyro/molecules.html"));

            _webView->page()->setLinkDelegationPolicy(QWebPage::DelegateExternalLinks);

            _layout->addWidget(_webView);

            _webView->show();

            update();
        }
    }

    QVariant parseDownload(QNetworkReply * reply)
    {
        QVariantMap map(data().toMap());
        map["bytes"] = reply->readAll();
        return map;
    }

private:
    QVBoxLayout * _layout;
    Utopia::WebView * _webView;
};

class MoleculesPaneFactory : public Papyro::EmbeddedPaneFactory
{

public:
    QSet< Utopia::FileFormat * > formats;

    // Constructor
    MoleculesPaneFactory()
        : Papyro::EmbeddedPaneFactory()
    {
        formats = Utopia::FileFormat::get(Utopia::StructureFormat);
    }

    // Destructor
    virtual ~MoleculesPaneFactory()
    {}

    virtual QWidget * create(Spine::AnnotationHandle annotation, QWidget * parent = 0)
    {
        MoleculesPane * pane = 0;
        std::string concept = annotation->getFirstProperty("concept");
        std::string identifier = annotation->getFirstProperty("property:identifier");
        std::vector< std::string > media = annotation->getProperty("session:media");
        if (concept == "DatabaseEntry" && identifier.substr(0, 23) == "http://bio2rdf.org/pdb:")
        {
            std::string code(identifier.substr(23));
            pane = new MoleculesPane(Papyro::qStringFromUnicode(code), parent);
        }
        else if (concept == "DataLink" && media.size() > 0)
        {
            QString mime_type, url, name;
            foreach(std::string media_link, media)
            {
                QString encoded(Papyro::qStringFromUnicode(media_link));
                QStringList pairs(encoded.split("&"));
                foreach(QString pair, pairs)
                {
                    QString key(QUrl::fromPercentEncoding(pair.section('=', 0, 0).toUtf8()));
                    QString value(QUrl::fromPercentEncoding(pair.section('=', 1, 1).toUtf8()));
                    if (key == "type") mime_type = value;
                    else if (key == "src") url = value;
                    else if (key == "name") name = value;
                }
            }
            if (!url.isEmpty())
            {
                if (mime_type == "chemical/x-pdb")
                {
                    pane = new MoleculesPane(QUrl(url), parent);
                }
                else if (!name.isEmpty())
                {
                    foreach (Utopia::FileFormat * format, formats)
                    {
                        if (format->contains(name.section(".", -1)))
                        {
                            pane = new MoleculesPane(QUrl(url), parent);
                            break;
                        }
                    }
                }
            }
        }

        return pane;
    }

    virtual QString title()
    {
        return "3D Molecule";
    }

}; // class MoleculesPaneFactory

#endif // MOLECULESPANEFACTORY_H
