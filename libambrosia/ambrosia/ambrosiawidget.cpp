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

#include <ambrosia/renderable.h>
#include <ambrosia/ambrosiawidget.h>
#include <qglviewer.h>
#include <string>
#include <math.h>

#include <QMessageBox>
#include <QLayout>
#include <QPixmap>
#include <QMouseEvent>
#include <QCloseEvent>

namespace AMBROSIA
{

    AmbrosiaWidget::AmbrosiaWidget(QWidget * parent, Qt::WFlags flags)
        : QGLViewer(parent, 0, flags), ambrosia(0), qglviewerDone(false)
    {
        qDebug() << "AmbrosiaWidget::AmbrosiaWidget" << this;
        construct();
    }

    AmbrosiaWidget::AmbrosiaWidget(Ambrosia * ambrosia, QWidget * parent, Qt::WFlags flags)
        : QGLViewer(parent, 0, flags), ambrosia(ambrosia), qglviewerDone(false)
    {
        construct();

        if (ambrosia) {
            qDebug() << "getSceneRadius" << ambrosia->getRadius();
            ambrosia->incRefCount();
            setSceneRadius(ambrosia->getRadius());
            showEntireScene();
            emit modelChanged(this);

            // Set widget caption
            /*
              Utopia::Node::child_iterator iter = getComplex()->childrenBegin(&UTOPIA::MODEL::isComplex);
              Utopia::Node::child_iterator end = getComplex()->childrenEnd();
              string label = "";
              if (iter != end && (*iter)->attributes.exists("utopia name")) {
              label = (*iter)->getAttribute<string>("utopia name");
              label += ": " + (*iter)->getAttribute<string>("utopia name");
              } else {
              label = "Unnamed model";
              }
              setWindowTitle(QString(label.c_str()));
            */
        }
    }

    void AmbrosiaWidget::construct() {
        setMouseTracking(FALSE);
        //      setFocusPolicy(QGLViewer::ClickFocus);
        resize(QSize(400, 300).expandedTo(minimumSizeHint()));
        //      clearWState(Qt::WState_Polished);
        this->setWindowIcon(QPixmap(":/icons/ambrosia-32.png"));

        // Set zooming to ALT+Left
        setMouseBinding(Qt::LeftButton + Qt::ALT, CAMERA, ZOOM);

        // Tidy up shotcuts
        setShortcut(DRAW_AXIS,  0);
        setShortcut(DRAW_GRID,  0);
        setShortcut(DISPLAY_FPS,        0);
        setShortcut(ENABLE_TEXT,        0);
        setShortcut(EXIT_VIEWER,        0);
        setShortcut(SAVE_SCREENSHOT,    Qt::CTRL+Qt::Key_S);
        setShortcut(CAMERA_MODE,        0);
        setShortcut(FULL_SCREEN,        Qt::ALT+Qt::Key_Return);
        setShortcut(STEREO,             Qt::Key_S);
        setShortcut(ANIMATION,  0);
        setShortcut(HELP,               0);
        setShortcut(EDIT_CAMERA,        0);
        setShortcut(MOVE_CAMERA_LEFT,   0);
        setShortcut(MOVE_CAMERA_RIGHT, 0);
        setShortcut(MOVE_CAMERA_UP,     0);
        setShortcut(MOVE_CAMERA_DOWN,   0);
        setShortcut(INCREASE_FLYSPEED,0);
        setShortcut(DECREASE_FLYSPEED,0);
        setShortcut(SNAPSHOT_TO_CLIPBOARD,Qt::CTRL+Qt::Key_C);


#ifndef Q_WS_MAC
        QIcon fileExportIcon(":/icons/fileexport.png");
        QIcon fileCloseIcon(":/icons/fileclose.png");
#else
        QIcon fileExportIcon("");
        QIcon fileCloseIcon("");
#endif

        // Context menu
        QActionGroup * renderFormatsGroup = new QActionGroup(this);

        contextMenu = new QMenu(this);
        contextMenuDisplay = new QMenu("Display", contextMenu);
        contextMenuOptions = new QMenu("Options", contextMenu);

        contextMenu->addMenu(contextMenuDisplay);
        contextMenu->addMenu(contextMenuOptions);
        contextMenu->addSeparator();
        //      contextMenu->addAction(fileExportIcon, "Export model...", this, SLOT(contextExportComplex()));
        contextMenu->addAction(fileExportIcon, "Save Snapshot...", this, SLOT(contextSaveSnapshot()));
        //      contextMenu->addSeparator();
        //      contextMenu->addAction("Clone View", this, SLOT(contextRequestClone()));
        contextMenu->addSeparator();
        contextMenu->addAction(fileCloseIcon, "Close View", this, SLOT(contextClose()));

        displaySpacefillAction = contextMenuDisplay->addAction("Spacefill", this, SLOT(contextDisplaySpacefill()));
        displaySpacefillAction->setCheckable(TRUE);
        renderFormatsGroup->addAction(displaySpacefillAction);
        displayBackboneAction = contextMenuDisplay->addAction("Backbone", this, SLOT(contextDisplayBackbone()));
        displayBackboneAction->setCheckable(TRUE);
        renderFormatsGroup->addAction(displayBackboneAction);
        displayCartoonAction = contextMenuDisplay->addAction("Cartoon", this, SLOT(contextDisplayCartoon()));
        displayCartoonAction->setCheckable(TRUE);
        renderFormatsGroup->addAction(displayCartoonAction);
        displayEncapsulatedBackboneAction = contextMenuDisplay->addAction("Encapsulated Backbone", this, SLOT(contextDisplayEncapsulatedBackbone()));
        displayEncapsulatedBackboneAction->setCheckable(TRUE);
        renderFormatsGroup->addAction(displayEncapsulatedBackboneAction);
        displayBackboneAction->setChecked(TRUE);

        //      contextMenuOptions->setCheckable(TRUE);
        optionsSmoothBackbonesAction = contextMenuOptions->addAction("Smooth Backbones", this, SLOT(contextOptionsSmoothBackbones()));
        optionsSmoothBackbonesAction->setCheckable(TRUE);
        optionsSmoothBackbonesAction->setChecked(true);
        optionsChunkyBackbonesAction = contextMenuOptions->addAction("Chunky Backbones", this, SLOT(contextOptionsChunkyBackbones()));
        optionsChunkyBackbonesAction->setCheckable(TRUE);
        optionsChunkyBackbonesAction->setChecked(true);
        contextMenuOptions->addSeparator();
        optionsShowSidechainsAction = contextMenuOptions->addAction("Show Sidechains", this, SLOT(contextOptionsShowSidechains()));
        optionsShowSidechainsAction->setCheckable(TRUE);
        optionsShowSidechainsAction->setChecked(true);
    }

