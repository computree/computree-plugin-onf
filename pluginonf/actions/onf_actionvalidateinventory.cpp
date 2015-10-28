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

#include "actions/onf_actionvalidateinventory.h"
#include "ct_global/ct_context.h"
#include "views/actions/onf_actionvalidateinventoryattributesdialog.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QIcon>
#include <QPainter>
#include <limits>

#include "ct_math/ct_mathpoint.h"

ONF_ActionValidateInventory::ONF_ActionValidateInventory(QMap<const CT_StandardItemGroup*, const CT_AbstractSingularItemDrawable*> *selectedItem,
                                                   QMultiMap<const CT_StandardItemGroup *, const CT_AbstractSingularItemDrawable*> *availableItem,
                                                   QMap<const CT_StandardItemGroup*, QString> *species,
                                                   QMap<const CT_StandardItemGroup *, QString> *ids,
                                                   const QStringList &speciesList) : CT_AbstractActionForGraphicsView()
{
    _selectedItem = selectedItem;
    _initiallySelectedItem = *selectedItem;
    _availableItem = availableItem;
    _species = species;
    _ids = ids;
    _speciesList = speciesList;
    _currentItem = NULL;
}

ONF_ActionValidateInventory::~ONF_ActionValidateInventory()
{
}

QString ONF_ActionValidateInventory::uniqueName() const
{
    return "ONF_ActionValidateInventory";
}

QString ONF_ActionValidateInventory::title() const
{
    return tr("Validation Inventaire");
}

QString ONF_ActionValidateInventory::description() const
{
    return tr("Validation Inventaire");
}

QIcon ONF_ActionValidateInventory::icon() const
{
    return QIcon(":/icons/cursor.png");
}

QString ONF_ActionValidateInventory::type() const
{
    return CT_AbstractAction::TYPE_MEASURE;
}

void ONF_ActionValidateInventory::init()
{
    CT_AbstractActionForGraphicsView::init();

    if(nOptions() == 0)
    {
        // create the option widget if it was not already created
        ONF_ActionValidateInventoryOptions *option = new ONF_ActionValidateInventoryOptions(this);

        // add the options to the graphics view
        graphicsView()->addActionOptions(option);

        connect(option, SIGNAL(parametersChanged()), this, SLOT(redraw()));
        connect(option, SIGNAL(askForCurrentItemReset()), this, SLOT(resetCurrentItem()));

        // register the option to the superclass, so the hideOptions and showOptions
        // is managed automatically
        registerOption(option);

        document()->redrawGraphics();
    }
}

void ONF_ActionValidateInventory::redraw()
{
    document()->redrawGraphics();
}

void ONF_ActionValidateInventory::updateSpeciesList(const QString &species)
{
    if (!_speciesList.contains(species))
    {
        _speciesList.append(species);
    }
}

void ONF_ActionValidateInventory::resetCurrentItem()
{
    if (_currentItem != NULL)

    {
        const CT_StandardItemGroup* group = _availableItem->key(_currentItem);
        _currentItem = _initiallySelectedItem.value(group);
        _selectedItem->insert(group, _currentItem);
        redraw();
    }
}

bool ONF_ActionValidateInventory::mousePressEvent(QMouseEvent *e)
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

bool ONF_ActionValidateInventory::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    return false;
}

bool ONF_ActionValidateInventory::mouseReleaseEvent(QMouseEvent *e)
{   
    ONF_ActionValidateInventoryOptions *option = (ONF_ActionValidateInventoryOptions*)optionAt(0);

    QPoint point = e->pos() - _oldPos;

    if (point.manhattanLength() <= 3)
    {
        if (_buttonsPressed == Qt::LeftButton)
        {

            if (option->getMode() == ONF_ActionValidateInventoryOptions::Mode_Item)
            {
                _currentItem = chooseForItem(e->pos());
            } else {
                _currentItem = chooseForAttributes(e->pos());
            }

            if (_currentItem!=NULL && option->shouldAutoCenterCamera())
            {
                graphicsView()->camera()->setCX(_currentItem->getCenterX());
                graphicsView()->camera()->setCY(_currentItem->getCenterY());
                graphicsView()->camera()->setCZ(_currentItem->getCenterZ());
            }
            redraw();
            return true;
        } else if (_buttonsPressed == Qt::RightButton)
        {
            option->toggleMode();
            redraw();
            return true;
        }
    }
    return false;
}

bool ONF_ActionValidateInventory::wheelEvent(QWheelEvent *e)
{
    Q_UNUSED(e);
    return false;

}

bool ONF_ActionValidateInventory::keyPressEvent(QKeyEvent *e)
{   
    Q_UNUSED(e);
    return false;

}

bool ONF_ActionValidateInventory::keyReleaseEvent(QKeyEvent *e)
{
    Q_UNUSED(e);
    return false;
}

