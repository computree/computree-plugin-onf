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

#include "actions/onf_actiondefineheightlayer.h"

#include "ct_pointcloudindex/abstract/ct_abstractpointcloudindex.h"
#include "ct_iterator/ct_pointiterator.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QIcon>
#include <QPainter>
#include <QDebug>

#define DEFAULT_RESOLUTION 0.05

ONF_ActionDefineHeightLayer_gridContainer::ONF_ActionDefineHeightLayer_gridContainer()
{
    _densityGrid = NULL;
    _mnsGrid = NULL;
}

ONF_ActionDefineHeightLayer::ONF_ActionDefineHeightLayer(const CT_AbstractResult *result, const QString &densityGridModel, const QString &mnsGridModel, ONF_ActionDefineHeightLayer_gridContainer *gridContainer, const QList<CT_Scene*> &list, double xmin, double ymin, double zmin, double xmax, double ymax, double zmax) : CT_AbstractActionForGraphicsView()
{
    m_status = 0;

    _result = (CT_AbstractResult*)result;
    _densityGridModel = densityGridModel;
    _mnsGridModel = mnsGridModel;
    _gridContainer = gridContainer;

    _sceneList.append(list);

    _xmin = xmin;
    _ymin = ymin;
    _xmax = xmax;
    _ymax = ymax;
    _gridContainer->_zmin = zmin;
    _gridContainer->_zmax = zmax;

    _xrange = _xmax - _xmin;
    _yrange = _ymax - _ymin;

    _densityThreshold = 1;
    _resolution = DEFAULT_RESOLUTION;
    _previousResolution = 0;
    createGrids(_resolution);
}

void ONF_ActionDefineHeightLayer::createGrids(double res)
{
    if (_gridContainer->_densityGrid!=NULL) {delete _gridContainer->_densityGrid;}
    if (_gridContainer->_mnsGrid!=NULL) {delete _gridContainer->_mnsGrid;}

    _gridContainer->_densityGrid = CT_Grid2DXY<int>::createGrid2DXYFromXYCoords(_densityGridModel, _result, _xmin, _ymin, _xmax, _ymax, res, _gridContainer->_zmin, -1, -1);
    _gridContainer->_mnsGrid = CT_Grid2DXY<double>::createGrid2DXYFromXYCoords(_mnsGridModel, _result, _xmin, _ymin, _xmax, _ymax, res, _gridContainer->_zmin, -1, -1);

    _gridContainer->_densityGrid->setlevel(_gridContainer->_zmin);
    _gridContainer->_mnsGrid->setlevel(_gridContainer->_zmax);

}

QString ONF_ActionDefineHeightLayer::uniqueName() const
{
    return "ONF_ActionDefineHeightLayer";
}

QString ONF_ActionDefineHeightLayer::title() const
{
    return "Measure Crown";
}

QString ONF_ActionDefineHeightLayer::description() const
{
    return "Permet de mesurer des attributs d'un houppier";
}

QIcon ONF_ActionDefineHeightLayer::icon() const
{
    return QIcon(":/icons/show_all.png");
}

double ONF_ActionDefineHeightLayer::getZmin() const
{
    return _gridContainer->_zmin;
}

double ONF_ActionDefineHeightLayer::getZmax() const
{
    return _gridContainer->_zmax;
}

QString ONF_ActionDefineHeightLayer::type() const
{
    return CT_AbstractAction::TYPE_MODIFICATION;
}

void ONF_ActionDefineHeightLayer::init()
{
    CT_AbstractActionForGraphicsView::init();

    m_status = 0;

    if(nOptions() == 0)
    {
        // create the option widget if it was not already created
        ONF_ActionDefineHeightLayerOptions *option = new ONF_ActionDefineHeightLayerOptions(this, _gridContainer->_zmin, _gridContainer->_zmax, _resolution, _densityThreshold);

        // add the options to the graphics view
        graphicsView()->addActionOptions(option);

        connect(option, SIGNAL(redrawNeeded()), this, SLOT(updateGraphics()));
        connect(option, SIGNAL(zValuesChanged(double,double)), this, SLOT(updateZValues(double,double)));
        connect(option, SIGNAL(askForSideView()), this, SLOT(setSideView()));
        connect(option, SIGNAL(askForCrownProjectionComputing()), this, SLOT(computeCrownProjection()));
        connect(option, SIGNAL(updateResolution(double)), this, SLOT(updateResolution(double)));
        connect(option, SIGNAL(updateThreshold(int)), this, SLOT(updateThreshold(int)));

        // register the option to the superclass, so the hideOptions and showOptions
        // is managed automatically
        registerOption(option);

        computeCrownProjection();

        document()->redrawGraphics();
    }
}


bool ONF_ActionDefineHeightLayer::mousePressEvent(QMouseEvent *e)
{
    ONF_ActionDefineHeightLayerOptions *option = (ONF_ActionDefineHeightLayerOptions*)optionAt(0);

    if ((e->buttons() & Qt::LeftButton) && (e->modifiers()  & Qt::ControlModifier))
    {
            option->increaseSingleStep();
            return true;
    } else if ((e->buttons() & Qt::RightButton) && (e->modifiers()  & Qt::ControlModifier))
    {
        option->decreaseSingleStep();
        return true;
    }

    return false;
}