    AmbrosiaWidget::~AmbrosiaWidget()
    {
        if (ambrosia) {
            ambrosia->decRefCount();
            if (ambrosia->getRefCount() == 0) {
                delete ambrosia;
                ambrosia = 0;
            }
        }
        emit deleted(this);
        clear();
    }

    bool AmbrosiaWidget::supports(Utopia::Node * model) const
    {
        // is it just a sequence?
        bool supported = (model->type() == Utopia::Node::getNode("complex"));

        // if not, is it a collection of sequences?
        if (!supported)
        {
            Utopia::Node::relation::iterator seq = model->relations(Utopia::UtopiaSystem.hasPart).begin();
            Utopia::Node::relation::iterator end = model->relations(Utopia::UtopiaSystem.hasPart).end();

            while (seq != end)
            {
                if ((*seq)->type() == Utopia::Node::getNode("complex"))
                {
                    supported = true;
                    break;
                }
                ++seq;
            }
        }

        return supported;
    }

    bool AmbrosiaWidget::load(QString uri)
    {
        // Create ambrosia object
        if (ambrosia == 0) {
            ambrosia = new Ambrosia();
            ambrosia->incRefCount();
        }

        // Load model
        bool success = ambrosia->load(uri.toStdString());
        if (success) {
            qDebug() << "getSceneRadius" << ambrosia->getRadius();
            setSceneRadius(ambrosia->getRadius());
            showEntireScene();
            emit modelChanged(this);
            /*
            // Set widget caption
            Utopia::Node::child_iterator iter = getComplex()->childrenBegin(&UTOPIA::MODEL::isComplex);
            Utopia::Node::child_iterator end = getComplex()->childrenEnd();
            string label = "";
            if (iter != end && (*iter)->attributes.exists("utopia name")) {
            label = (*iter)->getAttribute<string>("utopia name");
            } else {
            label = "Unnamed model";
            }
            setWindowTitle(QString(label.c_str()));
            */
        }

        contextDisplayBackbone();
        return success;
    }


    bool AmbrosiaWidget::load(Utopia::Node * complex)
    {
        qDebug() << "AmbrosiaWidget::load()";
        qDebug() << " ambrosia =" << ambrosia;

        // Create ambrosia object
        if (ambrosia == 0) {
            ambrosia = new Ambrosia();
            ambrosia->incRefCount();
        }
        qDebug() << " ambrosia =" << ambrosia;


        // is it just a sequence?
        if (complex->type() != Utopia::Node::getNode("complex"))
        {
            Utopia::Node::relation::iterator seq = complex->relations(Utopia::UtopiaSystem.hasPart).begin();
            Utopia::Node::relation::iterator end = complex->relations(Utopia::UtopiaSystem.hasPart).end();

            while (seq != end)
            {
                if ((*seq)->type() == Utopia::Node::getNode("complex"))
                {
                    complex = *seq;
                    break;
                }
                ++seq;
            }
        }


        // Load complex
        bool success = ambrosia->load(complex);
        if (success) {
            qDebug() << "getSceneRadius" << ambrosia->getRadius();
            setSceneRadius(ambrosia->getRadius());
            showEntireScene();
            emit modelChanged(this);

            // Set widget caption
            string label = "";
            if (complex && complex->attributes.exists("utopia name")) {
                label = complex->attributes.get("utopia name").toString().toStdString();
            } else {
                label = "Unnamed model";
            }
            if (this->isWindow()) { setWindowTitle(QString(label.c_str())); }
        }
        else
        {
            qDebug() << "AmbrosiaWidget::load() failed";
        }

        contextDisplayBackbone();
        //      contextDisplaySpacefill();
        return success;
    }


