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

#include "tablify.h"

#include <graffiti/grid.h>
#include <graffiti/gridview.h>
#include <graffiti/header.h>
#include <graffiti/sections.h>
#include <papyro/utils.h>
#include <utopia2/qt/spinner.h>

#include <QCoreApplication>
#include <QFile>
#include <QFileDialog>
#include <QImage>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QtAlgorithms>
#include <QStandardItemModel>
#include <QWheelEvent>

#include <QtCore/qmath.h>

#include <QDebug>


TablificationDialog::TablificationDialog(Spine::DocumentHandle document, Spine::AnnotationHandle annotation)
    : QWidget(0), hasChanged(false)
{
    source.document = document;
    source.annotation = annotation;
    source.rotation = 0;

    // Unpack area
    Spine::AreaSet areas(annotation->areas());
    if (areas.size() > 0) {
        source.area = *areas.begin();
    }

    // Unpack rotation
    bool ok = true;
    source.rotation = Papyro::qStringFromUnicode(annotation->getFirstProperty("property:rotation")).toInt(&ok);

    initialise();
}

TablificationDialog::TablificationDialog(Spine::DocumentHandle document, const Spine::Area & area)
    : QWidget(0), hasChanged(true)
{
    source.document = document;
    source.area = area;
    source.rotation = 0;

    initialise();
}

bool TablificationDialog::isValid() const
{
    return source.area.page >= 0;
}