bool ONF_ActionDefineHeightLayer::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    return false;
}


bool ONF_ActionDefineHeightLayer::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    return false;
}

bool ONF_ActionDefineHeightLayer::wheelEvent(QWheelEvent *e)
{
    ONF_ActionDefineHeightLayerOptions *option = (ONF_ActionDefineHeightLayerOptions*)optionAt(0);

    if (e->modifiers()  & Qt::ControlModifier)
    {
        if (e->delta()>0)
        {
            option->increaseHValue();
        } else if (e->delta()<0)
        {
            option->decreaseHValue();
        }
        return true;
    }

    return false;
}

bool ONF_ActionDefineHeightLayer::keyPressEvent(QKeyEvent *e)
{
    ONF_ActionDefineHeightLayerOptions *option = (ONF_ActionDefineHeightLayerOptions*)optionAt(0);

    // Key A : pass in "Add" mode
    if((e->key() == Qt::Key_PageUp)
            && !e->isAutoRepeat())
    {
        option->chooseZmax();
        return true;
    } else if((e->key() == Qt::Key_PageDown)
              && !e->isAutoRepeat())
    {
        option->chooseZmin();
        return true;
    }
    return false;
}

bool ONF_ActionDefineHeightLayer::keyReleaseEvent(QKeyEvent *e)
{
    Q_UNUSED(e);
    return false;
}

void ONF_ActionDefineHeightLayer::updateGraphics()
{
    document()->redrawGraphics();
}

void ONF_ActionDefineHeightLayer::setSideView()
{
    CameraInterface* camera = graphicsView()->camera();

    camera->setRZ(0);

    document()->redrawGraphics();
}

void ONF_ActionDefineHeightLayer::updateZValues(double zmin, double zmax)
{
    _gridContainer->_zmin = zmin;
    _gridContainer->_zmax = zmax;
}

void ONF_ActionDefineHeightLayer::updateResolution(double resolution)
{
    _resolution = resolution;
}

void ONF_ActionDefineHeightLayer::updateThreshold(int threshold)
{
    _densityThreshold = threshold;
}


void ONF_ActionDefineHeightLayer::computeCrownProjection()
{

    if (_resolution != _previousResolution)
    {
        createGrids(_resolution);
        _previousResolution = _resolution;
    } else {
        _gridContainer->_densityGrid->initGridWithValue(_gridContainer->_densityGrid->NA());
        _gridContainer->_mnsGrid->initGridWithValue(_gridContainer->_mnsGrid->NA());
    }

    int size = _sceneList.size();
    for (int s = 0 ; s < size ; s++)
    {
        CT_PointIterator itP(_sceneList.at(s)->getPointCloudIndex());

        while(itP.hasNext())
        {
            const CT_Point &point = itP.next().currentPoint();

            if (point(2) >= _gridContainer->_zmin && point(2) <= _gridContainer->_zmax)
            {
                _gridContainer->_densityGrid->addValueAtXY(point(0), point(1), 1);
                _gridContainer->_mnsGrid->setMaxValueAtXY(point(0), point(1), point(2));
            }
        }
    }

    for (size_t i = 0 ; i < _gridContainer->_densityGrid->nCells() ; i++)
    {
        if (_gridContainer->_densityGrid->valueAtIndex(i) < _densityThreshold)
        {
            _gridContainer->_densityGrid->setValueAtIndex(i, _gridContainer->_densityGrid->NA());
            _gridContainer->_mnsGrid->setValueAtIndex(i, _gridContainer->_mnsGrid->NA());
        }
    }

    _gridContainer->_densityGrid->computeMinMax();
    _gridContainer->_mnsGrid->computeMinMax();

    updateGraphics();
}


void ONF_ActionDefineHeightLayer::draw(GraphicsViewInterface &view, PainterInterface &painter)
{
    ONF_ActionDefineHeightLayerOptions *option = (ONF_ActionDefineHeightLayerOptions*)optionAt(0);


    double zmin = option->getZmin();
    double zmax = option->getZmax();

    _gridContainer->_densityGrid->setlevel(zmin);
    _gridContainer->_mnsGrid->setlevel(zmax);

    _gridContainer->_densityGrid->draw(view, painter);
    _gridContainer->_mnsGrid->draw(view, painter);
}

void ONF_ActionDefineHeightLayer::drawOverlay(GraphicsViewInterface &view, QPainter &painter)
{
    Q_UNUSED(view);
    Q_UNUSED(painter);
}

CT_AbstractAction *ONF_ActionDefineHeightLayer::copy() const
{
    return new ONF_ActionDefineHeightLayer(_result, _densityGridModel, _mnsGridModel, _gridContainer, _sceneList, _xmin, _ymin, _gridContainer->_zmin, _xmax, _ymax, _gridContainer->_zmax);
}

