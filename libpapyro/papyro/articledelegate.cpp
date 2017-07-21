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

#include <papyro/articledelegate.h>
#include <papyro/librarymodel.h>

#include <utopia2/qt/hidpi.h>

#include <QApplication>
#include <QColor>
#include <QFontMetrics>
#include <QPainter>
#include <QPixmap>
#include <QTextLayout>
#include <QTime>
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
        QPixmap image;
        QPixmap pdfOverlay;
        QPixmap starredIcon;
        QPixmap unstarredIcon;

        QModelIndex hoverIndex;
        bool hoverStarred;
        bool pressStarred;

        int flaggedRow;
        int mouseRow;

        int assetScale;
    };




    ///////////////////////////////////////////////////////////////////////////////////////////////
    // ArticleDelegate ////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ArticleDelegate::ArticleDelegate(QObject * parent)
        : QStyledItemDelegate(parent), d(new ArticleDelegatePrivate)
    {
        d->flaggedRow = -1;
        d->assetScale = Utopia::isHiDPI() ? 1 : 2;

        d->image = QPixmap(":/icons/article-icon.png");
        d->icon = QPixmap(":/icons/article-icon.png");
        d->pdfOverlay = QPixmap(":/icons/article-icon-pdf-overlay.png");

        d->starredIcon = QPixmap(":/icons/article-starred.png");
        d->unstarredIcon = QPixmap(":/icons/article-unstarred.png");

        d->hoverStarred = false;
        d->pressStarred = false;
    }

    ArticleDelegate::~ArticleDelegate()
    {}

    bool ArticleDelegate::editorEvent(QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index)
    {
        bool updateRequired = false;

        // Capture move events and set hover info appropriately
        switch (event->type()) {
        case QEvent::MouseMove:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        {
            QMouseEvent * me = dynamic_cast< QMouseEvent * >(event);

            // Get the dimensions available for each element
            QRect controlRect, imageRect, infoRect;
            getRects(option, &controlRect, &imageRect, &infoRect);
            QRect starredIconRect(controlRect.topLeft(), QSize(controlRect.width(), controlRect.width()));

            switch (event->type()) {
            case QEvent::MouseMove:
            {
                // Save which index is under the mouse
                if (d->hoverIndex != index) {
                    d->hoverIndex = index;
                    updateRequired = true;
                }
                // Work out if it's in the controls
                bool hoverStarred = starredIconRect.contains(me->pos());
                if (d->hoverStarred != hoverStarred) {
                    d->hoverStarred = hoverStarred;
                    updateRequired = true;
                }
                break;
            }
            case QEvent::MouseButtonPress:
            {
                bool pressStarred = starredIconRect.contains(me->pos());
                if (d->pressStarred != pressStarred) {
                    d->pressStarred = pressStarred;
                }
                break;
            }
            case QEvent::MouseButtonRelease:
            {
                if (d->pressStarred) {
                    // Toggle the starredness of the citation
                    CitationHandle citation =
                        index.data(Citation::ItemRole).value< CitationHandle >();
                    Citation::Flags flags =
                        citation->field(Citation::FlagsRole).value< Citation::Flags >();
                    if (flags & Citation::StarredFlag) {
                        flags &= ~Citation::StarredFlag;
                    } else {
                        flags |= Citation::StarredFlag;
                    }
                    citation->setField(Citation::FlagsRole, QVariant::fromValue< Citation::Flags >(flags));
                    d->pressStarred = false;
                }
                break;
            }
            default:
                break;
            }

            if (updateRequired) {
                emit updateRequested();
            }
        }
        default:
            break;
        }

        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

    void ArticleDelegate::getRects(const QStyleOptionViewItem &option,
                                   QRect * controlRect,
                                   QRect * imageRect,
                                   QRect * infoRect) const
    {
        // Guard
        if (!controlRect || !imageRect || !infoRect) {
            return;
        }

        // General spacing
        static const int margin = 3;
        static const int spacing = 4;

        // Contents rect, adjusting for margin
        QRect contentsRect(option.rect.adjusted(margin, margin, -margin, -margin));

        // Control width comes from the icon size, height is 100%
        *controlRect = QRect(contentsRect.left(), contentsRect.top(),
                             d->starredIcon.width() / d->assetScale, contentsRect.height());
        controlRect->adjust(0, margin, 0, -margin);

        // Image comes next, and is as big as it needs to be, and no more
        *imageRect = QRect(controlRect->right() + spacing/2, contentsRect.top(),
                           d->image.width() / d->assetScale, contentsRect.height());
        // Resize to correct size
        QPoint imageCenter(imageRect->center());
        imageRect->setSize(d->image.size() / d->assetScale);
        imageRect->moveCenter(imageCenter);

        // Finally the info gets all the rest of the space, but vertically centred
        *infoRect = contentsRect.adjusted(imageRect->right() + spacing, 0, 0, 0);
        int textHeight = option.fontMetrics.height() * 3 - option.fontMetrics.leading() * 2;
        infoRect->adjust(0, (contentsRect.height() - textHeight) / 2,
                         0, 1-(contentsRect.height() - textHeight) / 2);

        // NOTE: this assumes only three lines of text. If more are displayed,
        // or at a larger size than expected, the text may run into the next
        // row.
    }

    void ArticleDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
    {
        // Standard sanity-checking
		if (index.isValid() && painter && painter->isActive()) {
		    // The two mains bits of information to be rendered
		    QString primaryInfo;
		    QString secondaryInfo;

		    // Get citation data
            qRegisterMetaType< AbstractBibliography::State >();
            QString title = index.data(Citation::TitleRole).toString();
            QString subTitle = index.data(Citation::SubTitleRole).toString();
            QUrl originatingUri = index.data(Citation::OriginatingUriRole).toUrl();
            Citation::Flags flags =
                index.data(Citation::FlagsRole).value< Citation::Flags >();
            AbstractBibliography::State state =
                index.data(Citation::StateRole).value< AbstractBibliography::State >();
            bool isStarred = flags & Citation::StarredFlag;
            //bool isKnown = index.data(Citation::KnownRole).toBool();
            bool isMouseOverStarredIcon = (index == d->hoverIndex && d->hoverStarred);

		    // Collect option information
            const QStyleOptionViewItemV3 * optionV3 = qstyleoption_cast< const QStyleOptionViewItemV3 * >(&option);
            const QWidget * widget = optionV3 ? optionV3->widget : 0;
            QStyle * style = widget ? widget->style() : QApplication::style();
			QStyleOptionViewItemV4 opt(option);
            initStyleOption(&opt, index);

            // Draw standard background
            style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, widget);

            // Get the dimensions available for each element
            QRect controlRect, imageRect, infoRect;
            getRects(option, &controlRect, &imageRect, &infoRect);

            // Delegate painting code
            painter->save();
            painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);

            // Start with the controls
            QRect starredIconRect(controlRect.topLeft(), QSize(controlRect.width(), controlRect.width()));
            painter->save();
            painter->setOpacity((isStarred ? 0.9 : 0.2) + (isMouseOverStarredIcon ? 0.1 : 0.0));
            painter->drawPixmap(starredIconRect, isStarred ? d->starredIcon : d->unstarredIcon);
            painter->restore();

            // Check for the state of the item to be rendered
            if (!title.isEmpty() && subTitle.length() > 0) {
                title += " : " + subTitle;
            }
            if (title.isEmpty()) {
                if (originatingUri.isValid()) {
                    if (originatingUri.isLocalFile()) {
                        QFileInfo originatingUriInfo(originatingUri.toLocalFile());
                        primaryInfo = originatingUriInfo.fileName();
                        secondaryInfo = originatingUriInfo.path();
                    } else {
                        primaryInfo = originatingUri.fileName();
                        secondaryInfo = originatingUri.scheme() + "://" + originatingUri.host();
                        if (originatingUri.port() >= 0) {
                            secondaryInfo = secondaryInfo + QString(":%1").arg(originatingUri.port());
                        }
                    }
                } else {
                    title = "Unknown";
                }
            } else if (title.right(1) != ".") {
                title += ".";
            }

            if (!title.isEmpty()) {
                primaryInfo = title;
            }

            QFont titleFont(option.font);
            painter->save();
            painter->setFont(titleFont);
            QFontMetrics titleFontMetrics(painter->font());
            painter->setOpacity((option.state & QStyle::State_Selected) ? 1.0 : 0.85);

            int textCursor = infoRect.top();
            QTextLayout titleLayout(primaryInfo, titleFont);
            titleLayout.beginLayout();
            QTextLine titleLine = titleLayout.createLine();
            if (titleLine.isValid()) {
                titleLine.setLineWidth(infoRect.width());
                QString firstTitleLine(primaryInfo.mid(titleLine.textStart(), titleLine.textLength()));
                painter->drawText(QPoint(infoRect.left(), textCursor + titleFontMetrics.ascent()), firstTitleLine);
                //titleLine.draw(painter, infoRect.topLeft());
                textCursor += titleFontMetrics.lineSpacing();
                titleLine = titleLayout.createLine();
                if (titleLine.isValid()) {
                    QString lastTitleLine = primaryInfo.mid(titleLine.textStart());
                    QString elidedLastTitleLine = titleFontMetrics.elidedText(lastTitleLine, Qt::ElideRight, infoRect.width());
                    painter->drawText(QPoint(infoRect.left(), textCursor + titleFontMetrics.ascent()), elidedLastTitleLine);
                    textCursor += titleFontMetrics.lineSpacing();
                }
            }
            //textCursor += 1;
            titleLayout.endLayout();

            QFont authorFont(option.font);
            authorFont.setItalic(true);
            painter->setFont(authorFont);

            // Author gets the remaining space
            QRect authorRect(QPoint(infoRect.left(), textCursor + titleFontMetrics.leading()), infoRect.bottomRight());
            //painter->drawRect(authorRect);
            QStringList authors(index.data(Citation::AuthorsRole).toStringList());
            QString authorString;
            int removeAuthor = 0;
            QRect authorRequiredRect;

            do {
                if (removeAuthor > authors.count()) {
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

            if (!authorString.isEmpty()) {
                secondaryInfo = authorString;
            }

            painter->setOpacity(painter->opacity() / 2.0);
            painter->drawText(authorRect, Qt::AlignLeft | Qt::TextWordWrap, secondaryInfo);
            painter->restore();

            // Covers
            painter->save();
            if (state == AbstractBibliography::BusyState) {
                painter->setOpacity(0.5);
            }
            QString publicationTitle(index.data(Citation::PublicationTitleRole).toString());
            painter->drawPixmap(imageRect, d->icon);
            // Draw PDF overlay if there is a file
            if (index.data(Citation::ObjectFileRole).toUrl().isValid()) {
                painter->drawPixmap(imageRect, d->pdfOverlay);
            }
            painter->restore();

            // If busy, paint spinner
            if (state == AbstractBibliography::BusyState) {
                QRect spinnerRect(QPoint(0, 0), imageRect.size() / 2);
                spinnerRect.setWidth(qMin(spinnerRect.width(), spinnerRect.height()));
                spinnerRect.setHeight(spinnerRect.width());
                spinnerRect.moveCenter(imageRect.center());
                int sqr = qMin(spinnerRect.width(), spinnerRect.height());
                qreal penWidth = (qreal) qRound(sqr * 3.0 / 16.0);
                int startAngle = 360*16*((QTime::currentTime().msecsSinceStartOfDay() % 1000) / 1000.0);
                int spanAngle = 120*16*2;
                QPen pen(painter->pen());
                pen.setWidth(penWidth);
                painter->setPen(pen);
                painter->drawArc(spinnerRect.adjusted(penWidth/2.0, penWidth/2.0, -penWidth/2.0, -penWidth/2.0), -startAngle, spanAngle);
            }

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
        return QSize(0, qMax(d->icon.height() / d->assetScale, option.fontMetrics.lineSpacing() * 3) + 2 * ITEM_MARGIN + 1);
    }

}
