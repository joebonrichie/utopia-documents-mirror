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

#ifndef QAMBROSIAWIDGET_H
#define QAMBROSIAWIDGET_H

//#include <xne/xne.h>
#include <gtl/vertexbuffer.h>
#include <ambrosia/config.h>
#include <qglviewer.h>
#include <utopia2/qt/abstractwidget.h>
#include <utopia2/utopia2.h>
#include <ambrosia/ambrosia.h>

#include <QMenu>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QList>
#include <QString>

/*
  #include <huds/toolbar.h>
  #include <huds/orientation.h>
  #include <huds/annotations.h>
  #include <huds/timeline.h>
*/
#include <map>

namespace AMBROSIA
{
    class Renderable;


    class AmbrosiaWidget : public QGLViewer, public Utopia::AbstractWidgetInterface
    {
        Q_OBJECT

    public:
        AmbrosiaWidget(QWidget * = 0, Qt::WFlags = 0);
        AmbrosiaWidget(AMBROSIA::Ambrosia *, QWidget * = 0, Qt::WFlags = 0);
        void construct();
        virtual ~AmbrosiaWidget();

    public slots:
        // AbstractWidget Interface
        virtual bool load(Utopia::Node * model);
        virtual bool supports(Utopia::Node * model) const;

    public:
        // General Management
        bool load(QString);
        Utopia::Node * getComplex();
        void clear();
        void processEvents();

        // Visualisation state
        bool viewingAnnotationOnComplex(Utopia::Node *, QString, QString);

        // Ambrosia Rendering Options
        void setDisplay(bool, AMBROSIA::Ambrosia::RenderSelection = AMBROSIA::Ambrosia::ALL, AMBROSIA::Selection * = 0);
        void setVisible(bool, AMBROSIA::Ambrosia::RenderSelection = AMBROSIA::Ambrosia::ALL, AMBROSIA::Selection * = 0);
        void setRenderFormat(unsigned int, AMBROSIA::Ambrosia::RenderSelection = AMBROSIA::Ambrosia::ALL, AMBROSIA::Selection * = 0);
        void setRenderOption(unsigned int, bool, AMBROSIA::Ambrosia::RenderSelection = AMBROSIA::Ambrosia::ALL, AMBROSIA::Selection * = 0);
        void setColour(AMBROSIA::Colour *, AMBROSIA::Ambrosia::RenderSelection = AMBROSIA::Ambrosia::ALL, AMBROSIA::Selection * = 0);
        void setAlpha(GLubyte, AMBROSIA::Ambrosia::RenderSelection = AMBROSIA::Ambrosia::ALL, AMBROSIA::Selection * = 0);
        void setTintColour(AMBROSIA::Colour *, AMBROSIA::Ambrosia::RenderSelection = AMBROSIA::Ambrosia::ALL, AMBROSIA::Selection * = 0);
        void setHighlightColour(AMBROSIA::Colour *, AMBROSIA::Ambrosia::RenderSelection = AMBROSIA::Ambrosia::ALL, AMBROSIA::Selection * = 0);
        void setRenderTag(AMBROSIA::Ambrosia::RenderTag, AMBROSIA::Ambrosia::RenderSelection = AMBROSIA::Ambrosia::ALL, AMBROSIA::Selection * = 0);
        bool isEnabled(AMBROSIA::Ambrosia::RenderOption option);
        void enable(AMBROSIA::Ambrosia::RenderOption);
        void disable(AMBROSIA::Ambrosia::RenderOption);
        void enable(AMBROSIA::Ambrosia::RenderOption, bool);
        void setBackgroundColour(AMBROSIA::Colour * = 0);

    protected:
        virtual void mousePressEvent(QMouseEvent *);
        virtual void mouseMoveEvent(QMouseEvent *);
        virtual void mouseReleaseEvent(QMouseEvent *);
        virtual void mouseDoubleClickEvent(QMouseEvent *);
        virtual void closeEvent(QCloseEvent*);

        // State for popup creation
        bool popupClicked;
        bool popupMoved;
        int popupX;
        int popupY;
        QMenu * contextMenu;
        QMenu * contextMenuDisplay;
        QAction * displaySpacefillAction;
        QAction * displayBackboneAction;
        QAction * displayCartoonAction;
        QAction * displayEncapsulatedBackboneAction;
        QMenu * contextMenuOptions;
        QAction * optionsSmoothBackbonesAction;
        QAction * optionsChunkyBackbonesAction;
        QAction * optionsShowSidechainsAction;

        virtual QString getAnnotationPath(Utopia::Node *);

    public slots:
        // General slots
        virtual void windowActivationChange(bool oldState);
        virtual void showComplex(Utopia::Node *);
        virtual void hideComplex(Utopia::Node *);

        // Annotation slots
        virtual void applyHighlights();
        virtual void newHighlight(unsigned int, unsigned int, unsigned int);
        virtual void modifyHighlight(unsigned int, unsigned int, unsigned int);
        virtual void removeHighlight(unsigned int);
        virtual void showExtentAnnotation(QString);
        virtual void showBooleanAnnotation(QString);
        virtual void showValueAnnotation(QString, QString);
        virtual void showAnnotations(Utopia::Node *, QList<QString>);
        virtual void hideAnnotations(Utopia::Node *, QList<QString>);
        virtual void hideAnnotations();

        // Context menu slots
        virtual void popup();
        virtual void contextExportComplex();
        virtual void contextSaveSnapshot();
        virtual void contextRequestClone();
        virtual void contextClose();
        virtual void contextDisplaySpacefill();
        virtual void contextDisplayBackbone();
        virtual void contextDisplayCartoon();
        virtual void contextDisplayEncapsulatedBackbone();
        virtual void contextOptionsSmoothBackbones();
        virtual void contextOptionsChunkyBackbones();
        virtual void contextOptionsShowSidechains();

    signals:
        void annotationsShown(Utopia::Node *, QList<QString>);
        void annotationsHidden(Utopia::Node *, QList<QString>);
        void modelChanged(AmbrosiaWidget *);
        void cloneRequested(AmbrosiaWidget *, AMBROSIA::Ambrosia *);
        void focusReceived(AmbrosiaWidget *);
        void focusLost(AmbrosiaWidget *);
        void deleted(QWidget *);
        void closed(QWidget *);

    private:
        // QGLViewer methods
        virtual void init();
        virtual void drawWithNames();
        virtual void draw();
        virtual void postSelection(const QPoint& point);

        // Connection information
//                      Endpoint connection_out;
//                      Endpoint connection_in;

        // Internal ambrosia object
        AMBROSIA::Ambrosia * ambrosia;
        QList<QString> visualisedAnnotations;
        bool qglviewerDone;

        std::set< AMBROSIA::Renderable* > highlighted;
        /*
          AMBROSIA::ToolbarHUD* _toolbar;
          AMBROSIA::OrientationHUD* _orientation;
          AMBROSIA::AnnotationHUD* _annotation;
          AMBROSIA::TimelineHUD* _timeline;
        */
        // Highlights
        std::map< unsigned int, std::pair< unsigned int, unsigned int > > _highlights;

    };

}
#endif // QAMBROSIAWIDGET_H