    void AmbrosiaWidget::clear()
    {
        if (ambrosia) {
            ambrosia->clear();
            emit modelChanged(this);
        }
    }

    Utopia::Node * AmbrosiaWidget::getComplex()
    {
        return ambrosia == 0 ? 0 : ambrosia->getComplex();
    }


    void AmbrosiaWidget::processEvents()
    {
    }


    bool AmbrosiaWidget::viewingAnnotationOnComplex(Utopia::Node * model, QString className, QString name)
    {
        return visualisedAnnotations.contains(name);
    }

    void AmbrosiaWidget::setDisplay(bool display, Ambrosia::RenderSelection renderSelection, Selection * selection)
    {
        if (ambrosia)
            ambrosia->setDisplay(display, renderSelection, selection);
    }


    void AmbrosiaWidget::setVisible(bool visible, Ambrosia::RenderSelection renderSelection, Selection * selection)
    {
        if (ambrosia)
            ambrosia->setVisible(visible, renderSelection, selection);
    }


    void AmbrosiaWidget::setRenderFormat(unsigned int renderFormat, Ambrosia::RenderSelection renderSelection, Selection * selection)
    {
        if (ambrosia)
            ambrosia->setRenderFormat(renderFormat, renderSelection, selection);
    }


    void AmbrosiaWidget::setRenderOption(unsigned int renderOption, bool flag, Ambrosia::RenderSelection renderSelection, Selection * selection)
    {
        if (ambrosia)
            ambrosia->setRenderOption(renderOption, flag, renderSelection, selection);
    }


    void AmbrosiaWidget::setColour(Colour * colour, Ambrosia::RenderSelection renderSelection, Selection * selection)
    {
        if (ambrosia)
            ambrosia->setColour(colour, renderSelection, selection);
    }


    void AmbrosiaWidget::setAlpha(GLubyte alpha, Ambrosia::RenderSelection renderSelection, Selection * selection)
    {
        if (ambrosia)
            ambrosia->setAlpha(alpha, renderSelection, selection);
    }


    void AmbrosiaWidget::setTintColour(Colour * colour, Ambrosia::RenderSelection renderSelection, Selection * selection)
    {
        if (ambrosia)
            ambrosia->setTintColour(colour, renderSelection, selection);
    }


    void AmbrosiaWidget::setHighlightColour(Colour * colour, Ambrosia::RenderSelection renderSelection, Selection * selection)
    {
        if (ambrosia)
            ambrosia->setHighlightColour(colour, renderSelection, selection);
    }


    void AmbrosiaWidget::setRenderTag(Ambrosia::RenderTag renderTag, Ambrosia::RenderSelection renderSelection, Selection * selection)
    {
        if (ambrosia)
            ambrosia->setRenderTag(renderTag, renderSelection, selection);
    }


    void AmbrosiaWidget::enable(Ambrosia::RenderOption option)
    {
        if (ambrosia)
            ambrosia->enable(option);
    }


    void AmbrosiaWidget::disable(Ambrosia::RenderOption option)
    {
        if (ambrosia)
            ambrosia->disable(option);
    }


    void AmbrosiaWidget::enable(Ambrosia::RenderOption option, bool enable)
    {
        if (ambrosia)
            ambrosia->enable(option, enable);
    }


    bool AmbrosiaWidget::isEnabled(Ambrosia::RenderOption option)
    {
        return (ambrosia && ambrosia->isEnabled(option));
    }


    void AmbrosiaWidget::setBackgroundColour(Colour * colour)
    {
        makeCurrent();
        if (colour) {
            unsigned char _r, _g, _b;
            colour->get(_r, _g, _b);
            float r, g, b;
            r = ((float) _r) / 255.0;
            g = ((float) _g) / 255.0;
            b = ((float) _b) / 255.0;
            glClearColor(r, g, b, 1.0);
        } else {
            glClearColor(0.98, 0.97, 0.93, 1.0);
        }
    }


    void AmbrosiaWidget::mousePressEvent(QMouseEvent * e)
    {
        popupClicked = (e->button() == Qt::RightButton);
        popupMoved = false;
        popupX = e->globalX();
        popupY = e->globalY();
        QGLViewer::mousePressEvent(e);
    }


    void AmbrosiaWidget::mouseMoveEvent(QMouseEvent * e)
    {
        popupMoved = popupClicked;
        QGLViewer::mouseMoveEvent(e);
    }


    void AmbrosiaWidget::mouseReleaseEvent(QMouseEvent * e)
    {
        if (popupClicked && !popupMoved) {
            QTimer::singleShot(200, this, SLOT(popup()));
        }
        QGLViewer::mouseReleaseEvent(e);
    }


