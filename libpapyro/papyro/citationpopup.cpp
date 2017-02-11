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

#include <papyro/citationpopup_p.h>
#include <papyro/citationpopup.h>
#include <papyro/citationfinder.h>
#include <papyro/citations.h>
#include <papyro/cslengine.h>
#include <papyro/cslengineadapter.h>
#include <papyro/utils.h>
#include <papyro/resolverrunnable.h>

#include <QHBoxLayout>
#include <QJsonDocument>
#include <QLabel>
#include <QList>
#include <QVBoxLayout>
#include <QStackedLayout>

namespace Papyro
{

    CitationPanel::CitationPanel(Spine::AnnotationHandle citation)
        : QWidget(0), citation(citation), completed(citation->capabilities< CitationFinderCapability >().size())
    {
        // Get a pointer to the CSL engine for formatting
        boost::shared_ptr< CSLEngine > cslengine(CSLEngine::instance());

        // Create layout for this widget
        _layout = new QHBoxLayout(this);
        _layout->setContentsMargins(4, 4, 4, 4);
        _layout->setSpacing(6);

        // Compile a citation map from this annotation
        QVariantMap metadata = citationToMap(citation);

        // Turn the citation into a formatted string
        QString formatted;
        if (metadata.contains("authors") && metadata.contains("title") && metadata.contains("publication-title")) {
            formatted = cslengine->format(convert_to_cslengine(metadata));
        } else {
            formatted = qStringFromUnicode(citation->getFirstProperty("property:displayText"));
        }

        citationLabel = new QLabel(formatted);
        {
#ifndef Q_OS_WIN
            QFont f(citationLabel->font());
            f.setPointSizeF(f.pointSizeF() * 0.85);
            citationLabel->setFont(f);
#endif
        }
        citationLabel->setWordWrap(true);
        citationLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        citationLabel->setFixedWidth(280);
        citationLabel->adjustSize();
        _layout->addWidget(citationLabel, 1);

        pdfLabel = new QLabel;
        pdfLabel->setPixmap(QPixmap(":/icons/mime-pdf.png"));
        pdfLabel->setFixedWidth(32);
        pdfLabel->setCursor(Qt::PointingHandCursor);
        pdfLabel->setToolTip("View cited article");
        pdfLabel->installEventFilter(this);
        _layout->addWidget(pdfLabel, 1, Qt::AlignVCenter | Qt::AlignRight);
        pdfLabel->hide();

        QWidget * moreWidget = new QWidget;
        moreLayout = new QStackedLayout(moreWidget);
        _layout->addWidget(moreWidget, 1, Qt::AlignVCenter | Qt::AlignRight);

        spinner = new Utopia::Spinner;
        moreLayout->addWidget(spinner);
        spinner->setFixedWidth(32);
        spinner->start();

        moreLabel = new QLabel;
        moreLabel->setPixmap(QPixmap(":/icons/other-links.png"));
        moreLabel->setCursor(Qt::PointingHandCursor);
        moreLabel->setToolTip("Find cited article...");
        moreLabel->installEventFilter(this);
        moreLabel->setFixedWidth(32);
        moreLayout->addWidget(moreLabel);

        if (completed == 0) {
            moreLayout->setCurrentWidget(moreLabel);
            spinner->stop();
        }

        connect(&signalMapper, SIGNAL(mapped(const QString &)), this, SLOT(onLinkClicked(const QString &)));

        Athenaeum::ResolverRunnable::dereference(Athenaeum::Citation::fromMap(metadata), this, SLOT(onResolverRunnableCompleted(Athenaeum::CitationHandle)));
    }

    void CitationPanel::addLink(QString title, QString url)
    {
        if (!url.isEmpty()) {
            if (title.isEmpty()) { title = url; }
            QRegExp digits("\\d+");
            title.indexOf(digits);
            int order = digits.cap().toInt();
            title = title.mid(digits.matchedLength());
            QAction * action = new QAction(title, &menu);
            action->setProperty("order", order);
            connect(action, SIGNAL(triggered()), &signalMapper, SLOT(map()));
            signalMapper.setMapping(action, url);

            // Insert action in right place
            QAction * after = 0;
            foreach (QAction * action, menu.actions()) {
                if (action->property("order").toInt() < order) {
                    after = action;
                    break;
                }
            }
            menu.insertAction(after, action);
        }
    }

    void CitationPanel::addPdf(QString title, QString url)
    {
        if (!url.isEmpty()) {
            pdfLabel->show();
            pdfLabel->setProperty("url", url);
            pdfLabel->setToolTip(title);
        }
    }

    bool CitationPanel::eventFilter(QObject * obj, QEvent * event)
    {
        if (event->type() == QEvent::MouseButtonRelease) {
            if (obj == pdfLabel) {
                QUrl url = pdfLabel->property("url").toString();
                emit requestUrl(url, "tab");
                if ((QApplication::keyboardModifiers() & Qt::ControlModifier) == 0) {
                    window()->close();
                }
            } else if (obj == moreLabel) {
                QMouseEvent * me = static_cast< QMouseEvent * >(event);
                menu.exec(me->globalPos());
            }
        }
        return QWidget::eventFilter(obj, event);
    }

