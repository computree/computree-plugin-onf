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

#include "actions/onf_actionmodifypositions2d.h"
#include "ct_global/ct_context.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QIcon>
#include <QPainter>
#include <limits>


ONF_ActionModifyPositions2D::ONF_ActionModifyPositions2D(QList<CT_Point2D*> &positions, CT_OutAbstractSingularItemModel *model, CT_ResultGroup* outRes) : CT_AbstractActionForGraphicsView()
{
    _positions = &positions;
    _model = model;
    _outRes = outRes;

    _selectedColor = QColor(0, 255, 0);
    _normalColor   = QColor(255, 0, 0);

    _selectedPoint = NULL;
    _leftButton = false;
    _zmin = 0;
    _zmax = 10;
}

ONF_ActionModifyPositions2D::~ONF_ActionModifyPositions2D()
{
}

QString ONF_ActionModifyPositions2D::uniqueName() const
{
    return "ONF_ActionModifyPositions2D";
}

QString ONF_ActionModifyPositions2D::title() const
{
    return tr("Modifier positions 2D");
}

QString ONF_ActionModifyPositions2D::description() const
{
    return tr("Modifier positions 2D");
}

QIcon ONF_ActionModifyPositions2D::icon() const
{
    return QIcon(":/icons/select_rectangular.png");
}

QString ONF_ActionModifyPositions2D::type() const
{
    return CT_AbstractAction::TYPE_MODIFICATION;
}

void ONF_ActionModifyPositions2D::init()
{
    CT_AbstractActionForGraphicsView::init();

    if(nOptions() == 0)
    {
        // create the option widget if it was not already created
        ONF_ActionModifyPositions2DOptions *option = new ONF_ActionModifyPositions2DOptions(this);

        // add the options to the graphics view
        graphicsView()->addActionOptions(option);

        connect(option, SIGNAL(parametersChanged()), this, SLOT(zValChanged()));

        // register the option to the superclass, so the hideOptions and showOptions
        // is managed automatically
        registerOption(option);

        _min(0) = std::numeric_limits<double>::max();
        _min(1) = std::numeric_limits<double>::max();

        _max(0) = -std::numeric_limits<double>::max();
        _max(1) = -std::numeric_limits<double>::max();

        for (int i = 0 ; i < _positions->size() ; i++)
        {
            CT_Point2D *point = _positions->at(i);
            if (point->x() < _min(0)) {_min(0) = point->x();}
            if (point->y() < _min(1)) {_min(1) = point->y();}

            if (point->x() > _max(0)) {_max(0) = point->x();}
            if (point->y() > _max(1)) {_max(1) = point->y();}

            document()->addItemDrawable(*point);
            document()->setColor(point, _normalColor);
        }

        dynamic_cast<GraphicsViewInterface*>(document()->views().first())->camera()->fitCameraToVisibleItems();
        document()->redrawGraphics(DocumentInterface::RO_WaitForConversionCompleted);
    }
}

void ONF_ActionModifyPositions2D::zValChanged()
{
    if (_positions->size()>0)
    {
        ONF_ActionModifyPositions2DOptions *option = (ONF_ActionModifyPositions2DOptions*)optionAt(0);

       ((CT_StandardPoint2DDrawManager*) _positions->first()->getBaseDrawManager())->setZValue(option->getZValue());
    }

    setDrawing3DChanged();
    document()->redrawGraphics();
}


bool ONF_ActionModifyPositions2D::mousePressEvent(QMouseEvent *e)
{
    ONF_ActionModifyPositions2DOptions *option = (ONF_ActionModifyPositions2DOptions*)optionAt(0);

    if (e->button() == Qt::LeftButton)
    {
        _leftButton = true;
        if (option->isMovePositionSelected() || option->isRemovePositionSelected() || option->isAddPositionSelected())
        {
            _selectedPoint = NULL;

            double x, y;
            if (getCoordsForMousePosition(e, x, y))
            {
                if (option->isMovePositionSelected() || option->isRemovePositionSelected())
                {
                    _selectedPoint = getNearestPosition(x, y);
                } else if (option->isAddPositionSelected())
                {
                    _selectedPoint = new CT_Point2D(_model, _outRes, new CT_Point2DData(x, y));
                    _positions->append(_selectedPoint);

                    document()->addItemDrawable(*_selectedPoint);
                }

                if (_selectedPoint != NULL)
                {
                    document()->setColor(_selectedPoint, _selectedColor);
                    document()->redrawGraphics();
                    return true;
                }
            }
        }

    }

    return false;
}

bool ONF_ActionModifyPositions2D::mouseMoveEvent(QMouseEvent *e)
{
    ONF_ActionModifyPositions2DOptions *option = (ONF_ActionModifyPositions2DOptions*)optionAt(0);

    if (_leftButton && (option->isMovePositionSelected() || option->isAddPositionSelected()))
    {
        if (_selectedPoint != NULL)
        {
            double x, y;
            if (getCoordsForMousePosition(e, x, y))
            {
                document()->lock();
                _selectedPoint->setCenterX(x);
                _selectedPoint->setCenterY(y);
                document()->unlock();

                document()->redrawGraphics();
                return true;
            }
        }
    }

    return false;
}