    void AmbrosiaWidget::mouseDoubleClickEvent(QMouseEvent * e)
    {
        popupClicked = popupMoved = false;
        QGLViewer::mouseDoubleClickEvent(e);
    }

    void AmbrosiaWidget::windowActivationChange( bool oldState )
    {
        if (isActiveWindow() && oldState == FALSE) {
            emit focusReceived(this);
        } else if (!isActiveWindow() && oldState == TRUE) {
            emit focusLost(this);
        }
    }

    void AmbrosiaWidget::closeEvent(QCloseEvent * event)
    {
        emit closed(this);
        this->QGLViewer::closeEvent(event);
    }

    void AmbrosiaWidget::showComplex(Utopia::Node * model)
    {
        makeCurrent();
        Selection selection;
        selection.add(model);
        setDisplay(true, Ambrosia::CUSTOM, &selection);
    }


    void AmbrosiaWidget::hideComplex(Utopia::Node * model)
    {
        makeCurrent();
        Selection selection;
        selection.add(model);
        setDisplay(false, Ambrosia::CUSTOM, &selection);
    }

    void AmbrosiaWidget::applyHighlights()
    {
        makeCurrent();
        /*
          Utopia::Node * complex = getComplex();
          Selection solid;
          Selection shade;
          unsigned int resIndex = 0;
          Utopia::Node::descendant_iterator iter = complex->descendantsBegin(&UTOPIA::MODEL::isResidue, false);
          Utopia::Node::descendant_iterator end = complex->descendantsEnd();
          for (; iter != end; ++iter, ++resIndex) {
          bool highlighted = this->_highlights.empty();
          std::map< unsigned int, std::pair< unsigned int, unsigned int > >::iterator hl_iter = this->_highlights.begin();
          std::map< unsigned int, std::pair< unsigned int, unsigned int > >::iterator hl_end = this->_highlights.end();
          for (; !highlighted && hl_iter != hl_end; ++hl_iter)
          {
          highlighted = (resIndex >= hl_iter->second.first && resIndex < hl_iter->second.first + hl_iter->second.second);
          }
          if (highlighted) {
          solid.add(*iter);
          } else {
          shade.add(*iter);
          }
          }
          setRenderTag(Ambrosia::SOLID, Ambrosia::CUSTOM, &solid);
          setRenderTag(Ambrosia::Am_TRANSPARENT, Ambrosia::CUSTOM, &shade);
        */
    }

    void AmbrosiaWidget::newHighlight(unsigned int id, unsigned int start, unsigned int length)
    {
        this->_highlights[id] = std::make_pair(start, length);

        this->applyHighlights();
    }

    void AmbrosiaWidget::modifyHighlight(unsigned int id, unsigned int start, unsigned int length)
    {
        if (this->_highlights.find(id) == this->_highlights.end())
        {
            // Danger Will Robinson!
            std::cout << "Warning! Attempt made to modify a non-existent highlight (id=" << id << ")" << std::endl;
        }
        else if (length == 0)
        {
            this->removeHighlight(id);
        }
        else
        {
            this->newHighlight(id, start, length);
        }

        this->applyHighlights();
    }

    void AmbrosiaWidget::removeHighlight(unsigned int id)
    {
        this->_highlights.erase(id);

        this->applyHighlights();
    }

    void AmbrosiaWidget::showExtentAnnotation(QString name)
    {
        /*
          makeCurrent();
          setTintColour(Colour::getColour("dark-grey", 50, 50, 50), Ambrosia::RESIDUES);

          // Show Annotations
          Utopia::Node * source = getComplex();
          // Retrieve annotation data from source
          Utopia::Node::descendant_iterator model_iter = source->descendantsBegin(&UTOPIA::MODEL::isComplex, false);
          Utopia::Node::descendant_iterator model_end = source->descendantsEnd();
          for (; model_iter != model_end; ++model_iter) {
          // Check ALL nodes for specific annotation type
          Utopia::Node::descendant_iterator iter = (*model_iter)->descendantsBegin(&UTOPIA::MODEL::isResidue, false);
          Utopia::Node::descendant_iterator end = (*model_iter)->descendantsEnd();
          for (; iter != end; ++iter) {
          Utopia::Node::parent_iterator ann_iter = (*iter)->parentsBegin(&UTOPIA::MODEL::isAnnotation);
          Utopia::Node::parent_iterator ann_end = (*iter)->parentsEnd();
          for (; ann_iter != ann_end; ++ann_iter) {
          if((*ann_iter)->attributes.exists("class") && (*ann_iter)->attributes.exists("name") && (*ann_iter)->attributes.exists("width") && (*ann_iter)->getAttribute<string>("class") == "extent" && QString((*ann_iter)->getAttribute<string>("name").c_str()) == name) {
          Selection select;
          int width = (*ann_iter)->getAttribute<int>("width");
          string type = "?";
          if ((*ann_iter)->attributes.exists("type")) {
          type = (*ann_iter)->getAttribute<string>("type");
          }
          Utopia::Node::descendant_iterator copy_iter = iter;
          for (; copy_iter != end && width > 0; ++copy_iter, --width) {
          select.add(*copy_iter);
          }
          //                                    setDisplay(true, Ambrosia::CUSTOM, &select);
          if (name == "Secondary Structure") {
          setTintColour(Colour::getColour(string("ss.") + type), Ambrosia::CUSTOM, &select);
          } else {
          Colour * colour = Colour::getColour("green");
          if (name == "Tight End Fragments")
          colour = Colour::getColour("yellow");
          setTintColour(colour, Ambrosia::CUSTOM, &select);
          }
          //                                    setRenderFormat(Ambrosia::getToken("Render Format", "Spacefill"), Ambrosia::CUSTOM, &select);
          //                                    setRenderTag(Ambrosia::SOLID, Ambrosia::CUSTOM, &select);
          }
          }
          }
          }
        */
    }

