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

#include "actions/onf_actionmodifyaffiliations.h"
#include "ct_global/ct_context.h"
#include "ct_math/ct_mathpoint.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QIcon>
#include <QPainter>
#include <QDebug>
#include <limits>

#include <eigen/Eigen/Core>


ONF_ActionModifyAffiliations::ONF_ActionModifyAffiliations(const QList<CT_AbstractSingularItemDrawable*> *sourceList,
                                                         const QList<CT_AbstractSingularItemDrawable*> *targetList,
                                                         QMap<CT_AbstractSingularItemDrawable*, CT_AbstractSingularItemDrawable*> *correspondances) : CT_AbstractActionForGraphicsView()
{
    m_status = 0;

    _symbolSize = 0.25;

    _sourceList = sourceList;
    _targetList = targetList;
    _correspondances = correspondances;

    for (int i = 0 ; i < _sourceList->size() ; i++)
    {
        CT_AbstractSingularItemDrawable* item = _sourceList->at(i);
        _itemCenters.insert(item, Eigen::Vector3d(item->getCenterX(), item->getCenterY(), item->getCenterZ()));
    }

    for (int i = 0 ; i < _targetList->size() ; i++)
    {
        CT_AbstractSingularItemDrawable* item = _targetList->at(i);
        _itemCenters.insert(item, Eigen::Vector3d(item->getCenterX(), item->getCenterY(), item->getCenterZ()));
    }

    _activeSource = NULL;
    _activeTarget = NULL;
}


QString ONF_ActionModifyAffiliations::uniqueName() const
{
    return "ONF_ActionModifyAffiliations";
}

QString ONF_ActionModifyAffiliations::title() const
{
    return "Modify affiliations";
}

QString ONF_ActionModifyAffiliations::description() const
{
    return "Permet de modifier des affiliations";
}

QIcon ONF_ActionModifyAffiliations::icon() const
{
    return QIcon(":/icons/show_all.png");
}

QString ONF_ActionModifyAffiliations::type() const
{
    return CT_AbstractAction::TYPE_MEASURE;
}

void ONF_ActionModifyAffiliations::init()
{
    CT_AbstractActionForGraphicsView::init();

    m_status = 0;

    if(nOptions() == 0)
    {
        // create the option widget if it was not already created
        ONF_ActionModifyAffiliationsOptions *option = new ONF_ActionModifyAffiliationsOptions(this);

        // add the options to the graphics view
        graphicsView()->addActionOptions(option);

        connect(option, SIGNAL(parametersChanged()), this, SLOT(updateGraphics()));
        connect(option, SIGNAL(askForAffiliation()), this, SLOT(affiliate()));
        connect(option, SIGNAL(askForBreakingAffiliation()), this, SLOT(breakAffiliation()));

        // register the option to the superclass, so the hideOptions and showOptions
        // is managed automatically
        registerOption(option);

        Eigen::Vector3d min, max;

        min(0) = std::numeric_limits<double>::max();
        min(1) = std::numeric_limits<double>::max();
        min(2) = std::numeric_limits<double>::max();

        max(0) = -std::numeric_limits<double>::max();
        max(1) = -std::numeric_limits<double>::max();
        max(2) = -std::numeric_limits<double>::max();

        QMapIterator<CT_AbstractSingularItemDrawable*, Eigen::Vector3d> it(_itemCenters);
        while (it.hasNext())
        {
            it.next();
            const Eigen::Vector3d &position = it.value();

            if (position(0) < min(0)) {min(0) = position(0);}
            if (position(1) < min(1)) {min(1) = position(1);}
            if (position(2) < min(2)) {min(2) = position(2);}

            if (position(0) > max(0)) {max(0) = position(0);}
            if (position(1) > max(1)) {max(1) = position(1);}
            if (position(2) > max(2)) {max(2) = position(2);}
        }

        dynamic_cast<GraphicsViewInterface*>(document()->views().first())->camera()->fitToSpecifiedBox(min, max);
    }
}

CT_AbstractSingularItemDrawable * ONF_ActionModifyAffiliations::closetItemFromClickDirection(QMouseEvent *e,
                                                                                            const QList<CT_AbstractSingularItemDrawable*> *itemList)
{
    Eigen::Vector3d origin, direction;
    GraphicsViewInterface *view = graphicsView();
    view->convertClickToLine(e->pos(), origin, direction);

    float minDist = std::numeric_limits<float>::max();
    CT_AbstractSingularItemDrawable* closestItem = NULL;

    int size = itemList->size();
    for (int i = 0 ; i < size ; i++)
    {
        CT_AbstractSingularItemDrawable* item = itemList->at(i);
        const Eigen::Vector3d &center = _itemCenters.value(item);

        double distance = CT_MathPoint::distancePointLine(center, direction, origin);
        if (distance < minDist)
        {
            minDist = distance;
            closestItem = item;
        }
    }

    return closestItem;
}


