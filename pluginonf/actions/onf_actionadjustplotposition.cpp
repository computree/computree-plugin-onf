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
#include <QMouseEvent>
#include <QKeyEvent>
#include <QIcon>
#include <QPainter>

#include "math.h"


ONF_ActionAdjustPlotPosition_dataContainer::ONF_ActionAdjustPlotPosition_dataContainer()
{
}

ONF_ActionAdjustPlotPosition::ONF_ActionAdjustPlotPosition(ONF_ActionAdjustPlotPosition_dataContainer *dataContainer) : CT_AbstractActionForGraphicsView()
{
    _dataContainer = dataContainer;
}

ONF_ActionAdjustPlotPosition::~ONF_ActionAdjustPlotPosition()
{
    qDeleteAll(_cylinders);
    _cylinders.clear();
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

        connect(option, SIGNAL(parametersChanged()), this, SLOT(update()));

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
            _cylinders.append(cyl);
            document()->addItemDrawable(*cyl);
            document()->setColor(cyl, Qt::red);
        }


        document()->redrawGraphics(DocumentInterface::RO_WaitForConversionCompleted);
        dynamic_cast<GraphicsViewInterface*>(document()->views().first())->camera()->fitCameraToVisibleItems();
    }
}

void ONF_ActionAdjustPlotPosition::update()
{
    ONF_ActionAdjustPlotPositionOptions *option = (ONF_ActionAdjustPlotPositionOptions*)optionAt(0);

    redrawOverlayAnd3D();
}

void ONF_ActionAdjustPlotPosition::redrawOverlay()
{
    document()->redrawGraphics();
}

void ONF_ActionAdjustPlotPosition::redrawOverlayAnd3D()
{
    setDrawing3DChanged();
    document()->redrawGraphics();
}

bool ONF_ActionAdjustPlotPosition::mousePressEvent(QMouseEvent *e)
{
    ONF_ActionAdjustPlotPositionOptions *option = (ONF_ActionAdjustPlotPositionOptions*)optionAt(0);

    if ((e->modifiers() & Qt::ControlModifier) || (e->modifiers()  & Qt::ShiftModifier))
    {
        if (e->buttons() & Qt::LeftButton)
        {
            update();
            return true;
        } else if (e->buttons() & Qt::RightButton)
        {
            update();
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
        update();
        return true;
    } else if (e->modifiers()  & Qt::ShiftModifier)
    {
        if (e->delta() > 0)
        {
        } else if (e->delta() < 0)
        {
        }
        update();
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

    painter.save();

    QColor oldColor = painter.getColor();
    painter.setColor(QColor(0,125,0,100));


    painter.setColor(oldColor);
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