    void AmbrosiaWidget::showBooleanAnnotation(QString name)
    {
        /*
          makeCurrent();
          setTintColour(Colour::getColour("dark-grey", 50, 50, 50), Ambrosia::RESIDUES);

          // Show Annotations
          Utopia::Node * source = getComplex();
          // Retrieve annotation data from source
          Utopia::Node::descendant_iterator model_iter = source->descendantsBegin(&UTOPIA::MODEL::isComplex, false);
          Utopia::Node::descendant_iterator model_end = source->descendantsEnd();
          for (; model_iter != model_end; ++model_iter) {
          // Check ALL nodes for specific annotation type
          Utopia::Node::descendant_iterator iter = (*model_iter)->descendantsBegin(&UTOPIA::MODEL::isResidue, false);
          Utopia::Node::descendant_iterator end = (*model_iter)->descendantsEnd();
          for (; iter != end; ++iter) {
          Utopia::Node::parent_iterator ann_iter = (*iter)->parentsBegin(&UTOPIA::MODEL::isAnnotation);
          Utopia::Node::parent_iterator ann_end = (*iter)->parentsEnd();
          for (; ann_iter != ann_end; ++ann_iter) {
          if((*ann_iter)->attributes.exists("class") && (*ann_iter)->attributes.exists("name") && (*ann_iter)->getAttribute<string>("class") == "boolean" && QString((*ann_iter)->getAttribute<string>("name").c_str()) == name) {
          Selection select;
          select.add(*iter);
          //                                    setDisplay(true, Ambrosia::CUSTOM, &select);
          Colour * colour = Colour::getColour("green");
          if (name == "Topohydrophobic Residues")
          colour = Colour::getColour("topo-blue", 50, 150, 255);
          else if (name == "Most Interacting Residues")
          colour = Colour::getColour("mirs-red", 255, 70, 20);
          setTintColour(colour, Ambrosia::CUSTOM, &select);
          //                                    setRenderFormat(Ambrosia::getToken("Render Format", "Spacefill"), Ambrosia::CUSTOM, &select);
          //                                    setRenderTag(Ambrosia::SOLID, Ambrosia::CUSTOM, &select);
          }
          }
          }
          }
        */
    }

    void AmbrosiaWidget::showValueAnnotation(QString name, QString value)
    {
        makeCurrent();
        cout << "request received to show value annotation" << endl;
    }

    QString AmbrosiaWidget::getAnnotationPath(Utopia::Node * annotation)
    {
        /*
          QString path(annotation->getAttribute<string>("name").c_str());
          Utopia::Node::parent_iterator ann_iter = annotation->parentsBegin(&UTOPIA::MODEL::isAnnotation);
          Utopia::Node::parent_iterator ann_end = annotation->parentsEnd();
          if (ann_iter != ann_end && (*ann_iter)->attributes.exists("class") && (*ann_iter)->attributes.exists("name")) {
          return getAnnotationPath(*ann_iter) + "/" + path;
          } else {
          return path;
          }
        */
        return "";
    }