bool ONF_ActionModifyPositions2D::mouseReleaseEvent(QMouseEvent *e)
{   
    ONF_ActionModifyPositions2DOptions *option = (ONF_ActionModifyPositions2DOptions*)optionAt(0);

    if (_leftButton)
    {
        _leftButton = false;
        if (_selectedPoint != NULL)
        {
            if (option->isMovePositionSelected() || option->isAddPositionSelected())
            {
                double x, y;
                if (getCoordsForMousePosition(e, x, y))
                {
                    document()->lock();
                    _selectedPoint->setCenterX(x);
                    _selectedPoint->setCenterY(y);
                    document()->unlock();

                    if (option->isMovePositionSelected()) {option->selectFreeMove();}

                    document()->setColor(_selectedPoint, _normalColor);
                    document()->redrawGraphics();
                }
                return true;
            } else if (option->isRemovePositionSelected())
            {
                document()->removeItemDrawable(*_selectedPoint);

                _positions->removeOne(_selectedPoint);
                delete _selectedPoint;
                _selectedPoint = NULL;
                option->selectFreeMove();
                document()->redrawGraphics();
                return true;
            }
            _selectedPoint = NULL;
        }
    }

    return false;
}

bool ONF_ActionModifyPositions2D::wheelEvent(QWheelEvent *e)
{
    ONF_ActionModifyPositions2DOptions *option = (ONF_ActionModifyPositions2DOptions*)optionAt(0);

    if (e->modifiers() & Qt::ControlModifier)
    {
        if (e->delta()>0)
        {
            option->increaseZValue();
        } else if (e->delta() < 0)
        {
            option->decreaseZValue();
        }
        return true;
    }

    return false;
}

bool ONF_ActionModifyPositions2D::keyPressEvent(QKeyEvent *e)
{
    ONF_ActionModifyPositions2DOptions *option = (ONF_ActionModifyPositions2DOptions*)optionAt(0);

    if((e->key() == Qt::Key_D) && !e->isAutoRepeat())
    {
        option->selectMovePosition();
        return true;
    }

    if((e->key() == Qt::Key_A) && !e->isAutoRepeat())
    {
        option->selectAddPosition();
        return true;
    }

    if((e->key() == Qt::Key_S) && !e->isAutoRepeat())
    {
        option->selectRemovePosition();
        return true;
    }

    if((e->key() == Qt::Key_F) && !e->isAutoRepeat())
    {
        option->selectFreeMove();
        return true;
    }


    return false;

}

bool ONF_ActionModifyPositions2D::keyReleaseEvent(QKeyEvent *e)
{
    Q_UNUSED(e);
    return false;
}

void ONF_ActionModifyPositions2D::draw(GraphicsViewInterface &view, PainterInterface &painter)
{
    Q_UNUSED(view)

    ONF_ActionModifyPositions2DOptions *option = (ONF_ActionModifyPositions2DOptions*)optionAt(0);

    painter.save();

    painter.setColor(QColor(75, 75, 75, 125));
    if (option->isDrawPlaneSelected()) {painter.fillRectXY(_min, _max, option->getZValue() - 0.10);}

    if (option->isDrawLinesSelected())
    {
        if (option->isUpdateLinesSelected())
        {
            const QList<CT_AbstractItemDrawable*>& itemList = document()->getItemDrawable();

            _zmin = std::numeric_limits<double>::max();
            _zmax = -std::numeric_limits<double>::max();

            for (int i = 0 ; i < itemList.size() ; i++)
            {
                Eigen::Vector3d min, max;
                itemList.at(i)->getBoundingBox(min, max);

                if (min(2) < std::numeric_limits<double>::max() &&
                        min(2) > -std::numeric_limits<double>::max() &&
                        max(2) < std::numeric_limits<double>::max() &&
                        max(2) > -std::numeric_limits<double>::max())
                {
                    if (min(2) < _zmin) {_zmin = min(2);}
                    if (min(2) > _zmax) {_zmax = min(2);}
                    if (max(2) < _zmin) {_zmin = max(2);}
                    if (max(2) > _zmax) {_zmax = max(2);}
                }
            }
        }

        painter.setColor(QColor(255, 0, 0));
        for (int i = 0 ; i < _positions->size() ; i++)
        {
            const CT_Point2D *point = _positions->at(i);
            painter.drawLine(point->getCenterX(), point->getCenterY(), _zmin, point->getCenterX(), point->getCenterY(), _zmax);
        }
    }

    painter.restore();
}

void ONF_ActionModifyPositions2D::drawOverlay(GraphicsViewInterface &view, QPainter &painter)
{
    Q_UNUSED(view)
    Q_UNUSED(painter)
}

CT_AbstractAction* ONF_ActionModifyPositions2D::copy() const
{
    return new ONF_ActionModifyPositions2D(*_positions, _model, _outRes);
}

bool ONF_ActionModifyPositions2D::getCoordsForMousePosition(const QMouseEvent *e, double &x, double &y)
{
    ONF_ActionModifyPositions2DOptions *option = (ONF_ActionModifyPositions2DOptions*)optionAt(0);

    Eigen::Vector3d origin, direction;
    GraphicsViewInterface *view = graphicsView();
    view->convertClickToLine(e->pos(), origin, direction);

    if (direction.z() == 0) {return false;}

    double coef = (option->getZValue() - origin.z())/direction.z();

    x = origin.x() + coef*direction.x();
    y = origin.y() + coef*direction.y();

    return true;
}

CT_Point2D *ONF_ActionModifyPositions2D::getNearestPosition(double x, double y)
{
    double minDist = std::numeric_limits<double>::max();
    CT_Point2D* nearestPoint = NULL;

    for (int i = 0 ; i < _positions->size() ; i++)
    {
        CT_Point2D *point = _positions->at(i);
        double dist = pow(point->x() - x, 2) + pow(point->y() - y, 2);

        if (dist < minDist)
        {
            minDist = dist;
            nearestPoint = point;
        }
    }

    return nearestPoint;
}
