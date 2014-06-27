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

#include <athenaeum/articledelegate.h>
#include <athenaeum/librarymodel.h>

#include <QApplication>
#include <QColor>
#include <QFontMetrics>
#include <QPainter>
#include <QPixmap>
#include <QTextLayout>
#include <QUrl>

#include <QDebug>

#define ITEM_MARGIN 4
#define ITEM_SPACING 10

namespace Athenaeum
{

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // ArticleDelegatePrivate /////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

    class ArticleDelegatePrivate
    {
    public:
        //QPixmap pm;
        QPixmap icon;
        QPixmap pdfOverlay;

        int flaggedRow;
        int mouseRow;
    };




    ///////////////////////////////////////////////////////////////////////////////////////////////
    // ArticleDelegate ////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ArticleDelegate::ArticleDelegate()
        : d(new ArticleDelegatePrivate)
    {
        d->flaggedRow = -1;

        d->icon = QPixmap(":/icons/article-icon-blank-34x48.png");
        d->pdfOverlay = QPixmap(":/icons/article-icon-pdf-overlay-34x48.png");
    }

    ArticleDelegate::~ArticleDelegate()
    {}

    void ArticleDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
    {
        // Standard sanity-checking
		if (index.isValid() && painter && painter->isActive()) {
		    // Collect option information
            const QStyleOptionViewItemV3 * optionV3 = qstyleoption_cast< const QStyleOptionViewItemV3 * >(&option);
            const QWidget * widget = optionV3 ? optionV3->widget : 0;
            QStyle * style = widget ? widget->style() : QApplication::style();
			QStyleOptionViewItemV4 opt(option);
            initStyleOption(&opt, index);

            // Draw standard background
            style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, widget);

            // Get the dimensions available for each element
            QRect contentsRect(option.rect.adjusted(ITEM_MARGIN, ITEM_MARGIN, -ITEM_MARGIN, -ITEM_MARGIN));
            QRect iconRect(contentsRect.left(), contentsRect.top(), d->icon.width(), contentsRect.height());
            QPoint iconCenter(iconRect.center());
            iconRect.setSize(d->icon.size());
            iconRect.moveCenter(iconCenter);
            QRect textRect(contentsRect);
            textRect.setLeft(iconRect.right() + ITEM_SPACING);
            textRect.setHeight(option.fontMetrics.height() * 3);
            textRect.translate(0, (contentsRect.height() - textRect.height()) / 2);

            // Delegate painting code
            painter->save();
            painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);

            // Check for the state of the item to be rendered
            qRegisterMetaType< Athenaeum::AbstractBibliographicCollection::ItemState >();
            AbstractBibliographicCollection::ItemFlags flags = index.data(AbstractBibliographicCollection::ItemFlagsRole).value< Athenaeum::AbstractBibliographicCollection::ItemFlags >();
            QString title = index.data(AbstractBibliographicCollection::TitleRole).toString();
            QString subTitle = index.data(AbstractBibliographicCollection::SubtitleRole).toString();
            if (subTitle.length() > 0) {
                title += " : " + subTitle;
            }
            if (title.right(1) != ".") {
                title += ".";
            }
            if (title.length() == 1) {
                title = "Unknown";
            }

            QFont titleFont(option.font);
            painter->setFont(titleFont);
            QFontMetrics titleFontMetrics(painter->font());
            painter->setBrush(Qt::NoBrush);
            painter->setPen(QColor(255, 255, 255, (option.state & QStyle::State_Selected) ? 255 : 220));

            int textCursor = textRect.top();
            QTextLayout titleLayout(title, titleFont);
            titleLayout.beginLayout();
            QTextLine titleLine = titleLayout.createLine();
            if (titleLine.isValid()) {
                titleLine.setLineWidth(textRect.width());
                titleLine.draw(painter, textRect.topLeft());
                textCursor += titleFontMetrics.lineSpacing();
                titleLine = titleLayout.createLine();
                if (titleLine.isValid()) {
                    QString lastTitleLine = title.mid(titleLine.textStart());
                    QString elidedLastTitleLine = titleFontMetrics.elidedText(lastTitleLine, Qt::ElideRight, textRect.width());
                    painter->drawText(QPoint(textRect.left(), textCursor + titleFontMetrics.ascent()), elidedLastTitleLine);
                    textCursor += titleFontMetrics.lineSpacing();
                }
            }
            titleLayout.endLayout();