    void AmbrosiaWidget::showAnnotations(Utopia::Node * complex, QList<QString> annotations)
    {
        /*
          QListIterator<QString> i(annotations);
          while (i.hasNext()) {
          QString annotation = i.next();
          if (annotation.section( '/', -1 ) == annotation.section( '/', -2, -2 )) {
          annotation = annotation.left(annotation.lastIndexOf('/'));
          }
          if (visualisedAnnotations.contains(annotation)) {
          annotations.removeAll(annotation);
          } else {
          visualisedAnnotations.append(annotation);
          }
          }

          makeCurrent();
          //setTintColour(Colour::getColour("dark-grey", 100, 100, 100), Ambrosia::RESIDUES);
          setTintColour(Colour::getColour("ss.?"), Ambrosia::RESIDUES);

          // Check ALL nodes for specific annotation type
          Utopia::Node::descendant_iterator iter = complex->descendantsBegin(&UTOPIA::MODEL::isResidue, false);
          Utopia::Node::descendant_iterator end = complex->descendantsEnd();
          for (; iter != end; ++iter) {
          Utopia::Node::parent_iterator ann_iter = (*iter)->parentsBegin(&UTOPIA::MODEL::isAnnotation);
          Utopia::Node::parent_iterator ann_end = (*iter)->parentsEnd();
          for (; ann_iter != ann_end; ++ann_iter) {
          if((*ann_iter)->attributes.exists("class") && (*ann_iter)->attributes.exists("name")) {
          QString className((*ann_iter)->getAttribute<string>("class").c_str());
          QString name((*ann_iter)->getAttribute<string>("name").c_str());
          QString fullPath = getAnnotationPath(*ann_iter);
          if (className == "value") {
          // Deal with values...
          QListIterator<QString> i(annotations);
          while (i.hasNext()) {
          QString annotation = i.next();
          if (annotation.section( '/', -1 ) == annotation.section( '/', -2, -2 )) {
          annotation = annotation.left(annotation.lastIndexOf('/'));
          }
          QString attribute = annotation.section('/', -1);
          if (QString("%1/%2").arg(fullPath).arg(attribute) == annotation) {
          // Found value to draw
          if ((*ann_iter)->attributes.exists(attribute.toStdString())) {
          QString annotationWithUnderscores = annotation;
          annotationWithUnderscores.replace(' ', '_');
          Selection select;
          select.add(*iter);
          Colour * max_col = Colour::getColour(QString("value/%1:max").arg(annotationWithUnderscores).toStdString());
          Colour * min_col = Colour::getColour(QString("value/%1:min").arg(annotationWithUnderscores).toStdString());
          float value;
          if ((*ann_iter)->getAttributeType(attribute.toStdString()) == "float") {
          value = (*ann_iter)->getAttribute<float>(attribute.toStdString());
          } else if ((*ann_iter)->getAttributeType(attribute.toStdString()) == "int") {
          value = (*ann_iter)->getAttribute<int>(attribute.toStdString());
          }
          float min = 0.0, max = 1.0;
          //                                 std::cout << QString("value/%1:min").arg(annotationWithUnderscores).toStdString() << std::endl;
          if (annotation == "PFF/FoldX/value") {
          min = -5.0;
          max = 10.0;
          } else if (annotation == "PFF/Accessibility/value") {
          min = 0.0;
          max = 300.0;
          } else if (annotation.endsWith("Torsion Angles/Psi")) {
          min = -180.0;
          max = 180.0;
          } else if (annotation.endsWith("Torsion Angles/Alpha")) {
          min = -180.0;
          max = 180.0;
          } else if (annotation.endsWith("Torsion Angles/Omega")) {
          min = -180.0;
          max = 180.0;
          } else if (annotation.endsWith("Torsion Angles/Phi")) {
          min = -180.0;
          max = 180.0;
          } else if (annotation.endsWith("Torsion Angles/Kappa")) {
          min = 0.0;
          max = 180.0;
          } else if (annotation == "PFF/PoPMuSiC Analyses/Destabilisation") {
          min = 0.0;
          max = 160.0;
          } else if (annotation == "PFF/PoPMuSiC Analyses/Stabilisation") {
          min = -55.0;
          max = 0.0;
          } else if (annotation == "PFF/Fugue/score") {
          min = 0.0;
          max = 9.0;
          } else if (annotation == "PFF/Lattice/value") {
          min = 0.0;
          max = 9.0;
          } else if (annotation == "PFF/Folding Scores/Folding Region Conservation") {
          min = 0.0;
          max = 1.0;
          } else if (annotation == "PFF/Folding Scores/Functional Region Conservation") {
          min = 0.0;
          max = 1.0;
          } else if (annotation == "PFF/Folding Scores/Raw Score") {
          min = -8.0;
          max = 11.0;
          } else if (annotation == "Crescendo/score") {
          min = -3.0;
          max = 3.0;
          } else {
          cout << "Cannot find range for " << annotation.toStdString() << endl;
          }
          double unit = (value - min) / (max - min);
          Colour * colour = Colour::getColour(min_col->r + (unsigned char)((max_col->r - min_col->r) * unit), min_col->g + (unsigned char)((max_col->g - min_col->g) * unit), min_col->b + (unsigned char)((max_col->b - min_col->b) * unit));
          setTintColour(colour, Ambrosia::CUSTOM, &select);
          }
          }
          }
          } else if (visualisedAnnotations.contains(fullPath)) {
          if (className == "extent") {
          Selection select;
          int width = (*ann_iter)->getAttribute<int>("width");
          Utopia::Node::descendant_iterator copy_iter = iter;
          for (; copy_iter != end && width > 0; ++copy_iter, --width) {
          select.add(*copy_iter);
          }
          QString annotationWithUnderscores = fullPath;
          annotationWithUnderscores.replace(' ', '_');

          Colour * col = 0;
          QColor qcol;
          if ((*ann_iter)->attributes.exists("colour") && (qcol.setNamedColor((*ann_iter)->getAttribute< std::string >("colour").c_str()), qcol.isValid()))
          {
          int r, g, b;
          qcol.getRgb(&r, &g, &b);
          col = Colour::getColour((unsigned char) r, (unsigned char) g, (unsigned char) b);
          }
          else
          {
          col = Colour::getColour(QString("extent/%1").arg(annotationWithUnderscores).toStdString());
          }
          setTintColour(col, Ambrosia::CUSTOM, &select);
          } else if (className == "boolean") {
          Selection select;
          select.add(*iter);

          QString annotationWithUnderscores = fullPath;
          annotationWithUnderscores.replace(' ', '_');

          Colour * col = 0;
          QColor qcol;
          if ((*ann_iter)->attributes.exists("colour") && (qcol.setNamedColor((*ann_iter)->getAttribute< std::string >("colour").c_str()), qcol.isValid()))
          {
          int r, g, b;
          qcol.getRgb(&r, &g, &b);
          col = Colour::getColour((unsigned char) r, (unsigned char) g, (unsigned char) b);
          }
          else
          {
          col = Colour::getColour(QString("boolean/%1").arg(annotationWithUnderscores).toStdString());
          }
          setTintColour(col, Ambrosia::CUSTOM, &select);
          }
          }
          }
          }
          }

          // Send signals
          if (!annotations.empty()) {
          emit annotationsShown(complex, annotations);
          }
        */
    }

