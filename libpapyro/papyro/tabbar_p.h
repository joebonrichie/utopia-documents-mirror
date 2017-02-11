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

#ifndef PAPYRO_TABBAR_P_H
#define PAPYRO_TABBAR_P_H

#include <papyro/papyrotab.h>
#include <papyro/citation.h>

#include <QIcon>
#include <QObject>
#include <QPixmap>
#include <QMap>
#include <QSignalMapper>
#include <QTime>
#include <QTimer>
#include <QUrl>

namespace Papyro
{

    typedef struct TabData
    {
        PapyroTab * tab;
        Athenaeum::CitationHandle citation;
        QString title;
        int size;
        int offset;
        bool busy;
        bool error;
        QTime time;
        qreal progress;
        bool starred;
        bool known;
    } TabData; // struct TabData




    class TabBar;
    class TabBarPrivate : public QObject
    {
        Q_OBJECT

    public:
        TabBarPrivate(TabBar * tabBar);
        ~TabBarPrivate();

        TabBar * tabBar;

        // Interaction state
        int currentIndex;
        QList< TabData > tabs;
        QSignalMapper citationMapper;

        // Render options
        QSize tabCurveSize;
        QIcon closeButtonIcon;
        QIcon starButtonIcon;
        int tabEdgeSize;
        int minTabSize;
        int maxTabSize;
        int tabSpacing;
        int tabPadding;
        int tabFading;
        int tabMargin;
        int spinnerSize;

        int actionLeft() const;
        int tabLeft() const;

        // Render state
        int extent; // total height of all tabs
        int position; // current scroll position
        QTimer animationTimer;

        // Mouse state
        QPoint hoverPos;
        int tabCloseButtonPressed;
        int tabCloseButtonUnderMouse;
        int tabStarButtonPressed;
        int tabStarButtonUnderMouse;
        struct {
            struct {
                int tab;
                int section;
            } move;
            struct {
                int section;
            } press;
        } mouse;
        QTimer wheelDelay;

        // Hi DPI scaling
        qreal dpiScaling;

        int getCurrentIndex() const;
        int getPosition() const;
        int getTabOffset(int index) const;
        QRect getTabCloseButtonRect(int index) const;
        QRect getTabStarButtonRect(int index) const;
        QRect getTabRect(int index) const;
        QRect getTabRect(const TabData * data) const;
        int getTabSize(int index) const;
        void paintTab(QPainter * painter, int index);
        int tabAt(const QPoint & pos) const;
        TabData * tabData(int index);
        const TabData * tabData(int index) const;
        const TabData * tabData(QObject * tab) const;
        const TabData * tabDataAt(const QPoint & pos) const;
        void toggleAnimationTimer();
        void updateGeometries();
        void updateState(TabData * data);

    signals:
        void closeRequested(int index);

    public slots:
        void tabCloseRequested();
        void tabDestroyed(QObject * obj);
        void tabProgressChanged(qreal progress);
        void tabStateChanged(PapyroTab::State progress);
        void tabTitleChanged(const QString & title);
        void tabUrlChanged(const QUrl & url);
        void onCitationChanged(QObject * tab);
        void onTabCitationChanged();
        void updateHoverPos();
    };

}

#endif // PAPYRO_TABBAR_P_H