bool ONF_ActionModifyAffiliations::mouseDoubleClickEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    return true;
}

bool ONF_ActionModifyAffiliations::mousePressEvent(QMouseEvent *e)
{
    ONF_ActionModifyAffiliationsOptions *option = (ONF_ActionModifyAffiliationsOptions*)optionAt(0);

    if (option->selectionActivated())
    {
        if (e->buttons() & Qt::LeftButton)
        {
            _activeSource = closetItemFromClickDirection(e, _sourceList);
            document()->redrawGraphics();
        } else if (e->buttons() & Qt::RightButton)
        {
            _activeTarget = closetItemFromClickDirection(e, _targetList);
            document()->redrawGraphics();
        }
    }

    return false;
}

bool ONF_ActionModifyAffiliations::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    return false;
}

bool ONF_ActionModifyAffiliations::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    return false;
}

bool ONF_ActionModifyAffiliations::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers()  & Qt::ControlModifier)
    {
        int sourceIndex = _sourceList->indexOf(_activeSource);
        if (e->delta()>0)
        {
            if (sourceIndex < (_sourceList->size() - 1))
            {
                _activeSource = _sourceList->at(sourceIndex + 1);
            }
        } else if (e->delta()<0)
        {
            if (sourceIndex > 0)
            {
                _activeSource = _sourceList->at(sourceIndex - 1);
            }
        }
        document()->redrawGraphics();
        return true;
    } else if (e->modifiers()  & Qt::ShiftModifier)
    {
        int targetIndex = _targetList->indexOf(_activeTarget);
        if (e->delta()>0)
        {
            if (targetIndex < (_targetList->size() - 1))
            {
                _activeTarget = _targetList->at(targetIndex + 1);
            }
        } else if (e->delta()<0)
        {
            if (targetIndex > 0)
            {
                _activeTarget = _targetList->at(targetIndex - 1);
            }
        }
        document()->redrawGraphics();
        return true;
    }

    return false;
}

bool ONF_ActionModifyAffiliations::keyPressEvent(QKeyEvent *e)
{
    if((e->key() == Qt::Key_A)
            && !e->isAutoRepeat())
    {
        affiliate();
        return true;
    } else if((e->key() == Qt::Key_Z)
              && !e->isAutoRepeat())
    {
        breakAffiliation();
        return true;
    } else if((e->key() == Qt::Key_S)
                 && !e->isAutoRepeat())
       {
        ONF_ActionModifyAffiliationsOptions *option = (ONF_ActionModifyAffiliationsOptions*)optionAt(0);
        option->toggleSelection();
        return true;
       }
    return false;
}

bool ONF_ActionModifyAffiliations::keyReleaseEvent(QKeyEvent *e)
{
    Q_UNUSED(e);
    return false;
}

void ONF_ActionModifyAffiliations::updateGraphics()
{
    document()->redrawGraphics();
}

void ONF_ActionModifyAffiliations::affiliate()
{
    if (_activeSource!=NULL && _activeTarget!=NULL)
    {
        _correspondances->remove(_activeTarget);

        QMutableMapIterator<CT_AbstractSingularItemDrawable*, CT_AbstractSingularItemDrawable*> it(*_correspondances);
        while (it.hasNext())
        {
            it.next();
            if (it.value() == _activeSource)
            {
                it.remove();
            }
        }

        _correspondances->insert(_activeTarget, _activeSource);

        document()->redrawGraphics();
    }
}

void ONF_ActionModifyAffiliations::breakAffiliation()
{
    _correspondances->remove(_activeTarget);
    document()->redrawGraphics();
}

