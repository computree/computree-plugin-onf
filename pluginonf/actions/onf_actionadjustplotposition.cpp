/****************************************************************************
 Copyright (C) 2010-2012 the Office National des Forêts (ONF), France
                         All rights reserved.

 Contact : alexandre.piboule@onf.fr

 Developers : Alexandre PIBOULE (ONF)

 This file is part of PluginONF library.

 PluginONF is free library: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 PluginONF is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with PluginONF.  If not, see <http://www.gnu.org/licenses/lgpl.html>.
*****************************************************************************/

#include "actions/onf_actionadjustplotposition.h"
#include "ct_global/ct_context.h"
#include "ct_iterator/ct_pointiterator.h"
#include "ct_color.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QIcon>
#include <QPainter>
#include <QDebug>

#include "math.h"


ONF_ActionAdjustPlotPosition_dataContainer::ONF_ActionAdjustPlotPosition_dataContainer()
{
    _transX = 0;
    _transY = 0;
}

ONF_ActionAdjustPlotPosition::ONF_ActionAdjustPlotPosition(ONF_ActionAdjustPlotPosition_dataContainer *dataContainer) : CT_AbstractActionForGraphicsView()
{

    _dataContainer = dataContainer;
    _drawManager = new ONF_AdjustPlotPositionCylinderDrawManager(tr("Tree"));
    _drawManager->setColor(QColor(0, 190, 255));

    _minZ = std::numeric_limits<double>::max();
    _maxZ = -std::numeric_limits<double>::max();
    _rangeZ = 1;
    _minI = std::numeric_limits<double>::max();
    _maxI = -std::numeric_limits<double>::max();
    _rangeI = 1;
    _colorizeByIntensity = false;

    // GRAY
    QLinearGradient gr = QLinearGradient(0, 0, 1, 0);
    gr.setColorAt(0, Qt::black);
    gr.setColorAt(1, Qt::white);
    _gradientGrey.constructFromQGradient(gr);

    // HOT
    gr = QLinearGradient(0, 0, 1, 0);
    gr.setColorAt(0, Qt::black);
    gr.setColorAt(0.25, Qt::red);
    gr.setColorAt(0.75, Qt::yellow);
    gr.setColorAt(1, Qt::white);
    _gradientHot.constructFromQGradient(gr);

    // Arcgis greenyellow - violet
    gr = QLinearGradient(0, 0, 1, 0);
    gr.setColorAt(0, Qt::green);
    gr.setColorAt(0.333, Qt::yellow);
    gr.setColorAt(0.666, Qt::red);
    gr.setColorAt(1, Qt::blue);
    _gradientRainbow.constructFromQGradient(gr);

}

ONF_ActionAdjustPlotPosition::~ONF_ActionAdjustPlotPosition()
{
    document()->removeAllItemDrawable();
    qDeleteAll(_cylinders);
    _cylinders.clear();
    delete _drawManager;
}

QString ONF_ActionAdjustPlotPosition::uniqueName() const
{
    return "ONF_ActionAdjustPlotPosition";
}

QString ONF_ActionAdjustPlotPosition::title() const
{
    return "Ajust Plot Position";
}

QString ONF_ActionAdjustPlotPosition::description() const
{
    return "Ajust Plot Position";
}

QIcon ONF_ActionAdjustPlotPosition::icon() const
{
    return QIcon(":/icons/select_rectangular.png");
}

QString ONF_ActionAdjustPlotPosition::type() const
{
    return CT_AbstractAction::TYPE_MODIFICATION;
}