    void AmbrosiaWidget::hideAnnotations(Utopia::Node * complex, QList<QString> annotations)
    {
        QListIterator<QString> i(annotations);
        while (i.hasNext()) {
            QString annotation = i.next();
            if (annotation.section( '/', -1 ) == annotation.section( '/', -2, -2 )) {
                annotation = annotation.left(annotation.lastIndexOf('/'));
            }
            visualisedAnnotations.removeAll(annotation);
        }

        if (visualisedAnnotations.empty()) {
            hideAnnotations();
        } else {
            showAnnotations(complex, QList<QString>());
        }

        // Emit signals
        i.toFront();
        while (i.hasNext()) {
            QString annotation = i.next();
            if (annotation.section( '/', -1 ) == annotation.section( '/', -2, -2 )) {
                annotation = annotation.left(annotation.lastIndexOf('/'));
            }
            if (visualisedAnnotations.contains(annotation)) {
                annotations.removeAll(annotation);
            }
        }
        if (!annotations.empty()) {
            emit annotationsHidden(complex, annotations);
        }
    }

    void AmbrosiaWidget::hideAnnotations()
    {
        makeCurrent();
        setTintColour(0, Ambrosia::RESIDUES);
        visualisedAnnotations.clear();
    }

    void AmbrosiaWidget::popup()
    {
        if (popupClicked && !popupMoved) {
            contextMenu->popup(QPoint(popupX, popupY));
        }
    }

    void AmbrosiaWidget::contextDisplaySpacefill()
    {
        setDisplay(true);
        setRenderFormat(Ambrosia::getToken("Render Format", "Spacefill"));
        setRenderTag(Ambrosia::SOLID);
        setDisplay(false, Ambrosia::WATER);

        //     setHighlightColour(Colour::getColour("cyan"), Ambrosia::SULPHUR);
        //     setRenderTag(Ambrosia::OUTLINE, Ambrosia::SULPHUR);

        updateGL();
    }

    void AmbrosiaWidget::contextDisplayBackbone()
    {
        setDisplay(true);
        setRenderFormat(Ambrosia::getToken("Render Format", "Backbone Trace"));
        setRenderTag(Ambrosia::SOLID);
        setRenderFormat(Ambrosia::getToken("Render Format", "Spacefill"), Ambrosia::HETEROGENS);
        setRenderTag(Ambrosia::SOLID, Ambrosia::HETEROGENS);
        setDisplay(false, Ambrosia::WATER);

        //   setHighlightColour(Colour::getColour("cyan"), Ambrosia::SULPHUR);
        //   setRenderTag(Ambrosia::OUTLINE, Ambrosia::SULPHUR);

        updateGL();
    }

    void AmbrosiaWidget::contextDisplayCartoon()
    {
        setDisplay(true);
        setRenderFormat(Ambrosia::getToken("Render Format", "Cartoon"));
        setRenderTag(Ambrosia::SOLID);
        setRenderFormat(Ambrosia::getToken("Render Format", "Spacefill"), Ambrosia::HETEROGENS);
        setRenderTag(Ambrosia::SOLID, Ambrosia::HETEROGENS);
        setDisplay(false, Ambrosia::WATER);

        //   setHighlightColour(Colour::getColour("cyan"), Ambrosia::SULPHUR);
        //   setRenderTag(Ambrosia::OUTLINE, Ambrosia::SULPHUR);

        updateGL();
    }

    void AmbrosiaWidget::contextDisplayEncapsulatedBackbone()
    {
        setDisplay(true);
        setRenderFormat(Ambrosia::getToken("Render Format", "Backbone Trace"));
        setRenderTag(Ambrosia::SOLID);
        setRenderFormat(Ambrosia::getToken("Render Format", "Spacefill"), Ambrosia::ATOMS);
        setRenderTag(Ambrosia::Am_TRANSPARENT, Ambrosia::ATOMS);
        setRenderTag(Ambrosia::SOLID, Ambrosia::HETEROGENS);
        setDisplay(false, Ambrosia::WATER);

        //     setHighlightColour(Colour::getColour("cyan"), Ambrosia::SULPHUR);
        //     setRenderTag(Ambrosia::OUTLINE, Ambrosia::SULPHUR);

        updateGL();
    }