void ONF_ActionValidateInventory::draw(GraphicsViewInterface &view, PainterInterface &painter)
{
    Q_UNUSED(view)

    ONF_ActionValidateInventoryOptions *option = (ONF_ActionValidateInventoryOptions*)optionAt(0);

    painter.save();

    // Draw Circles
    QList<const CT_AbstractSingularItemDrawable*> seletedCircles = _selectedItem->values();
    QMapIterator<const CT_StandardItemGroup*, const CT_AbstractSingularItemDrawable*> itItems(*_availableItem);
    while (itItems.hasNext())
    {
        itItems.next();
        CT_AbstractSingularItemDrawable* item = (CT_AbstractSingularItemDrawable*) itItems.value();

        if (item != NULL)
        {
            bool selected = false;
            if (item == _currentItem) {
                painter.setColor(Qt::red);
                selected = true;
            } else if (seletedCircles.contains(item)) {
                painter.setColor(Qt::darkRed);
                selected = true;
            } else {
                painter.setColor(Qt::blue);
            }
            if (selected || option->shouldShowCandidateItems())
            {
                item->draw(view, painter);
            }
        }
    }

    painter.restore();
}

void ONF_ActionValidateInventory::drawOverlay(GraphicsViewInterface &view, QPainter &painter)
{
    Q_UNUSED(view)

    ONF_ActionValidateInventoryOptions *option = (ONF_ActionValidateInventoryOptions*)optionAt(0);

    if (option->shouldShowData())
    {
        int add = painter.fontMetrics().height()+2;

        QMapIterator<const CT_StandardItemGroup*, const CT_AbstractSingularItemDrawable*> it(*_selectedItem);
        while (it.hasNext())
        {
            it.next();
            CT_StandardItemGroup* scene = (CT_StandardItemGroup*) it.key();
            CT_AbstractSingularItemDrawable* item = (CT_AbstractSingularItemDrawable*) it.value();
            QString id = _ids->value(scene);
            QString species = _species->value(scene);

            QPoint pixel;
            graphicsView()->convert3DPositionToPixel(Eigen::Vector3d(item->getCenterX(), item->getCenterY(), item->getCenterZ()), pixel);

            painter.setPen(Qt::darkRed);
            int y =  pixel.y() + add + 2;
            painter.drawText(pixel.x(), y, tr("ID = %1").arg(id));
            y += add;
            painter.drawText(pixel.x(), y, tr("Espèce = %1").arg(species));

        }
    }
}

CT_AbstractAction* ONF_ActionValidateInventory::copy() const
{
    return new ONF_ActionValidateInventory(_selectedItem, _availableItem, _species, _ids, _speciesList);
}

const CT_AbstractSingularItemDrawable* ONF_ActionValidateInventory::chooseForItem(const QPoint &point)
{
    Eigen::Vector3d origin, direction;
    graphicsView()->convertClickToLine(point, origin, direction);

    float minDist = std::numeric_limits<float>::max();
    const CT_AbstractSingularItemDrawable* pickedItem = NULL;
    const CT_StandardItemGroup*  pickedItemScene = NULL;

    QMapIterator<const CT_StandardItemGroup*, const CT_AbstractSingularItemDrawable*> it(*_availableItem);
    while (it.hasNext())
    {
        it.next();
        const CT_StandardItemGroup* scene = it.key();
        const CT_AbstractSingularItemDrawable* item = it.value();

        float dist = CT_MathPoint::distancePointLine(item->getCenterCoordinate(), direction, origin);

        if (dist < minDist)
        {
            minDist = dist;
            pickedItem = item;
            pickedItemScene = scene;
        }
    }

    if (pickedItem != NULL && pickedItemScene != NULL)
    {
        _selectedItem->insert(pickedItemScene, pickedItem);
    }

    return pickedItem;
}

const CT_AbstractSingularItemDrawable* ONF_ActionValidateInventory::chooseForAttributes(const QPoint &point)
{
    Eigen::Vector3d origin, direction;
    graphicsView()->convertClickToLine(point, origin, direction);

    float minDist = std::numeric_limits<float>::max();
    const CT_AbstractSingularItemDrawable* pickedItem = NULL;
    const CT_StandardItemGroup*  pickedItemScene = NULL;

    QMapIterator<const CT_StandardItemGroup*, const CT_AbstractSingularItemDrawable*> it(*_selectedItem);
    while (it.hasNext())
    {
        it.next();
        const CT_StandardItemGroup* scene = it.key();
        const CT_AbstractSingularItemDrawable* circle = it.value();

        float dist = CT_MathPoint::distancePointLine(circle->getCenterCoordinate(), direction, origin);

        if (dist < minDist)
        {
            minDist = dist;
            pickedItem = circle;
            pickedItemScene = scene;
        }
    }

    redraw();
    setAttributes(pickedItemScene);

    return pickedItem;
}

void ONF_ActionValidateInventory::setAttributes(const CT_StandardItemGroup* scene)
{
    QString currentSpecies = _species->value(scene);
    QString currentId = _ids->value(scene);

    ONF_ActionValidateInventoryAttributesDialog dialog(_speciesList, currentSpecies, currentId);

    if (dialog.exec() == QDialog::Accepted)
    {
        QString newSpecies = dialog.getSpecies();
        _species->insert(scene, newSpecies);
        _ids->insert(scene, dialog.getId());

        updateSpeciesList(newSpecies);
    }

}


