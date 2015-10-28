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

#include "actions/onf_actionslicepointcloud.h"
#include "ct_global/ct_context.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QIcon>
#include <QPainter>

#include "math.h"


ONF_ActionSlicePointCloud_dataContainer::ONF_ActionSlicePointCloud_dataContainer()
{
    _thickness = 0;
    _spacing = 0;
}

ONF_ActionSlicePointCloud::ONF_ActionSlicePointCloud(QList<CT_Scene *> *sceneList,
                                                   float xmin, float ymin, float zmin, float xmax, float ymax, float zmax,
                                                   ONF_ActionSlicePointCloud_dataContainer *dataContainer) : CT_AbstractActionForGraphicsView()
{
    _sceneList = sceneList;
    _xmin = xmin;
    _ymin = ymin;
    _zmin = zmin;

    _xmax = xmax;
    _ymax = ymax;
    _zmax = zmax;

    _xwidth = std::abs(_xmax - _xmin);
    _ywidth = std::abs(_ymax - _ymin);

    _dataContainer = dataContainer;
}

ONF_ActionSlicePointCloud::~ONF_ActionSlicePointCloud()
{
}

QString ONF_ActionSlicePointCloud::uniqueName() const
{
    return "ONF_ActionSlicePointCloud";
}

QString ONF_ActionSlicePointCloud::title() const
{
    return "Action Slicing";
}

QString ONF_ActionSlicePointCloud::description() const
{
    return "Action Slicing";
}

QIcon ONF_ActionSlicePointCloud::icon() const
{
    return QIcon(":/icons/select_rectangular.png");
}

QString ONF_ActionSlicePointCloud::type() const
{
    return CT_AbstractAction::TYPE_MODIFICATION;
}

void ONF_ActionSlicePointCloud::init()
{
    CT_AbstractActionForGraphicsView::init();

    if(nOptions() == 0)
    {
        // create the option widget if it was not already created
        ONF_ActionSlicePointCloudOptions *option = new ONF_ActionSlicePointCloudOptions(this);

        // add the options to the graphics view
        graphicsView()->addActionOptions(option);

        option->setThickness(_dataContainer->_thickness);
        option->setSpacing(_dataContainer->_spacing);

        connect(option, SIGNAL(parametersChanged()), this, SLOT(update()));

        // register the option to the superclass, so the hideOptions and showOptions
        // is managed automatically
        registerOption(option);
        for (int i = 0 ; i < _sceneList->size() ; i++)
        {
            document()->addItemDrawable(*(_sceneList->at(i)));
        }
        document()->redrawGraphics(DocumentInterface::RO_WaitForConversionCompleted);
        dynamic_cast<GraphicsViewInterface*>(document()->views().first())->camera()->fitCameraToVisibleItems();
    }
}

void ONF_ActionSlicePointCloud::update()
{
    ONF_ActionSlicePointCloudOptions *option = (ONF_ActionSlicePointCloudOptions*)optionAt(0);

    _dataContainer->_thickness = option->getThickness();
    _dataContainer->_spacing = option->getSpacing();

    redrawOverlayAnd3D();
}

void ONF_ActionSlicePointCloud::redrawOverlay()
{
    document()->redrawGraphics();
}

void ONF_ActionSlicePointCloud::redrawOverlayAnd3D()
{
    setDrawing3DChanged();
    document()->redrawGraphics();
}

bool ONF_ActionSlicePointCloud::mousePressEvent(QMouseEvent *e)
{
    ONF_ActionSlicePointCloudOptions *option = (ONF_ActionSlicePointCloudOptions*)optionAt(0);

    if ((e->modifiers() & Qt::ControlModifier) || (e->modifiers()  & Qt::ShiftModifier))
    {
        if (e->buttons() & Qt::LeftButton)
        {
            option->decreaseIncrement();
            update();
            return true;
        } else if (e->buttons() & Qt::RightButton)
        {
            option->increaseIncrement();
            update();
            return true;
        }
    }

    return false;
}

bool ONF_ActionSlicePointCloud::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    return false;
}

bool ONF_ActionSlicePointCloud::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    return false;
}

bool ONF_ActionSlicePointCloud::wheelEvent(QWheelEvent *e)
{
    ONF_ActionSlicePointCloudOptions *option = (ONF_ActionSlicePointCloudOptions*)optionAt(0);

    if (e->modifiers()  & Qt::ControlModifier)
    {
        if (e->delta() > 0)
        {
            option->setThickness(option->getThickness() + option->getIncrement());
        } else if (e->delta() < 0)
        {
            option->setThickness(option->getThickness() - option->getIncrement());
        }
        update();
        return true;
    } else if (e->modifiers()  & Qt::ShiftModifier)
    {
        if (e->delta() > 0)
        {
            option->setSpacing(option->getSpacing() + option->getIncrement());
        } else if (e->delta() < 0)
        {
            option->setSpacing(option->getSpacing() - option->getIncrement());
        }
        update();
        return true;
    }

    return false;
}

bool ONF_ActionSlicePointCloud::keyPressEvent(QKeyEvent *e)
{
    Q_UNUSED(e);
    return false;

}

bool ONF_ActionSlicePointCloud::keyReleaseEvent(QKeyEvent *e)
{
    Q_UNUSED(e);
    return false;
}

void ONF_ActionSlicePointCloud::draw(GraphicsViewInterface &view, PainterInterface &painter)
{
    Q_UNUSED(view)

    if (_dataContainer->_thickness == 0) {return;}

    painter.save();

    QColor oldColor = painter.getColor();
    painter.setColor(QColor(0,125,0,100));

    double z_current = _zmin;
    while (z_current <= _zmax)
    {
        painter.fillRectXY(Eigen::Vector2d(_xmin, _ymin), Eigen::Vector2d(_xmin+_xwidth, _ymin+_ywidth), z_current);
        painter.fillRectXY(Eigen::Vector2d(_xmin, _ymin), Eigen::Vector2d(_xmin+_xwidth, _ymin+_ywidth), z_current + _dataContainer->_thickness);
        painter.drawRectXZ(Eigen::Vector2d(_xmin, z_current), Eigen::Vector2d(_xmin+_xwidth, z_current+_dataContainer->_thickness), _ymin);
        painter.drawRectXZ(Eigen::Vector2d(_xmin, z_current), Eigen::Vector2d(_xmin+_xwidth, z_current+_dataContainer->_thickness), _ymax);
        painter.drawRectYZ(Eigen::Vector2d(_ymin, z_current), Eigen::Vector2d(_ymin+_ywidth, z_current+_dataContainer->_thickness), _xmin);
        painter.drawRectYZ(Eigen::Vector2d(_ymin, z_current), Eigen::Vector2d(_ymin+_ywidth, z_current+_dataContainer->_thickness), _xmax);

        z_current += _dataContainer->_thickness;
        z_current += _dataContainer->_spacing;
    }
    painter.setColor(oldColor);
    painter.restore();
}

void ONF_ActionSlicePointCloud::drawOverlay(GraphicsViewInterface &view, QPainter &painter)
{
    Q_UNUSED(view);
    Q_UNUSED(painter);
}

CT_AbstractAction* ONF_ActionSlicePointCloud::copy() const
{
    return new ONF_ActionSlicePointCloud(_sceneList, _xmin, _ymin, _zmin, _xmax, _ymax, _zmax, _dataContainer);
}