            QRect resultingAuthorRect;
            QFont authorFont(option.font);
            authorFont.setItalic(true);
            painter->setFont(authorFont);

            // Author gets the remaining space, minus 20 pixels for the 'info' icon at the bottom right
            QRect authorRect(QPoint(textRect.left(), textCursor), contentsRect.bottomRight());
            //painter->drawRect(authorRect);
            QStringList authors(index.data(AbstractBibliographicModel::AuthorsRole).toStringList());
            QString authorString;
            int removeAuthor = 0;
            QRect authorRequiredRect;

            do {
                if (removeAuthor > authors.count()) {
                    qDebug() << "Title is " << title;
                    qDebug() << "Author list shorter than expected" << removeAuthor << authors.count();
                    break;
                }

                QStringList authorStrings;
                int index = 0;
                foreach (const QString & author, authors) {
                    if (index >= authors.size() - removeAuthor) {
                        break;
                    }
                    ++index;

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
                authorString = QString();
                if (!authorStrings.isEmpty()) {
                    if (removeAuthor > 0) {
                        authorString = authorStrings.join(", ") + ", et al.";
                    } else {
                        if (authorStrings.size() == 1) {
                            authorString = authorStrings.at(0) + ".";
                        } else {
                            if (authorStrings.size() > 2) {
                                authorString = QStringList(authorStrings.mid(0, authorStrings.size() - 2)).join(", ") + ", ";
                            }
                            authorString += authorStrings.at(authorStrings.size() - 2) + " and " + authorStrings.at(authorStrings.size() - 1);
                        }
                    }
                }

                ++removeAuthor;

                authorRequiredRect = QFontMetrics(authorFont).boundingRect(authorRect, Qt::AlignLeft | Qt::TextWordWrap, authorString);

            } while (authorRequiredRect.height() >= authorRect.height());

            painter->setPen(QColor(255, 255, 255, (option.state & QStyle::State_Selected) ? 135 : 100));
            painter->drawText(authorRect, Qt::AlignLeft | Qt::TextWordWrap, authorString);

            // Covers
            painter->save();
            AbstractBibliographicCollection::ItemState state = index.data(AbstractBibliographicCollection::ItemStateRole).value< Athenaeum::AbstractBibliographicCollection::ItemState >();
            if (state == AbstractBibliographicCollection::BusyItemState) {
                painter->setOpacity(0.5);
            }
            QString publicationTitle(index.data(AbstractBibliographicCollection::PublicationTitleRole).toString());
            painter->drawPixmap(iconRect.topLeft(), d->icon);
            // Draw PDF overlay if there is a file
            if (index.data(AbstractBibliographicCollection::PdfRole).toUrl().isValid()) {
                painter->drawPixmap(iconRect.topLeft(), d->pdfOverlay);
            }
            painter->restore();

            // Light up the drop area for the PDF
            if (index.row() == d->flaggedRow) {
                painter->setBackgroundMode(Qt::TransparentMode);
                QPen pen(QColor(64, 93, 141));
                painter->setPen(pen);
                painter->setBrush(Qt::NoBrush);
                painter->drawRect(QRect(option.rect.topLeft() + QPoint(0, 1), QSize(option.rect.width() - 2, option.rect.height() - 2)));
            }

            painter->restore();
		}
    }

    void ArticleDelegate::setFlagged(int row)
    {
        d->flaggedRow = row;
    }

    void ArticleDelegate::setMouseEnteredRow(int row)
    {
        d->mouseRow = row;
    }

    int ArticleDelegate::mouseEnteredRow() const
    {
        return d->mouseRow;
    }


    QSize ArticleDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex &) const
    {
        // Width is irrelevant here, just return a sensible height big enough for title + author and icon
        return QSize(0, qMax(d->icon.height(), option.fontMetrics.height() * 3 + option.fontMetrics.leading() * 3) + 2 * ITEM_MARGIN);
    }

}