    void CitationPanel::onLinkClicked(const QString & url)
    {
        emit requestUrl(url, QString());
    }

    struct CompareLinks {
        // is lhs more important than rhs?
        bool operator () (const QVariantMap & lhs, const QVariantMap & rhs) {
            // Ordering of type strings
            static QStringList types;
            if (types.isEmpty()) {
                types << "search";
                types << "abstract";
                types << "article";
            }

            int lhs_type(types.indexOf(lhs.value("type").toString()));
            int rhs_type(types.indexOf(rhs.value("type").toString()));
            int lhs_weight(lhs.value(":weight").toInt());
            int rhs_weight(rhs.value(":weight").toInt());
            if (lhs_type == rhs_type) {
                return lhs_weight > rhs_weight;
            } else {
                return lhs_type > rhs_type;
            }
        }
    };

    void CitationPanel::onResolverRunnableCompleted(Athenaeum::CitationHandle citation)
    {
        QVariantMap metadata(citation->toMap());
        static CompareLinks compareLinks;
        QMap< QString, QVariantMap > links;
        QMap< QString, QVariantMap > pdfs;
        foreach (QVariant item, metadata.value("links").toList()) {
            // Should we add this link?
            bool add = false;
            // Get the link, and unpack some useful values
            QVariantMap link(item.toMap());
            QString whence(link.value(":whence").toString());
            //int weight(link.value(":weight").toInt());
            QString type(link.value("type").toString());
            QString mime(link.value("mime").toString());
            // If no exisiting link with the same whence, add this item
            if (mime == "text/html" && !links.contains(whence)) {
                add = true;
            } else {
                // Check existing values to make sure we add the correct links
                QVariantMap existing(links.value(whence));
                add = compareLinks(link, existing);
            }
            // Add if need be
            if (add) {
                links[whence] = link;
            }
        }
        QList< QVariantMap > ordered(links.values());
        qSort(ordered.begin(), ordered.end(), compareLinks);

        // Now add links to the panel
        bool pdf = false;
        foreach (QVariantMap link, ordered) {
            if (link["mime"] == "application/pdf") {
                if (!pdf) {
                    pdf = true;
                    addPdf(link["title"].toString(), link["url"].toString());
                }
            } else {
                addLink(link["title"].toString(), link["url"].toString());
            }
        }

        moreLayout->setCurrentWidget(moreLabel);
        spinner->stop();
    }





    CitationPopup::CitationPopup(QWidget * parent)
        : Utopia::BubbleWidget(parent, Qt::Popup), d(new CitationPopupPrivate(this))
    {
        //setFixedWidth(300);
        setPopup(true);

        d->layout = new QVBoxLayout(this);
        d->layout->setSpacing(8);
        d->layout->setContentsMargins(0, 0, 0, 0);
        d->layout->setSizeConstraint(QLayout::SetFixedSize);
        //d->layout->addStrut(300);
    }

    CitationPopup::~CitationPopup()
    {}

    void CitationPopup::addCitation(Spine::AnnotationHandle citation)
    {
        CitationPanel * panel = new CitationPanel(citation);
        connect(panel, SIGNAL(requestUrl(const QUrl &, const QString &)), this, SIGNAL(requestUrl(const QUrl &, const QString &)), Qt::QueuedConnection);

        std::string id_str(citation->getFirstProperty("property:label"));
        int id_num = 0;
        if (id_str.empty()) {
            id_str = citation->getFirstProperty("property:id");
        } else if (d->numerical) {
            bool ok;
            id_num = qStringFromUnicode(id_str).toInt(&ok);
            if (!ok) {
                d->numerical = false;
            }
        }

        //qDebug() << "---" << d->numerical << id_num << qStringFromUnicode(id_str);

        // Insert into the proper place
        int index;
        for (index = 0; index < d->layout->count(); ++index) {
            if (CitationPanel * other = qobject_cast< CitationPanel * >(d->layout->itemAt(index)->widget())) {
                std::string other_id_str = other->citation->getFirstProperty("property:label");
                int other_id_num = 0;
                if (other_id_str.empty()) {
                    other_id_str = other->citation->getFirstProperty("property:id");
                } else if (d->numerical) {
                    bool ok;
                    other_id_num = qStringFromUnicode(other_id_str).toInt(&ok);
                    if (!ok) {
                        d->numerical = false;
                    }
                }
                //qDebug() << "  -" << d->numerical << other_id_num << qStringFromUnicode(other_id_str);
                if (other_id_num > id_num || (!d->numerical && other_id_str > id_str)) {
                    break;
                }
            }
        }
        d->layout->insertWidget(index, panel);

        // Generate links
        foreach (boost::shared_ptr< CitationFinderCapability > finder, citation->capabilities< CitationFinderCapability >()) {
            //finder->generate(citation, panel, SLOT(addLink(QString, QString)));
        }
    }

}
