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

#include <papyro/annotationprocessor.h>
#include <papyro/selectionprocessor.h>
#include <utopia2/qt/slidelayout.h>

#include <graffiti/header.h>
#include <graffiti/gridview.h>
#include <graffiti/grid.h>

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QSlider>
#include <QStackedLayout>
#include <QTableWidget>
#include <QTransform>


class TablificationDialog : public QWidget
{
    Q_OBJECT

public:
    TablificationDialog(Spine::DocumentHandle document, Spine::AnnotationHandle annotation);
    TablificationDialog(Spine::DocumentHandle document, const Spine::Area & area);
    ~TablificationDialog();

    bool isValid() const;

signals:
    void idle();
    void message(const QString & msg);
    void saving();

protected slots:
    void onBackPressed();
    void onCancelPressed();
    void onClosePressed();
    void onExportPressed();
    void onSavePressed();
    void onHorizontalScrollBarActionTriggered(int value);
    void onHorizontalScrollBarRangeChanged(int from, int to);
    void onHorizontalScrollBarValueChanged(int value);
    void onNextPressed();
    void onRotateLeftPressed();
    void onRotateRightPressed();
    void onSlide(QWidget * widget);
    void onTableDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight);
    void onTableRowsOrColumnsChanged(const QModelIndex & parent, int start, int end);
    void onVerticalScrollBarActionTriggered(int value);
    void onVerticalScrollBarRangeChanged(int from, int to);
    void onVerticalScrollBarValueChanged(int value);
    void onZoomSliderValueChanged(int value);
    void setChanged(bool changed = true);

protected:
    struct {
        Spine::DocumentHandle document;
        Spine::AnnotationHandle annotation;
        Spine::Area area;
        int rotation;

        QTransform transform;
        QTransform logicalTransform;

        struct {
            double width;
            double height;
        } transformed;

        QPixmap image;
    } source;
    double defaultResolution;
    double minimumResolution;
    double resolution;
    double effectiveResolution();
    void calculateMinimumResolution();

    QSize zoomedSize;
    QPoint offset;
    QPoint visibleOffset;
    bool scrolled;
    double zoom;

    bool hasChanged;

    QGridLayout * gridLayout;

    void swapSections();

    QVector< QRectF > obstacles;
    void setHeaderSnapPoints();

    void renderImage();

    void repositionImage();

    // r = 0,1,2,3
    void setRotation(int rotation);

    void setResolution(double resolution);
    void setOffset(const QPoint & offset);

    bool eventFilter(QObject * obj, QEvent * event);

    Graffiti::Header * horizontalHeader;
    Graffiti::Header * verticalHeader;
    Graffiti::GridView * gridView;
    QWidget * imageLabel;
    QPushButton * rotateLeftButton;
    QPushButton * rotateRightButton;
    QPushButton * saveButton;

    QScrollBar * horizontalScrollBar;
    QScrollBar * verticalScrollBar;
    QSlider * zoomSlider;

    Utopia::SlideLayout * slideLayout;
    QWidget * gridPane;
    QWidget * tablePane;

    QTableWidget * table;

    void initialise();

    QRectF logicalRectForArea(const Spine::Area & area);
    QRectF logicalRectForBoundingBox(const Spine::BoundingBox & boundingBox);

    void calculateObstacles();

    void keyPressEvent(QKeyEvent * event);
    void keyReleaseEvent(QKeyEvent * event);

}; // class TablificationDialog


class Tablify : public QObject, public Papyro::SelectionProcessor, public Papyro::AnnotationProcessor, public Papyro::SelectionProcessorFactory
{
    Q_OBJECT

public:
    /////////////////////////////////////////////////////////////////////////////////////
    // Factory API

    void activate(Spine::DocumentHandle document, Spine::AnnotationSet annotations, const QPoint & globalPos = QPoint());
    bool canActivate(Spine::DocumentHandle document, Spine::AnnotationHandle annotation) const;
    int category() const;
    using Papyro::AnnotationProcessor::icon;
    QIcon icon() const;
    void processSelection(Spine::DocumentHandle document, Spine::CursorHandle cursor, const QPoint & globalPos = QPoint());
    QList< boost::shared_ptr< Papyro::SelectionProcessor > > selectionProcessors(Spine::DocumentHandle document, Spine::CursorHandle cursor);
    using Papyro::AnnotationProcessor::title;
    QString title() const;
    QString title(Spine::DocumentHandle document, Spine::AnnotationHandle annotation) const;
    int weight() const;

    /////////////////////////////////////////////////////////////////////////////////////
    // Required members

    // Put your dialog / whatever here
    // And any slots required

}; // class Tablify