void TablificationDialog::initialise()
{
    resolution = 0.0;
    offset = QPoint(0, 0);
    visibleOffset = QPoint(0, 0);
    scrolled = false;
    zoom = 1.0;

    // Work out whether we should default to another rotation
    {
        QMap< int, int > rotations;

        // Get text from document
        Spine::CursorHandle cursor(source.document->newCursor(source.area.page));
        while (/* const Spine::Line * line = */ cursor->line()) {
            while (const Spine::Word * word = cursor->word()) {
                Spine::BoundingBox bb(word->boundingBox());
                double x, y;
                bb.getCenter(&x, &y);
                if (source.area.boundingBox.contains(x, y)) {
                    int rotation = word->rotation();
                    rotations.insert(rotation, rotations.value(rotation, 0) + 1);
                }
                cursor->nextWord(Spine::WithinLine);
            }
            cursor->nextLine(Spine::WithinPage);
        }

        QMap< int, int > frequencies;
        for (int i = 0; i < 4; ++i) {
            if (rotations.contains(i)) {
                frequencies[rotations[i]] = i;
            }
        }

        if (!rotations.isEmpty()) {
            source.rotation = (-*--frequencies.end()) % 4;
        }
    }

    setFocusPolicy(Qt::StrongFocus);
    setFocus(Qt::OtherFocusReason);

    slideLayout = new Utopia::SlideLayout(this);
    connect(slideLayout, SIGNAL(widgetChanged(QWidget *)), this, SLOT(onSlide(QWidget *)));

    // Grid pane
    gridPane = new QWidget;
    gridPane->setWindowTitle("Draw rows and columns...");
    gridPane->setObjectName("gridPane");
    slideLayout->addWidget(gridPane, "grid");
    {
        gridLayout = new QGridLayout(gridPane);
        gridLayout->setSpacing(0);
        gridLayout->setContentsMargins(10, 10, 10, 10);

        rotateLeftButton = new QPushButton("CCW");
        rotateLeftButton->setObjectName("rotateLeftButton");
        connect(rotateLeftButton, SIGNAL(clicked()), this, SLOT(onRotateLeftPressed()));
        rotateRightButton = new QPushButton("CW");
        rotateRightButton->setObjectName("rotateRightButton");
        connect(rotateRightButton, SIGNAL(clicked()), this, SLOT(onRotateRightPressed()));

        horizontalHeader = new Graffiti::Header(Qt::Horizontal);
        horizontalHeader->setSections(new Graffiti::Sections(0, 1, this));
        horizontalHeader->setObjectName("horizontal_graffiti_header");
        verticalHeader = new Graffiti::Header(Qt::Vertical);
        verticalHeader->setSections(new Graffiti::Sections(0, 1, this));
        verticalHeader->setObjectName("vertical_graffiti_header");

        gridView = new Graffiti::GridView;
        gridView->setHorizontalHeader(horizontalHeader);
        gridView->setVerticalHeader(verticalHeader);
        gridView->installEventFilter(this);

        horizontalScrollBar = new QScrollBar(Qt::Horizontal);
        horizontalScrollBar->setSingleStep(5);
        verticalScrollBar = new QScrollBar(Qt::Vertical);
        verticalScrollBar->setSingleStep(5);
        zoomSlider = new QSlider(Qt::Horizontal);
        zoomSlider->setFixedWidth(100);
        zoomSlider->setMinimum(10);
        zoomSlider->setMaximum(30);

        imageLabel = new QWidget;
        imageLabel->installEventFilter(this);

        gridLayout->addWidget(horizontalHeader, 0, 1);
        gridLayout->setRowMinimumHeight(0, 30);
        gridLayout->addWidget(verticalHeader, 1, 0);
        gridLayout->setColumnMinimumWidth(0, 30);

        QStackedLayout * stack = new QStackedLayout;
        stack->setStackingMode(QStackedLayout::StackAll);
        stack->addWidget(imageLabel);
        stack->addWidget(gridView);

        gridLayout->addLayout(stack, 1, 1);
        gridLayout->addWidget(horizontalScrollBar, 2, 1);
        gridLayout->addWidget(verticalScrollBar, 1, 2);

        gridLayout->setRowStretch(1, 1);
        gridLayout->setColumnStretch(1, 1);

        QHBoxLayout * hLayout = new QHBoxLayout;
        hLayout->addStretch(1);
        hLayout->addWidget(new QLabel("Zoom:"));
        hLayout->addWidget(zoomSlider);
        hLayout->addSpacing(40);
        hLayout->addWidget(new QLabel("Rotate:"));
        hLayout->addWidget(rotateLeftButton);
        hLayout->addWidget(rotateRightButton);
        hLayout->addStretch(1);
        gridLayout->addLayout(hLayout, 3, 1);

        hLayout = new QHBoxLayout;
        hLayout->setSpacing(5);
        hLayout->setContentsMargins(0, 10, 0, 0);
        QLabel * label = new QLabel;
        label->setObjectName("error_label");
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        connect(this, SIGNAL(message(const QString &)), label, SLOT(setText(const QString &)));
        hLayout->addWidget(label, 1);
        Utopia::Spinner * spinner = new Utopia::Spinner;
        spinner->setFixedSize(32, 24);
        connect(this, SIGNAL(saving()), spinner, SLOT(start()));
        connect(this, SIGNAL(idle()), spinner, SLOT(stop()));
        hLayout->addWidget(spinner);
        QPushButton * cancelButton = new QPushButton("Cancel");
        connect(cancelButton, SIGNAL(clicked()), this, SLOT(onCancelPressed()));
        hLayout->addWidget(cancelButton);
        QPushButton * nextButton = new QPushButton(QString("Accept ") + QChar(8250));
        connect(nextButton, SIGNAL(clicked()), this, SLOT(onNextPressed()));
        hLayout->addWidget(nextButton);
        gridLayout->addLayout(hLayout, 4, 0, 1, 3);

        QSizeF size = QSizeF(source.area.boundingBox.width(), source.area.boundingBox.height());
        size.scale(600, 600, Qt::KeepAspectRatio);
        defaultResolution = 72.0 * qCeil(size.width()) / source.area.boundingBox.width();
        minimumResolution = defaultResolution;
        resolution = minimumResolution;

        setRotation(source.rotation);
        calculateObstacles();
        setHeaderSnapPoints();
        renderImage();

        connect(horizontalScrollBar, SIGNAL(actionTriggered(int)),
                this, SLOT(onHorizontalScrollBarActionTriggered(int)));
        connect(horizontalScrollBar, SIGNAL(valueChanged(int)),
                this, SLOT(onHorizontalScrollBarValueChanged(int)));
        connect(horizontalScrollBar, SIGNAL(rangeChanged(int, int)),
                this, SLOT(onHorizontalScrollBarRangeChanged(int, int)));

        connect(verticalScrollBar, SIGNAL(actionTriggered(int)),
                this, SLOT(onVerticalScrollBarActionTriggered(int)));
        connect(verticalScrollBar, SIGNAL(valueChanged(int)),
                this, SLOT(onVerticalScrollBarValueChanged(int)));
        connect(verticalScrollBar, SIGNAL(rangeChanged(int, int)),
                this, SLOT(onVerticalScrollBarRangeChanged(int, int)));

        connect(zoomSlider, SIGNAL(valueChanged(int)),
                this, SLOT(onZoomSliderValueChanged(int)));
    }

    // Grid pane
    tablePane = new QWidget;
    tablePane->setWindowTitle("Edit the table...");
    tablePane->setObjectName("tablePane");
    slideLayout->addWidget(tablePane, "table");
    {
        QGridLayout * layout = new QGridLayout(tablePane);
        layout->setSpacing(1);
        layout->setContentsMargins(10, 10, 10, 10);

        QHBoxLayout * hLayout = new QHBoxLayout;
        hLayout->setSpacing(5);
        hLayout->setContentsMargins(0, 10, 0, 0);
        QPushButton * backButton = new QPushButton(QChar(8249) + QString(" Redraw"));
        connect(backButton, SIGNAL(clicked()), this, SLOT(onBackPressed()));
        hLayout->addWidget(backButton);
        QLabel * label = new QLabel;
        label->setObjectName("error_label");
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        connect(this, SIGNAL(message(const QString &)), label, SLOT(setText(const QString &)));
        hLayout->addWidget(label, 1);
        Utopia::Spinner * spinner = new Utopia::Spinner;
        spinner->setFixedSize(32, 24);
        connect(this, SIGNAL(saving()), spinner, SLOT(start()));
        connect(this, SIGNAL(idle()), spinner, SLOT(stop()));
        hLayout->addWidget(spinner);
        QPushButton * exportButton = new QPushButton("Export CSV...");
        connect(exportButton, SIGNAL(clicked()), this, SLOT(onExportPressed()));
        hLayout->addWidget(exportButton);
        saveButton = new QPushButton("Save");
        connect(saveButton, SIGNAL(clicked()), this, SLOT(onSavePressed()));
        hLayout->addWidget(saveButton);
        QPushButton * closeButton = new QPushButton("Close");
        connect(closeButton, SIGNAL(clicked()), this, SLOT(onClosePressed()));
        hLayout->addWidget(closeButton);

        layout->addLayout(hLayout, 1, 0);

        table = new QTableWidget;
        table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        layout->addWidget(table, 0, 0);

        layout->setRowStretch(0, 1);
    }

    horizontalHeader->setSnapToGuides(true);
    verticalHeader->setSnapToGuides(true);

    slideLayout->push("grid", false);

    adjustSize();

    if (source.annotation) {
        // Unpack section boundaries
        QList< double > horizontalBoundaries;
        foreach (const QString & boundary, Papyro::qStringFromUnicode(source.annotation->getFirstProperty("property:horizontalBoundaries")).split(' ')) {
            horizontalBoundaries << boundary.toDouble();
        }
        horizontalHeader->sections()->setBoundaries(horizontalBoundaries);
        QList< double > verticalBoundaries;
        foreach (const QString & boundary, Papyro::qStringFromUnicode(source.annotation->getFirstProperty("property:verticalBoundaries")).split(' ')) {
            verticalBoundaries << boundary.toDouble();
        }
        verticalHeader->sections()->setBoundaries(verticalBoundaries);

        onNextPressed();
    }

    connect(horizontalHeader->sections(), SIGNAL(boundariesChanged()),
            this, SLOT(setChanged()));
    connect(verticalHeader->sections(), SIGNAL(boundariesChanged()),
            this, SLOT(setChanged()));
    connect(table->model(), SIGNAL(modelReset()),
            this, SLOT(setChanged()));
    connect(table->model(), SIGNAL(layoutChanged()),
            this, SLOT(setChanged()));
    connect(table->model(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
            this, SLOT(onTableDataChanged(const QModelIndex &, const QModelIndex &)));
    connect(table->model(), SIGNAL(columnsInserted(const QModelIndex &, int, int)),
            this, SLOT(onTableRowsOrColumnsChanged(const QModelIndex &, int, int)));
    connect(table->model(), SIGNAL(columnsRemoved(const QModelIndex &, int, int)),
            this, SLOT(onTableRowsOrColumnsChanged(const QModelIndex &, int, int)));
    connect(table->model(), SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            this, SLOT(onTableRowsOrColumnsChanged(const QModelIndex &, int, int)));
    connect(table->model(), SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
            this, SLOT(onTableRowsOrColumnsChanged(const QModelIndex &, int, int)));

    setChanged(hasChanged);
}

void TablificationDialog::onRotateLeftPressed()
{
    setUpdatesEnabled(false);
    setRotation((source.rotation - 1) % 4);
    swapSections();
    calculateObstacles();
    renderImage();
    setUpdatesEnabled(true);
}

void TablificationDialog::onRotateRightPressed()
{
    setRotation((source.rotation + 1) % 4);
    swapSections();
    calculateObstacles();
    renderImage();
    setUpdatesEnabled(true);
}

void TablificationDialog::calculateObstacles()
{
    // Get text from document
    obstacles.clear();
    Spine::CursorHandle cursor(source.document->newCursor(source.area.page));
    while (const Spine::Word * word = cursor->word()) {
        Spine::BoundingBox bb(word->boundingBox());
        QRectF rect(logicalRectForBoundingBox(bb));
        if (rect.intersects(QRectF(0, 0, 1, 1))) {
            rect = source.logicalTransform.mapRect(rect);
            if (source.rotation == 1 || source.rotation == 2) {
                rect.moveLeft(1-rect.right());
            }
            if (source.rotation == 2 || source.rotation == 3) {
                rect.moveTop(1-rect.bottom());
            }
            obstacles.append(rect);
        }
        cursor->nextWord(Spine::WithinPage);
    }
    gridView->setObstacles(obstacles);
}

void TablificationDialog::renderImage()
{
    Spine::Image sImage = source.document->renderArea(source.area, effectiveResolution());
    QImage image(Papyro::qImageFromSpineImage(&sImage));
    image = image.transformed(QTransform().rotate(source.rotation * 90));
    source.image = QPixmap::fromImage(image);
    repositionImage();
}

TablificationDialog::~TablificationDialog()
{}

void TablificationDialog::setHeaderSnapPoints()
{
    typedef std::map< double, int > PointMap;
    typedef QVector< double > SnapList;

    // Set snap points in headers
    if (horizontalHeader) { // Horizontally
        SnapList snapPoints;

        // Compile obstacles endpoints
        PointMap points;
        points[0.0] = -1;
        points[1.0] = 1;
        foreach (const QRectF & obstacle, obstacles) {
            double left = obstacle.left();
            PointMap::const_iterator foundLeft = points.find(left);
            int deltaLeft = (foundLeft == points.end() ? 0 : foundLeft->second) + 1;
            points[left] = deltaLeft;
            double right = obstacle.right();
            PointMap::const_iterator foundRight = points.find(right);
            int deltaRight = (foundRight == points.end() ? 0 : foundRight->second) - 1;
            points[right] = deltaRight;
        }

        // Cycle through endpoints and find gaps
        int depth = 1;
        double previous = 0.0;
        PointMap::const_iterator iter = points.begin();
        PointMap::const_iterator end = points.end();
        for (; iter != end; ++iter) {
            if (depth == 0 && previous > 0.0 && iter->first < 1.0) {
                snapPoints.push_back((iter->first + previous) / 2.0);
            }
            depth += iter->second;
            previous = iter->first;
        }

        horizontalHeader->setSnapValues(snapPoints);
    }

    if (verticalHeader) { // Vertically
        SnapList snapPoints;

        // Compile obstacles endpoints
        PointMap points;
        points[0.0] = -1;
        points[1.0] = 1;
        foreach (const QRectF & obstacle, obstacles) {
            double top = obstacle.top();
            PointMap::const_iterator foundTop = points.find(top);
            int deltaTop = (foundTop == points.end() ? 0 : foundTop->second) + 1;
            points[top] = deltaTop;
            double bottom = obstacle.bottom();
            PointMap::const_iterator foundBottom = points.find(bottom);
            int deltaBottom = (foundBottom == points.end() ? 0 : foundBottom->second) - 1;
            points[bottom] = deltaBottom;
        }

        // Cycle through endpoints and find gaps
        int depth = 1;
        double previous = 0.0;
        PointMap::const_iterator iter = points.begin();
        PointMap::const_iterator end = points.end();
        for (; iter != end; ++iter) {
            if (depth == 0 && previous > 0.0 && iter->first < 1.0) {
                snapPoints.push_back((iter->first + previous) / 2.0);
            }
            depth += iter->second;
            previous = iter->first;
        }

        verticalHeader->setSnapValues(snapPoints);
    }
}

void TablificationDialog::swapSections()
{
    qSwap(horizontalHeader, verticalHeader);
    horizontalHeader->hide();
    verticalHeader->hide();
    gridLayout->addWidget(horizontalHeader, 0, 1);
    gridLayout->addWidget(verticalHeader, 1, 0);
    horizontalHeader->setOrientation(Qt::Horizontal);
    verticalHeader->setOrientation(Qt::Vertical);
    horizontalHeader->setReversed(source.rotation == 1 || source.rotation == 2);
    verticalHeader->setReversed(source.rotation == 2 || source.rotation == 3);
    gridView->setHorizontalHeader(horizontalHeader);
    gridView->setVerticalHeader(verticalHeader);
    horizontalHeader->show();
    verticalHeader->show();
}

double TablificationDialog::effectiveResolution()
{
    return qMax(minimumResolution, resolution);
}

void TablificationDialog::calculateMinimumResolution()
{
    double oldEffectiveResolution = effectiveResolution();

    QSizeF size = QSizeF(source.area.boundingBox.width(), source.area.boundingBox.height());
    if (source.rotation % 2) {
        size = QSizeF(size.height(), size.width());
    }
    size.scale(gridView->size(), Qt::KeepAspectRatio);
    if (source.rotation % 2) {
        size = QSizeF(size.height(), size.width());
    }
    minimumResolution = 72.0 * qCeil(size.width()) / source.area.boundingBox.width();

    if (oldEffectiveResolution != effectiveResolution()) {
        renderImage();
    } else {
        repositionImage();
    }
}

bool TablificationDialog::eventFilter(QObject * obj, QEvent * event)
{
    if (obj == gridView) {
        if (event->type() == QEvent::Resize) {
            calculateMinimumResolution();
        } else if (event->type() == QEvent::Wheel) {
            QWheelEvent * we = static_cast< QWheelEvent * >(event);
            QWidget * target = 0;
            if (we->modifiers() & Qt::ControlModifier) {
                target = zoomSlider;
            } else {
                target = (we->orientation() == Qt::Horizontal ? horizontalScrollBar : verticalScrollBar);
            }
            QCoreApplication::sendEvent(target, we);
        }
    } else if (obj == imageLabel) {
        if (event->type() == QEvent::Paint) {
            QPainter painter(imageLabel);
            painter.setPen(Qt::NoPen);
            painter.setBrush(Qt::white);
            painter.drawRect(imageLabel->rect());
            painter.setBrush(QBrush(QColor(200, 200, 200), Qt::DiagCrossPattern));
            painter.drawRect(imageLabel->rect());
            painter.drawPixmap(visibleOffset, source.image);
        }
    }

    return QWidget::eventFilter(obj, event);
}

void TablificationDialog::repositionImage()
{
    // Decide what to do with resolutions and offsets according to the size of
    // of the grid view

    // new visible size of grid
    QSize gridSize = gridView->size();
    // coordinate in qimage space of bottom right
    QPoint bottomRight = QPoint(gridSize.width(), gridSize.height()) - offset;

    // if bottomRight is outside the qimage size, move image to compensate
    visibleOffset = offset;
    if (bottomRight.x() > source.image.width()) {
        visibleOffset.rx() += (bottomRight.x() - source.image.width());
    }
    if (bottomRight.y() > source.image.height()) {
        visibleOffset.ry() += (bottomRight.y() - source.image.height());
    }

    // if topLeft is outside the qimage size, move image to compensate
    if (visibleOffset.x() > 0) {
        visibleOffset.rx() = (gridSize.width() - source.image.width()) / 2;
        horizontalScrollBar->setRange(-visibleOffset.x(), -visibleOffset.x());
    } else {
        horizontalScrollBar->setRange(0, (source.image.width() - gridSize.width()));
    }
    horizontalScrollBar->setValue(-visibleOffset.x());
    horizontalScrollBar->setPageStep((horizontalScrollBar->maximum() - horizontalScrollBar->minimum()) / 10);

    if (visibleOffset.y() > 0) {
        visibleOffset.ry() = (gridSize.height() - source.image.height()) / 2;
        verticalScrollBar->setRange(-visibleOffset.y(), -visibleOffset.y());
    } else {
        verticalScrollBar->setRange(0, (source.image.height() - gridSize.height()));
    }
    verticalScrollBar->setValue(-visibleOffset.y());
    verticalScrollBar->setPageStep((verticalScrollBar->maximum() - verticalScrollBar->minimum()) / 10);

    // What's the visible rectangle in qimage coordinates?
    QRect visibleRect(-visibleOffset, gridSize);
    // Convert to logical coordinates
    QRectF visibleRectF(QPointF(visibleRect.left() / (double) source.image.width(),
                                visibleRect.top() / (double) source.image.height()),
                        QSizeF(visibleRect.width() / (double) source.image.width(),
                               visibleRect.height() / (double) source.image.height()));

    // Set header ranges
    horizontalHeader->setVisibleRange(visibleRectF.left(), visibleRectF.right());
    verticalHeader->setVisibleRange(visibleRectF.top(), visibleRectF.bottom());
    gridView->setViewportRect(visibleRectF);

    update();
}

void TablificationDialog::keyPressEvent(QKeyEvent * event)
{
    if (event->key() == Qt::Key_Shift) {
        horizontalHeader->setSnapToGuides(false);
        verticalHeader->setSnapToGuides(false);
    }
    QWidget::keyPressEvent(event);
}

void TablificationDialog::keyReleaseEvent(QKeyEvent * event)
{
    if (event->key() == Qt::Key_Shift) {
        horizontalHeader->setSnapToGuides(true);
        verticalHeader->setSnapToGuides(true);
    }
    QWidget::keyReleaseEvent(event);
}

QRectF TablificationDialog::logicalRectForArea(const Spine::Area & area)
{
    if (area.page == source.area.page) {
        return logicalRectForBoundingBox(area.boundingBox);
    }

    return QRectF();
}

QRectF TablificationDialog::logicalRectForBoundingBox(const Spine::BoundingBox & boundingBox)
{
    QRectF rect(boundingBox.x1,
                boundingBox.y1,
                boundingBox.width(),
                boundingBox.height());
    Spine::Area area(source.area);
    rect.translate(-area.boundingBox.x1, -area.boundingBox.y1);
    double hScaling = 1.0 / area.boundingBox.width();
    double vScaling = 1.0 / area.boundingBox.height();
    return QRectF(rect.left() * hScaling, rect.top() * vScaling,
                  rect.width() * hScaling, rect.height() * vScaling);
}

void TablificationDialog::onBackPressed()
{
    if (slideLayout->top() == tablePane) {
        slideLayout->pop();
    }
}

void TablificationDialog::onCancelPressed()
{
    onClosePressed();
}

void TablificationDialog::onClosePressed()
{
    bool shouldClose = true;
    bool shouldSave = false;

    if (hasChanged) {
        static QString message(
            "There are unapplied changes to this table. You can "
            "choose to save or discard the changes, or cancel the "
            "request to check things over."
            );

        QMessageBox::Button button = QMessageBox::warning(this,
            "Changes to apply...", message,
            QMessageBox::Cancel | QMessageBox::Discard | QMessageBox::Apply,
            QMessageBox::Apply);

        shouldClose = (button != QMessageBox::Cancel);
        shouldSave = (button == QMessageBox::Apply);
    }

    if (shouldSave) {
        onSavePressed();
    }

    if (shouldClose) {
        close();
    }
}

void TablificationDialog::onExportPressed()
{
    if (QAbstractItemModel * model = table->model()) {
        QString csvFileName = QFileDialog::getSaveFileName(this, "Choose where to save CSV...", QString(), "CSV Files (*.csv)");
        if (!csvFileName.isEmpty()) {
            QString csv;
            for (int r = 0; r < model->rowCount(); ++r) {
                for (int c = 0; c < model->columnCount(); ++c) {
                    if (c != 0) { csv += ","; }
                    QModelIndex i = model->index(r, c);
                    QString cell = model->data(i).toString().replace('"', "\"\"");
                    csv += "\"" + cell + "\"";
                }
                csv += "\n";
            }

            // Save file
            if (!csv.isEmpty()) {
                QFile csvFile(csvFileName);
                if (csvFile.open(QIODevice::WriteOnly)) {
                    csvFile.write(csv.toUtf8());
                    csvFile.close();
                    QMessageBox::information(this, "Success", "CSV file saved successfully");
                } else {
                    // FIXME what if I can't open the file?
                }
            } else {
                // FIXME what if empty?
            }
        }
    }
}

void TablificationDialog::onHorizontalScrollBarActionTriggered(int value)
{
    scrolled = true;
}

void TablificationDialog::onHorizontalScrollBarRangeChanged(int from, int to)
{
    horizontalScrollBar->setEnabled(to != from);
}

void TablificationDialog::onHorizontalScrollBarValueChanged(int value)
{
    if (scrolled) {
        offset.rx() = -value;
        scrolled = false;
    }
    repositionImage();
}

void TablificationDialog::onNextPressed()
{
    if (slideLayout->top() == gridPane) {
        // Scrape document into table

        // Dimensions
        int rows = gridView->grid()->rowCount();
        int columns = gridView->grid()->columnCount();
        table->setColumnCount(columns);
        table->setRowCount(rows);

        for (int r = 0; r < gridView->grid()->rowCount(); ++r) {
            for (int c = 0; c < gridView->grid()->columnCount(); ++c) {
                Graffiti::Section vSec(verticalHeader->sections()->sectionAt(r));
                Graffiti::Section hSec(horizontalHeader->sections()->sectionAt(c));
                QRectF cellRect(hSec.offset, vSec.offset, hSec.size, vSec.size);
                cellRect = source.logicalTransform.inverted().mapRect(cellRect);

                // Translate cell to page coords
                Spine::Area area(source.area.page,
                                 0,
                                 Spine::BoundingBox(source.area.boundingBox.x1 + cellRect.left() * source.area.boundingBox.width(),
                                                    source.area.boundingBox.y1 + cellRect.top() * source.area.boundingBox.height(),
                                                    source.area.boundingBox.x1 + cellRect.right() * source.area.boundingBox.width(),
                                                    source.area.boundingBox.y1 + cellRect.bottom() * source.area.boundingBox.height()));

                // Get text from document
                Spine::CursorHandle cursor(source.document->newCursor(source.area.page));
                QString content;
                while (/* const Spine::Line * line = */ cursor->line()) {
                    while (const Spine::Word * word = cursor->word()) {
                        Spine::BoundingBox bb(word->boundingBox());
                        QRectF rect(logicalRectForBoundingBox(bb));
                        if (cellRect.contains(rect.center())) {
                            content += Papyro::qStringFromUnicode(word->text());
                            if (word->spaceAfter()) {
                                content += " ";
                            }
                        }
                        cursor->nextWord(Spine::WithinLine);
                    }
                    if (content.endsWith(" ")) {
                        content.chop(1);
                    }
                    content += "\n";
                    cursor->nextLine(Spine::WithinPage);
                }

                QTableWidgetItem * item = table->item(r, c);
                if (item == 0) {
                    item = new QTableWidgetItem;
                    table->setItem(r, c, item);
                }
                item->setText(content.trimmed());
            }
        }

        table->resizeRowsToContents();
        table->resizeColumnsToContents();
        table->resizeRowsToContents();
        table->resizeColumnsToContents();

        slideLayout->push("table", sender());
    }
}

void TablificationDialog::onSavePressed()
{
    // Save to document
    QString encodedHorizontalBoundaries;
    foreach (double boundary, horizontalHeader->sections()->boundaries()) {
        if (!encodedHorizontalBoundaries.isEmpty()) {
            encodedHorizontalBoundaries += " ";
        }
        encodedHorizontalBoundaries += QString("%1").arg(boundary);
    }
    QString encodedVerticalBoundaries;
    foreach (double boundary, verticalHeader->sections()->boundaries()) {
        if (!encodedVerticalBoundaries.isEmpty()) {
            encodedVerticalBoundaries += " ";
        }
        encodedVerticalBoundaries += QString("%1").arg(boundary);
    }
    Spine::AnnotationHandle annotation(source.annotation);
    if (!annotation) {
        annotation = Spine::AnnotationHandle(new Spine::Annotation);
        annotation->setProperty("concept", "Table");
        annotation->addArea(source.area);
    }
    annotation->removeProperty("property:horizontalBoundaries");
    annotation->setProperty("property:horizontalBoundaries", Papyro::unicodeFromQString(encodedHorizontalBoundaries));
    annotation->removeProperty("property:verticalBoundaries");
    annotation->setProperty("property:verticalBoundaries", Papyro::unicodeFromQString(encodedVerticalBoundaries));

    int oldRevision = Papyro::qStringFromUnicode(annotation->getFirstProperty("revision")).toInt();

    // Save the annotation
    emit saving();
    emit message("<span style=\"color: grey\">Saving...</span>");
    source.document->addAnnotation(annotation, "PersistQueue");
    emit idle();

    // Check the save worked out
    bool hasId = annotation->hasProperty("id");
    int newRevision = Papyro::qStringFromUnicode(annotation->getFirstProperty("revision")).toInt();
    if (hasId && newRevision > oldRevision) {
        setChanged(false);
        emit message("<span style=\"color: grey\">Saved</span>");
    } else {
        emit message("<span style=\"color: red\">Error saving table</span>");
    }
}

void TablificationDialog::onSlide(QWidget * widget)
{
    setWindowTitle(widget ? widget->windowTitle() : "");
}

void TablificationDialog::onTableDataChanged(const QModelIndex & /*topLeft*/, const QModelIndex & /*bottomRight*/)
{
    setChanged(true);
}

void TablificationDialog::onTableRowsOrColumnsChanged(const QModelIndex & /*parent*/, int /*start*/, int /*end*/)
{
    setChanged(true);
}

void TablificationDialog::onVerticalScrollBarActionTriggered(int value)
{
    scrolled = true;
}

void TablificationDialog::onVerticalScrollBarRangeChanged(int from, int to)
{
    verticalScrollBar->setEnabled(to != from);
}

void TablificationDialog::onVerticalScrollBarValueChanged(int value)
{
    if (scrolled) {
        offset.ry() = -value;
        scrolled = false;
    }
    repositionImage();
}

void TablificationDialog::onZoomSliderValueChanged(int value)
{
    setResolution(defaultResolution * (value / 10.0));
}

void TablificationDialog::setChanged(bool changed)
{
    hasChanged = changed;
    saveButton->setEnabled(changed);
}

void TablificationDialog::setOffset(const QPoint & offset)
{
    this->offset = offset;
}

void TablificationDialog::setResolution(double resolution)
{
    this->resolution = resolution;
    renderImage();
}

void TablificationDialog::setRotation(int rotation)
{
    source.rotation = rotation;

    // Cache rotated widths / heights
    if ((rotation % 2) == 0) {
        source.transformed.width = source.area.boundingBox.width();
        source.transformed.height = source.area.boundingBox.height();
    } else {
        source.transformed.width = source.area.boundingBox.height();
        source.transformed.height = source.area.boundingBox.width();
    }

    // Calculate transformation matrices
    QTransform r;
    r.rotate(rotation * 90);
    source.transform = QImage::trueMatrix(r, source.area.boundingBox.width(), source.area.boundingBox.height());
    source.logicalTransform = QImage::trueMatrix(r, 1, 1);

    // Work out minimum size of the grid view
    QSizeF size = QSizeF(source.transformed.width, source.transformed.height);
    size.scale(600, 600, Qt::KeepAspectRatio);
    gridView->setMinimumSize(size.toSize());

    calculateMinimumResolution();
}





/////////////////////////////////////////////////////////////////////////////////////////
/// Tablify

void Tablify::activate(Spine::DocumentHandle document, Spine::AnnotationSet annotations, const QPoint & globalPos)
{
    if (annotations.size() > 0) {
        Spine::AreaSet areas((*annotations.begin())->areas());
        if (areas.size() > 0) {
            TablificationDialog * dialog = new TablificationDialog(document, *annotations.begin());
            dialog->setAttribute(Qt::WA_DeleteOnClose, true);
            dialog->show();
        }
    }
}

bool Tablify::canActivate(Spine::DocumentHandle document, Spine::AnnotationHandle annotation) const
{
    return annotation->getFirstProperty("concept") == "Table" &&
           annotation->hasProperty("property:horizontalBoundaries");
}

int Tablify::category() const
{
    return 10;
}

QIcon Tablify::icon() const
{
    return generateFromMonoPixmap(QPixmap(":/processors/tabling/icon.png"));
}

void Tablify::processSelection(Spine::DocumentHandle document, Spine::CursorHandle cursor, const QPoint & globalPos)
{
    Spine::AreaSet areas = document->areaSelection();
    // Do stuff to the area(s)

    if (!areas.empty()) {
        TablificationDialog * dialog = new TablificationDialog(document, *areas.begin());
        dialog->setAttribute(Qt::WA_DeleteOnClose, true);
        dialog->show();
    }
}

QList< boost::shared_ptr< Papyro::SelectionProcessor > > Tablify::selectionProcessors(Spine::DocumentHandle document, Spine::CursorHandle cursor)
{
    QList< boost::shared_ptr< Papyro::SelectionProcessor > > list;
    if (hasAreaSelection(document, cursor)) {
        list << boost::shared_ptr< Papyro::SelectionProcessor >(new Tablify);
    }
    return list;
}

QString Tablify::title() const
{
    return "Make Table...";
}

QString Tablify::title(Spine::DocumentHandle document, Spine::AnnotationHandle annotation) const
{
    return "Open Table...";
}

int Tablify::weight() const
{
    return 10;
}




