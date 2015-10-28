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

#include "actions/onf_actionpickitemsinlist.h"
#include "ct_global/ct_context.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QIcon>
#include <QPainter>
#include <limits>

#include <eigen/Eigen/Core>

#include "ct_math/ct_mathpoint.h"

ONF_ActionPickItemsInList::ONF_ActionPickItemsInList(const QList<CT_AbstractItemDrawable *> &itemList, QList<CT_AbstractItemDrawable *> *selectedItems, float maxDist) : CT_AbstractActionForGraphicsView()
{
    _itemList.append(itemList);
    _selectedItems = selectedItems;
    _maxDist = maxDist;
}

ONF_ActionPickItemsInList::~ONF_ActionPickItemsInList()
{
}

QString ONF_ActionPickItemsInList::uniqueName() const
{
    return "ONF_ActionPickItemsInList";
}

QString ONF_ActionPickItemsInList::title() const
{
    return tr("Picking d'item");
}

QString ONF_ActionPickItemsInList::description() const
{
    return tr("Picking d'item");
}

QIcon ONF_ActionPickItemsInList::icon() const
{
    return QIcon(":/icons/select_rectangular.png");
}

QString ONF_ActionPickItemsInList::type() const
{
    return CT_AbstractAction::TYPE_INFORMATION;
}

void ONF_ActionPickItemsInList::init()
{
    CT_AbstractActionForGraphicsView::init();

    if(nOptions() == 0)
    {
        // create the option widget if it was not already created
        ONF_ActionPickItemsInListOptions *option = new ONF_ActionPickItemsInListOptions(this);

        // add the options to the graphics view
        graphicsView()->addActionOptions(option);

        connect(option, SIGNAL(parametersChanged()), this, SLOT(redraw()));
        connect(option, SIGNAL(allClicked()), this, SLOT(selectAll()));
        connect(option, SIGNAL(noneClicked()), this, SLOT(selectNone()));

        // register the option to the superclass, so the hideOptions and showOptions
        // is managed automatically
        registerOption(option);

        document()->redrawGraphics();
    }
}

void ONF_ActionPickItemsInList::redraw()
{
    document()->redrawGraphics();
}

void ONF_ActionPickItemsInList::selectAll()
{
    _selectedItems->clear();
    _selectedItems->append(_itemList);
    redraw();
}

void ONF_ActionPickItemsInList::selectNone()
{
    _selectedItems->clear();
    redraw();
}


bool ONF_ActionPickItemsInList::mousePressEvent(QMouseEvent *e)
{
    _oldPos = e->pos();

    if (e->modifiers() & Qt::ShiftModifier)
    {
        _buttonsPressed = e->buttons();
        return true;
    } else  {
        _buttonsPressed = Qt::NoButton;
    }
    return false;
}

bool ONF_ActionPickItemsInList::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    return false;
}

bool ONF_ActionPickItemsInList::mouseReleaseEvent(QMouseEvent *e)
{   
    ONF_ActionPickItemsInListOptions *option = (ONF_ActionPickItemsInListOptions*)optionAt(0);

    QPoint point = e->pos() - _oldPos;

    if (point.manhattanLength() <= 3)
    {
        if (_buttonsPressed == Qt::LeftButton)
        {
            CT_AbstractItemDrawable* item = addItemToSelection(e->pos());
            if (item!=NULL && option->souldAutoCenterCamera())
            {
                graphicsView()->camera()->setCX(item->getCenterX());
                graphicsView()->camera()->setCY(item->getCenterY());
                graphicsView()->camera()->setCZ(item->getCenterZ());
            }
            redraw();
            return true;
        } else if (_buttonsPressed == Qt::RightButton){
            CT_AbstractItemDrawable* item = removeItemFromSelection(e->pos());

            if (item!=NULL && option->souldAutoCenterCamera())
            {
                graphicsView()->camera()->setCX(item->getCenterX());
                graphicsView()->camera()->setCY(item->getCenterY());
                graphicsView()->camera()->setCZ(item->getCenterZ());
            }
            redraw();
            return true;
        }
    }
    return false;
}

bool ONF_ActionPickItemsInList::wheelEvent(QWheelEvent *e)
{
    Q_UNUSED(e);
    return false;

}

bool ONF_ActionPickItemsInList::keyPressEvent(QKeyEvent *e)
{
    Q_UNUSED(e);
    return false;

}

bool ONF_ActionPickItemsInList::keyReleaseEvent(QKeyEvent *e)
{
    Q_UNUSED(e);
    return false;
}

void ONF_ActionPickItemsInList::draw(GraphicsViewInterface &view, PainterInterface &painter)
{
    Q_UNUSED(view)

    ONF_ActionPickItemsInListOptions *option = (ONF_ActionPickItemsInListOptions*)optionAt(0);

    painter.save();

    QListIterator<CT_AbstractItemDrawable*> it(_itemList);
    while (it.hasNext())
    {
        CT_AbstractItemDrawable *item = it.next();

        if (_selectedItems->contains(item))
        {
            painter.setColor(QColor(255,0,0));
        } else {
            painter.setColor(option->getColor());
        }
        item->draw(view, painter);
    }

    painter.restore();
}

void ONF_ActionPickItemsInList::drawOverlay(GraphicsViewInterface &view, QPainter &painter)
{
    Q_UNUSED(view)
    Q_UNUSED(painter)
}

CT_AbstractAction* ONF_ActionPickItemsInList::copy() const
{
    return new ONF_ActionPickItemsInList(_itemList, _selectedItems, _maxDist);
}

CT_AbstractItemDrawable* ONF_ActionPickItemsInList::addItemToSelection(const QPoint &point)
{
    Eigen::Vector3d origin, direction;
    graphicsView()->convertClickToLine(point, origin, direction);

    float minDist = std::numeric_limits<float>::max();
    CT_AbstractItemDrawable* pickedItem = NULL;

    QListIterator<CT_AbstractItemDrawable*> it(_itemList);
    while (it.hasNext())
    {
        CT_AbstractItemDrawable *item = it.next();
        float dist = CT_MathPoint::distancePointLine(item->getCenterCoordinate(), direction, origin);

        if (dist < minDist && dist < _maxDist)
        {
            minDist = dist;
            pickedItem = item;
        }
    }

    if (pickedItem != NULL && !_selectedItems->contains(pickedItem))
    {
        _selectedItems->append(pickedItem);
    }
    return pickedItem;
}

CT_AbstractItemDrawable *ONF_ActionPickItemsInList::removeItemFromSelection(const QPoint &point)
{
    Eigen::Vector3d origin, direction;
    graphicsView()->convertClickToLine(point, origin, direction);

    float minDist = std::numeric_limits<float>::max();
    CT_AbstractItemDrawable* pickedItem = NULL;

    QListIterator<CT_AbstractItemDrawable*> it(*_selectedItems);
    while (it.hasNext())
    {
        CT_AbstractItemDrawable *item = it.next();
        float dist = CT_MathPoint::distancePointLine(item->getCenterCoordinate(), direction, origin);

        if (dist < minDist && dist < _maxDist)
        {
            minDist = dist;
            pickedItem = item;
        }
    }

    if (pickedItem != NULL && _selectedItems->contains(pickedItem))
    {
        _selectedItems->removeOne(pickedItem);
    }

    return pickedItem;
}