    void AmbrosiaWidget::contextOptionsSmoothBackbones()
    {
        setRenderOption(Ambrosia::getToken("Render Option", "Smooth Backbones"), optionsSmoothBackbonesAction->isChecked());
        updateGL();
    }

    void AmbrosiaWidget::contextOptionsChunkyBackbones()
    {
        setRenderOption(Ambrosia::getToken("Render Option", "Chunky Backbones"), optionsChunkyBackbonesAction->isChecked());
        updateGL();
    }

    void AmbrosiaWidget::contextOptionsShowSidechains()
    {
        setVisible(optionsShowSidechainsAction->isChecked(), Ambrosia::SIDECHAIN);
        updateGL();
    }

    void AmbrosiaWidget::contextExportComplex()
    {
        cout << "FIXME: AmbrosiaWidget::contextExportComplex() not yet implemented" << endl;
    }

    void AmbrosiaWidget::contextSaveSnapshot()
    {
        saveSnapshot(false);
    }

    void AmbrosiaWidget::contextRequestClone()
    {
        if (ambrosia) {
            emit cloneRequested(this, ambrosia);
        }
    }

    void AmbrosiaWidget::contextClose()
    {
        close();
    }

    void AmbrosiaWidget::init()
    {
        qDebug() << "AmbrosiaWidget::init()";

        // Initialise openGL
        Colour * col = 0;
        glEnable(GL_DEPTH_TEST);
        Colour::populate("ambrosia.colourmap");
        setBackgroundColour(Colour::getColour("ambrosia.background"));
        glClearDepth(1.0f);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_COLOR_MATERIAL);
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
        float dir[4] = {1.0, 1.0, 1.0, 0.0};
        glLightfv(GL_LIGHT0, GL_POSITION, dir);
        float amb_l[4] = {1.0};
        col = Colour::getColour("ambrosia.lighting.ambient");
        col->getf(amb_l[0], amb_l[1], amb_l[2]);
        float amb_m[4] = {1.0};
        col = Colour::getColour("ambrosia.material.ambient");
        col->getf(amb_m[0], amb_m[1], amb_m[2]);
        glLightfv(GL_LIGHT0, GL_AMBIENT, amb_l);
        glMaterialfv(GL_FRONT, GL_AMBIENT, amb_m);
        float dif_l[4] = {1.0};
        col = Colour::getColour("ambrosia.lighting.diffuse");
        col->getf(dif_l[0], dif_l[1], dif_l[2]);
        float dif_m[4] = {1.0};
        col = Colour::getColour("ambrosia.material.diffuse");
        col->getf(dif_m[0], dif_m[1], dif_m[2]);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, dif_l);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, dif_m);

        // Ensure Ambrosia is initialised to NULL
        ambrosia = 0;
    }

    void AmbrosiaWidget::drawWithNames()
    {
        if (ambrosia)
            ambrosia->render(Ambrosia::NAME_PASS);
    }

    bool smooth = true;

    void AmbrosiaWidget::draw()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        if (ambrosia) {

            if (!ambrosia->built())
            {
                ambrosia->build();
                setSceneRadius(ambrosia->getRadius());
                showEntireScene();
            }

//                      qDebug() << "draw()";
            //          ambrosia->render(Ambrosia::SHADOW_MAP_PASS);
            ambrosia->render(Ambrosia::STENCIL_PASS);
            ambrosia->render(Ambrosia::DRAW_PASS);
            ambrosia->render(Ambrosia::DEPTH_SHADE_PASS);
            ambrosia->render(Ambrosia::DRAW_SHADE_PASS);
            ambrosia->render(Ambrosia::DEPTH_TRANSPARENT_PASS);
            ambrosia->render(Ambrosia::DRAW_TRANSPARENT_PASS);
            ambrosia->render(Ambrosia::DRAW_OUTLINE_PASS);
        }

        //     this->_toolbar->draw();
        //     this->_orientation->draw();
        //     this->_annotation->draw();
        //     this->_timeline->draw();

        //    std::cout << selectedName() << std::endl;
    }

    void AmbrosiaWidget::postSelection(const QPoint& point)
    {
        Renderable* r = Renderable::v2_get_from_name(selectedName());
        if (r)
        {
            if (highlighted.find(r) == highlighted.end())
            {
                highlighted.insert(r);
            }
            else
            {
                highlighted.erase(r);
            }
        }
        else
        {
            highlighted.clear();
        }

        setRenderTag(Ambrosia::SOLID);

        std::set< Renderable* >::iterator iter = highlighted.begin();
        std::set< Renderable* >::iterator end = highlighted.end();
        for (; iter != end; ++iter)
        {
            (*iter)->setTag(Ambrosia::OUTLINE);
            (*iter)->setHighlightColour(Colour::getColour("cyan"));
        }
    }

}