void ONF_ActionAdjustPlotPosition::init()
{
    CT_AbstractActionForGraphicsView::init();

    if(nOptions() == 0)
    {
        // create the option widget if it was not already created
        ONF_ActionAdjustPlotPositionOptions *option = new ONF_ActionAdjustPlotPositionOptions(this);

        // add the options to the graphics view
        graphicsView()->addActionOptions(option);

        connect(option, SIGNAL(parametersChanged(double, double, bool, bool, double)), this, SLOT(update(double, double, bool, bool, double)));
        connect(option, SIGNAL(colorizationChanged(bool, int, int)), this, SLOT(updateColorization(bool, int, int)));

        // register the option to the superclass, so the hideOptions and showOptions
        // is managed automatically
        registerOption(option);

        for (int i = 0 ; i < _dataContainer->_positions.size() ; i++)
        {
            ONF_ActionAdjustPlotPosition_treePosition* pos = _dataContainer->_positions.at(i);

            Eigen::Vector3d center(pos->_x, pos->_y, pos->_height / 2.0);
            Eigen::Vector3d dir(0, 0, 1);
            CT_Cylinder* cyl = new CT_Cylinder(NULL, NULL, new CT_CylinderData(center,
                                                                               dir,
                                                                               pos->_dbh / 200.0,
                                                                               pos->_height));
            cyl->setBaseDrawManager(_drawManager);
            _cylinders.append(cyl);
            pos->_cyl = cyl;
        }

        _minZ = std::numeric_limits<double>::max();
        _maxZ = -std::numeric_limits<double>::max();

        _minI = std::numeric_limits<double>::max();
        _maxI = -std::numeric_limits<double>::max();

        for (int i = 0 ; i < _dataContainer->_scenes.size() ; i++)
        {
            CT_AbstractItemDrawableWithPointCloud* scene = (CT_AbstractItemDrawableWithPointCloud*) _dataContainer->_scenes.at(i);
            const CT_AbstractPointCloudIndex *pointCloudIndex = scene->getPointCloudIndex();

            CT_StdLASPointsAttributesContainer* LASAttributes = _dataContainer->_LASattributes.at(i);
            CT_AbstractPointAttributesScalar* attributeIntensity = NULL;
            CT_AbstractPointCloudIndex* pointCloudIndexLAS = NULL;

            if (LASAttributes != NULL)
            {
                attributeIntensity = (CT_AbstractPointAttributesScalar*)LASAttributes->pointsAttributesAt(CT_LasDefine::Intensity);
                pointCloudIndexLAS = (CT_AbstractPointCloudIndex*) attributeIntensity->getPointCloudIndex();
            }

            CT_PointIterator itP(pointCloudIndex);
            while(itP.hasNext())
            {
                const CT_Point &point = itP.next().currentPoint();
                size_t index = itP.currentGlobalIndex();

                if (point(2) > _maxZ) {_maxZ = point(2);}
                if (point(2) < _minZ) {_minZ = point(2);}

                if (LASAttributes != NULL)
                {
                    size_t localIndex = pointCloudIndexLAS->indexOf(index);

                    if (localIndex < pointCloudIndexLAS->size())
                    {
                        double intensity = attributeIntensity->dValueAt(localIndex);
                        if (intensity > _maxI) {_maxI = intensity;}
                        if (intensity < _minI) {_minI = intensity;}
                    }
                }
            }
            _rangeZ = _maxZ - _minZ;
            _rangeI = _maxI - _minI;

            document()->addItemDrawable(*scene);
        }

        GraphicsViewInterface* graphInterface = dynamic_cast<GraphicsViewInterface*>(document()->views().first());

        QColor col = Qt::black;
        graphInterface->getOptions().setBackgroudColor(col);
        graphInterface->getOptions().setPointSize(2);
        graphInterface->validateOptions();
        graphInterface->camera()->setType(CameraInterface::ORTHOGRAPHIC);

        document()->redrawGraphics(DocumentInterface::RO_WaitForConversionCompleted);

        graphInterface->camera()->fitCameraToVisibleItems();
        graphInterface->camera()->setOrientation(0.2, 0, 0, 0.95);
        colorizePoints(_gradientHot, 0, 100);
    }
}

void ONF_ActionAdjustPlotPosition::update(double x, double y, bool circles, bool fixedH, double h)
{
    //ONF_ActionAdjustPlotPositionOptions *option = (ONF_ActionAdjustPlotPositionOptions*)optionAt(0);

    _dataContainer->_transX += x;
    _dataContainer->_transY += y;
    _drawManager->setTranslation(_dataContainer->_transX, _dataContainer->_transY);
    _drawManager->setParameters(circles, fixedH, h);
    redrawOverlayAnd3D();
}

void ONF_ActionAdjustPlotPosition::updateColorization(bool colorizeByIntensity, int min, int max)
{
    _colorizeByIntensity = colorizeByIntensity;
    if (_colorizeByIntensity)
    {
        colorizePoints(_gradientGrey, min, max);
    } else {
        colorizePoints(_gradientHot, min, max);
    }
}

void ONF_ActionAdjustPlotPosition::colorizePoints(ONF_ColorLinearInterpolator &gradient, int min, int max)
{
    GraphicsViewInterface* graphInterface = dynamic_cast<GraphicsViewInterface*>(document()->views().first());

    for (int i = 0 ; i < _dataContainer->_scenes.size()  && _rangeZ > 0 ; i++)
    {
        CT_AbstractItemDrawableWithPointCloud* scene = (CT_AbstractItemDrawableWithPointCloud*) _dataContainer->_scenes.at(i);
        const CT_AbstractPointCloudIndex *pointCloudIndex = scene->getPointCloudIndex();

        CT_StdLASPointsAttributesContainer* LASAttributes = _dataContainer->_LASattributes.at(i);
        CT_AbstractPointAttributesScalar* attributeIntensity = NULL;
        CT_AbstractPointCloudIndex* pointCloudIndexLAS = NULL;

        if (_colorizeByIntensity && LASAttributes != NULL)
        {
            attributeIntensity = (CT_AbstractPointAttributesScalar*)LASAttributes->pointsAttributesAt(CT_LasDefine::Intensity);
            pointCloudIndexLAS = (CT_AbstractPointCloudIndex*) attributeIntensity->getPointCloudIndex();
        }

        CT_PointIterator itP(pointCloudIndex);
        while(itP.hasNext())
        {
            const CT_Point &point = itP.next().currentPoint();
            size_t index = itP.currentGlobalIndex();

            double minZ = _minZ + (double)min * _rangeZ / 100.0;
            double maxZ = _minZ + (double)max * _rangeZ / 100.0;
            if (minZ > maxZ) {minZ = maxZ;}
            double rangeZ = maxZ - minZ;

            double ratio = (point(2) - minZ) / rangeZ;

            if (point(2) < minZ) {ratio = 0;}
            if (point(2) > maxZ) {ratio = 1;}

            if (_colorizeByIntensity && LASAttributes != NULL)
            {
                size_t localIndex = pointCloudIndexLAS->indexOf(index);
                if (localIndex < pointCloudIndexLAS->size())
                {
                    double minI = _minI + (double)min * _rangeI / 100.0;
                    double maxI = _minI + (double)max * _rangeI / 100.0;
                    if (minI > maxI) {minI = maxI;}
                    double rangeI = maxI - minI;

                    double intensity = attributeIntensity->dValueAt(localIndex);
                    ratio = (intensity - minI) / rangeI;

                    if (intensity < minI) {ratio = 0;}
                    if (intensity > maxI) {ratio = 1;}
                }
            }


            CT_Color color(gradient.intermediateColor(ratio));
            graphInterface->setColorOfPoint(index, color);            
        }
    }
    redrawOverlayAnd3D();
    graphInterface->dirtyColorsOfItemDrawablesWithPoints();
}