void ONF_ActionModifyAffiliations::draw(GraphicsViewInterface &view, PainterInterface &painter)
{
    Q_UNUSED(view);

    ONF_ActionModifyAffiliationsOptions *option = (ONF_ActionModifyAffiliationsOptions*)optionAt(0);

    painter.save();

    if (option->sourceVisible())
    {
        for (int i = 0 ; i < _sourceList->size() ; i++)
        {
            CT_AbstractSingularItemDrawable* item = _sourceList->at(i);

            if (item == _activeSource)
            {
                painter.setColor(QColor(255, 0, 255));
            } else {
                painter.setColor(Qt::yellow);
            }


            if (!option->onlySelectedVisible() || item == _activeSource)
            {
                if (option->centersVisible())
                {
                    drawSourceItem(painter, _itemCenters.value(item));
                }

                if (option->itemsVisible())
                {
                    item->draw(view, painter);
                }
            }
        }
    }


    if (option->targetVisible())
    {
        for (int i = 0 ; i < _targetList->size() ; i++)
        {
            CT_AbstractSingularItemDrawable* item = _targetList->at(i);

            if (item == _activeTarget)
            {
                painter.setColor(QColor(255, 0, 0));
            } else {
                painter.setColor(QColor(0, 255, 0));
            }

            if (!option->onlySelectedVisible() || item == _activeTarget)
            {
                if (option->centersVisible())
                {
                    drawSourceItem(painter, _itemCenters.value(item));
                }

                if (option->itemsVisible())
                {
                    item->draw(view, painter);
                }
            }
        }
    }

    if (option->affiliationsLinesVisible())
    {
        QMapIterator<CT_AbstractSingularItemDrawable*, CT_AbstractSingularItemDrawable*> it(*_correspondances);
        while (it.hasNext())
        {
            it.next();

            if (it.key() == _activeTarget && it.value() == _activeSource)
            {
                painter.setColor(QColor(255, 0, 0));
            } else {
                painter.setColor(QColor(0, 255, 0));
            }

            if (!option->onlySelectedVisible() || it.key() == _activeTarget || it.value() == _activeSource)
            {

                drawAffiliation(painter, _itemCenters.value(it.key()), _itemCenters.value(it.value()));
            }
        }
    }

    painter.restore();
}

void ONF_ActionModifyAffiliations::drawSourceItem(PainterInterface &painter, const Eigen::Vector3d &position)
{
    painter.drawLine(position(0) - _symbolSize, position(1), position(2), position(0) + _symbolSize, position(1), position(2));
    painter.drawLine(position(0), position(1) - _symbolSize, position(2), position(0), position(1) + _symbolSize, position(2));
}

void ONF_ActionModifyAffiliations::drawTargetItem(PainterInterface &painter, const Eigen::Vector3d &position)
{
    painter.drawLine(position(0) - _symbolSize, position(1) - _symbolSize, position(2), position(0) + _symbolSize, position(1) + _symbolSize, position(2));
    painter.drawLine(position(0) - _symbolSize, position(1) + _symbolSize, position(2), position(0) + _symbolSize, position(1) - _symbolSize, position(2));
}

void ONF_ActionModifyAffiliations::drawAffiliation(PainterInterface &painter, const Eigen::Vector3d &source, const Eigen::Vector3d &target)
{
    painter.drawLine(source(0), source(1), source(2), target(0), target(1), target(2));
}

void ONF_ActionModifyAffiliations::drawOverlay(GraphicsViewInterface &view, QPainter &painter)
{
    Q_UNUSED(view)

    ONF_ActionModifyAffiliationsOptions *option = (ONF_ActionModifyAffiliationsOptions*)optionAt(0);

    if (option->showAttributes())
    {
        int add = painter.fontMetrics().height()+2;
        int y = add;

        if(_activeSource != NULL)
        {
            QList<CT_AbstractItemAttribute*> attList = _activeSource->itemAttributes();

            if(!attList.isEmpty())
            {
                painter.save();
                painter.setPen(QColor(255,0,255,127));
                QListIterator<CT_AbstractItemAttribute*> itAtt(attList);

                while(itAtt.hasNext())
                {
                    CT_AbstractItemAttribute *att = itAtt.next();

                    QString txt = att->displayableName() + " = ";
                    txt += att->toString(_activeSource, NULL);

                    painter.drawText(2, y, txt);
                    y += add;
                }

                painter.restore();

                y += add;
            }
        }

        if(_activeTarget != NULL)
        {
            QList<CT_AbstractItemAttribute*> attList = _activeTarget->itemAttributes();

            if(!attList.isEmpty())
            {
                painter.save();
                painter.setPen(QColor(255,255,0,127));
                QListIterator<CT_AbstractItemAttribute*> itAtt(attList);

                while(itAtt.hasNext())
                {
                    CT_AbstractItemAttribute *att = itAtt.next();

                    QString txt = att->displayableName() + " = ";
                    txt += att->toString(_activeTarget, NULL);

                    painter.drawText(2, y, txt);
                    y += add;
                }

                painter.restore();

                y += add;
            }
        }
    }
}

CT_AbstractAction *ONF_ActionModifyAffiliations::copy() const
{
    return new ONF_ActionModifyAffiliations(_sourceList, _targetList, _correspondances);
}

