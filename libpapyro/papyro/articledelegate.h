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

#ifndef ATHENAEUM_ARTICLEDELEGATE_H
#define ATHENAEUM_ARTICLEDELEGATE_H

#include <papyro/config.h>
#include <papyro/citation.h>

#include <QStyledItemDelegate>

namespace Athenaeum
{

    class ArticleDelegatePrivate;
    class ArticleDelegate : public QStyledItemDelegate
    {
        Q_OBJECT

    public:
        ArticleDelegate(QObject * parent);
        ~ArticleDelegate();

        void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex & index) const;

        void setMouseEnteredRow(int row);
        void setFlagged(int row);
        int mouseEnteredRow() const;

    signals:
        void updateRequested();

    protected:
        bool editorEvent(QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index);

        void getRects(const QStyleOptionViewItem &option,
                      QRect * controlRect,
                      QRect * imageRect,
                      QRect * infoRect) const;

    private:
        boost::scoped_ptr< ArticleDelegatePrivate > d;
    };

}

#endif // ATHENAEUM_ARTICLEDELEGATE_H
