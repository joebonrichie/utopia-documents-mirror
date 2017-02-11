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

#ifndef PAPYRO_TABBAR_H
#define PAPYRO_TABBAR_H

#include <QFrame>

namespace Papyro
{

    class PapyroTab;

    class TabBarPrivate;
    class TabBar : public QFrame
    {
        Q_OBJECT

    public:
        TabBar(QWidget * parent = 0, Qt::WindowFlags f = 0);
        ~TabBar();

        int addTab(PapyroTab * tab);
        int count() const;
        int currentIndex() const;
        int indexAt(const QPoint & pos) const;
        int indexOf(PapyroTab * tab) const;
        bool isEmpty() const;
        void removeTab(int index);
        PapyroTab * tabAt(int index) const;

    public slots:
        void nextTab();
        void previousTab();
        void setCurrentIndex(int index);

    signals:
        void closeRequested(int index);
        void currentIndexChanged(int index);
        void layoutChanged();
        void starredToggled(bool starred);
        void tabAdded(int index);
        void tabMetadataChanged(int index);
        void tabRemoved(int index);
        void tabAdded(PapyroTab * tab);
        void tabRemoved(PapyroTab * tab);

    protected:
        void enterEvent(QEvent * event);
        bool event(QEvent * event);
        void leaveEvent(QEvent * event);
        void mousePressEvent(QMouseEvent * event);
        void mouseMoveEvent(QMouseEvent * event);
        void mouseReleaseEvent(QMouseEvent * event);
        void paintEvent(QPaintEvent * event);
        void wheelEvent(QWheelEvent * event);

    private:
        TabBarPrivate * d;
    };

}

#endif // PAPYRO_TABBAR_H