void ONF_ActionAdjustPlotPosition::redrawOverlay()
{
    document()->redrawGraphics();
}

void ONF_ActionAdjustPlotPosition::redrawOverlayAnd3D()
{
    //document()->updateItems(_dataContainer->_scenes);
    setDrawing3DChanged();
    document()->redrawGraphics();
}

bool ONF_ActionAdjustPlotPosition::mousePressEvent(QMouseEvent *e)
{
    ONF_ActionAdjustPlotPositionOptions *option = (ONF_ActionAdjustPlotPositionOptions*)optionAt(0);

    if (e->buttons() & Qt::LeftButton)
    {
        if (e->modifiers()  & Qt::ShiftModifier)
        {
            GraphicsViewInterface* graphInterface = dynamic_cast<GraphicsViewInterface*>(document()->views().first());

            bool found = false;
            Eigen::Vector3d point = graphInterface->pointUnderPixel(e->pos(), found);

            if (found)
            {
                ONF_ActionAdjustPlotPosition_treePosition* pos = getPositionFromPoint(point);
            }
            //update();
            return true;
        }
    }

    return false;
}

bool ONF_ActionAdjustPlotPosition::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    return false;
}

bool ONF_ActionAdjustPlotPosition::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    return false;
}

bool ONF_ActionAdjustPlotPosition::wheelEvent(QWheelEvent *e)
{
    ONF_ActionAdjustPlotPositionOptions *option = (ONF_ActionAdjustPlotPositionOptions*)optionAt(0);

    if (e->modifiers()  & Qt::ControlModifier)
    {
        if (e->delta() > 0)
        {
        } else if (e->delta() < 0)
        {
        }
        //update();
        return true;
    } else if (e->modifiers()  & Qt::ShiftModifier)
    {
        if (e->delta() > 0)
        {
        } else if (e->delta() < 0)
        {
        }
        //update();
        return true;
    }

    return false;
}

bool ONF_ActionAdjustPlotPosition::keyPressEvent(QKeyEvent *e)
{
    Q_UNUSED(e);
    return false;

}

bool ONF_ActionAdjustPlotPosition::keyReleaseEvent(QKeyEvent *e)
{
    Q_UNUSED(e);
    return false;
}

void ONF_ActionAdjustPlotPosition::draw(GraphicsViewInterface &view, PainterInterface &painter)
{
    Q_UNUSED(view)

    for (int i = 0 ; i < _cylinders.size() ; i++)
    {
        _drawManager->draw(view, painter, *(_cylinders.at(i)));
    }
    painter.save();
    painter.restore();
}

void ONF_ActionAdjustPlotPosition::drawOverlay(GraphicsViewInterface &view, QPainter &painter)
{
    Q_UNUSED(view);
    Q_UNUSED(painter);
}

CT_AbstractAction* ONF_ActionAdjustPlotPosition::copy() const
{
    return new ONF_ActionAdjustPlotPosition(_dataContainer);
}

ONF_ActionAdjustPlotPosition_treePosition* ONF_ActionAdjustPlotPosition::getPositionFromPoint(Eigen::Vector3d &point)
{
    double minDist = std::numeric_limits<double>::max();
    ONF_ActionAdjustPlotPosition_treePosition* pos = NULL;
    for (int i = 0 ; i < _dataContainer->_positions.size() ; i++)
    {
        ONF_ActionAdjustPlotPosition_treePosition* treePos = _dataContainer->_positions.at(i);
        double x = treePos->_x + _dataContainer->_transX;
        double y = treePos->_y + _dataContainer->_transY;

        double distance = sqrt(pow(x - point(0), 2) + pow(y - point(1), 2));
        if (distance  < minDist)
        {
            minDist = distance;
            pos = treePos;
        }
    }
    return pos;
}

