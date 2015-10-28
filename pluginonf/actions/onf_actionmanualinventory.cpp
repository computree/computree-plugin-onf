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

#include "actions/onf_actionmanualinventory.h"
#include "ct_global/ct_context.h"
#include "views/actions/onf_actionmanualinventoryattributesdialog.h"


#include <QMouseEvent>
#include <QKeyEvent>
#include <QIcon>
#include <QPainter>
#include <limits>

#include "ct_math/ct_mathpoint.h"

ONF_ActionManualInventory::ONF_ActionManualInventory(QMap<const CT_Scene*, const CT_Circle*> *selectedDbh,
                                                   QMap<const CT_Scene *, QMultiMap<double, const CT_Circle *> > *availableDbh,
                                                   QList<const CT_Circle *> *preferredDbh,
                                                   QList<const CT_Scene*> *trashedScenes,
                                                   QMap<const CT_Scene *, double> *sceneDTMValues,
                                                   QMap<QString, QStringList> *paramData,
                                                   QMap<const CT_Scene*, QMap<QString, QString> >  *suppAttributes) : CT_AbstractActionForGraphicsView()
{
    _selectedDbh = selectedDbh;
    _availableDbh = availableDbh;
    _preferredDbh = preferredDbh;
    _trashedScenes = trashedScenes;
    _sceneDTMValues = sceneDTMValues;
    _paramData = paramData;
    _suppAttributes = suppAttributes;
    _currentCircle = NULL;
    _currentScene = NULL;

    _activeSceneCirclesColor = Qt::blue;
    _activeSceneCirclesLightColor = Qt::cyan;
    _othersScenesCirclesColor = Qt::darkGray;
    _othersScenesCirclesLightColor = Qt::lightGray;
    _othersCircleColor = QColor(255,125,0);
    _currentCircleColor = Qt::red;
    _trashActiveCircleColor = QColor(240,50,240);
    _trashOtherCircleColor = QColor(170,50,170);

    _validatedActiveCircleColor = Qt::green;
    _validatedOtherCircleColor = Qt::darkGreen;


}

ONF_ActionManualInventory::~ONF_ActionManualInventory()
{
}

QString ONF_ActionManualInventory::uniqueName() const
{
    return "ONF_ActionManualInventory";
}

QString ONF_ActionManualInventory::title() const
{
    return tr("Inventaire Manuel");
}

QString ONF_ActionManualInventory::description() const
{
    return tr("Inventaire Manuel");
}

QIcon ONF_ActionManualInventory::icon() const
{
    return QIcon(":/icons/cursor.png");
}

QString ONF_ActionManualInventory::type() const
{
    return CT_AbstractAction::TYPE_MEASURE;
}

void ONF_ActionManualInventory::init()
{
    CT_AbstractActionForGraphicsView::init();

    if(nOptions() == 0)
    {
        // create the option widget if it was not already created
        ONF_ActionManualInventoryOptions *option = new ONF_ActionManualInventoryOptions(this);

        // add the options to the graphics view
        graphicsView()->addActionOptions(option);

        connect(option, SIGNAL(visibilityChanged()), this, SLOT(visibilityChanged()));
        connect(option, SIGNAL(chooseUpperCircle()), this, SLOT(selectUpperCircle()));
        connect(option, SIGNAL(chooseLowerCircle()), this, SLOT(selectLowerCircle()));
        connect(option, SIGNAL(sendToTrash()), this, SLOT(sendToTrash()));
        connect(option, SIGNAL(retrieveFromTrash()), this, SLOT(retreiveFromTrash()));
        connect(option, SIGNAL(sendToValidated()), this, SLOT(sendToValidated()));

        // register the option to the superclass, so the hideOptions and showOptions
        // is managed automatically
        registerOption(option);

        QMapIterator<const CT_Scene*, const CT_Circle*> it(*_selectedDbh);
        if (it.hasNext())
        {
            it.next();

            _currentScene = (CT_Scene*) it.key();
            _currentCircle = (CT_Circle*) it.value();
        }

        visibilityChanged();

        document()->removeAllItemDrawable();
        QMapIterator<const CT_Scene*, QMultiMap<double, const CT_Circle*> > itAv(*_availableDbh);
        while (itAv.hasNext())
        {
            itAv.next();
            CT_Scene* scene = (CT_Scene*) itAv.key();
            const QMultiMap<double, const CT_Circle*> &circleMap = itAv.value();

            document()->addItemDrawable(*scene);

            QMapIterator<double, const CT_Circle*> itC(circleMap);
            while (itC.hasNext())
            {
                itC.next();
                CT_Circle* circle = (CT_Circle*) itC.value();
                document()->addItemDrawable(*circle);
            }
        }

        document()->redrawGraphics(DocumentInterface::RO_WaitForConversionCompleted);

        dynamic_cast<GraphicsViewInterface*>(document()->views().first())->camera()->fitCameraToVisibleItems();
    }
}

bool ONF_ActionManualInventory::mousePressEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    return false;
}

bool ONF_ActionManualInventory::mouseDoubleClickEvent(QMouseEvent *e)
{
    ONF_ActionManualInventoryOptions *option = (ONF_ActionManualInventoryOptions*)optionAt(0);

    if (e->button() == Qt::LeftButton)
    {
        if (option->isDbhModeSelected())
        {
            chooseForDbh(e->pos());
        } else if (option->isAttributesModeSelected()){
            chooseForAttributes(e->pos());
        } else {
            selectActiveScene(e->pos());
        }

        if (_currentCircle!=NULL && option->shouldAutoCenterCamera())
        {
            graphicsView()->camera()->setSceneCenter(_currentCircle->getCenterX(),
                                                     _currentCircle->getCenterY(),
                                                     _currentCircle->getCenterZ());
        }
        return true;
    }
    return false;
}

bool ONF_ActionManualInventory::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    return false;
}

bool ONF_ActionManualInventory::mouseReleaseEvent(QMouseEvent *e)
{   
    Q_UNUSED(e);
    return false;
}

bool ONF_ActionManualInventory::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() & Qt::ControlModifier)
    {
        if (e->delta()>0)
        {
            selectUpperCircle();
        } else if (e->delta() < 0)
        {
            selectLowerCircle();
        }
        return true;
    }

    return false;
}

bool ONF_ActionManualInventory::keyPressEvent(QKeyEvent *e)
{   
    ONF_ActionManualInventoryOptions *option = (ONF_ActionManualInventoryOptions*)optionAt(0);

    if((e->key() == Qt::Key_S) && !e->isAutoRepeat())
    {
        option->chooseSelectMode();
        return true;
    } else if ((e->key() == Qt::Key_D) && !e->isAutoRepeat())
    {
        option->chooseDbhMode();
        return true;
    } else if ((e->key() == Qt::Key_F) && !e->isAutoRepeat())
    {
        option->chooseAttributesMode();
        return true;
    } else if (((e->key() == Qt::Key_C) || (e->key() == Qt::Key_A)) && !e->isAutoRepeat())
    {
        setAttributes(_currentScene);
        document()->redrawGraphics();
        return true;
    } else if((e->key() == Qt::Key_Delete) && !e->isAutoRepeat())
    {
        if (_currentScene != NULL)
        {
            if (_trashedScenes->contains(_currentScene))
            {
                retreiveFromTrash();
            } else {
                sendToTrash();
            }
            return true;
        }
    }  else if(((e->key() == Qt::Key_End) || (e->key() == Qt::Key_V)) && !e->isAutoRepeat())
    {
        if (_currentScene != NULL)
        {
            sendToValidated();
            return true;
        }
    }

    return false;

}

bool ONF_ActionManualInventory::keyReleaseEvent(QKeyEvent *e)
{
    Q_UNUSED(e);
    return false;
}

void ONF_ActionManualInventory::draw(GraphicsViewInterface &view, PainterInterface &painter)
{
    Q_UNUSED(view)
    Q_UNUSED(painter)
}

void ONF_ActionManualInventory::drawOverlay(GraphicsViewInterface &view, QPainter &painter)
{
    Q_UNUSED(view)
    ONF_ActionManualInventoryOptions *option = (ONF_ActionManualInventoryOptions*)optionAt(0);
    int add = painter.fontMetrics().height()+2;

    painter.setPen(_currentCircleColor);

    if (_currentCircle != NULL && _currentScene != NULL)
    {
        int y =  add + 2;

        painter.drawText(2, y, tr("Attributs du cercle actif :"));
        y += add;
        painter.drawText(2, y, tr("H = %1 m").arg(_currentCircle->getCenterZ() - _sceneDTMValues->value(_currentScene, NAN)));
        y += add;
        painter.drawText(2, y, tr("D = %1 cm").arg(_currentCircle->getRadius()*200.0));
        y += add;


        QMap<QString, QString> attrMap = _suppAttributes->value(_currentScene);
        QMapIterator<QString, QString> it(attrMap);
        while (it.hasNext())
        {
            it.next();
            painter.drawText(2, y, tr("%1 = %2").arg(it.key()).arg(it.value()));
            y += add;
        }
    }



    painter.setPen(_othersCircleColor);

    if (option->isShowDataChecked())
    {
        QMapIterator<const CT_Scene*, const CT_Circle*> it(*_selectedDbh);
        while (it.hasNext())
        {
            it.next();
            CT_Scene* scene = (CT_Scene*) it.key();
            CT_Circle* circle = (CT_Circle*) it.value();

            QMap<QString, QString> attrMap = _suppAttributes->value(scene);

            QPoint pixel;
            graphicsView()->convert3DPositionToPixel(Eigen::Vector3d(circle->getCenterX(), circle->getCenterY(), circle->getCenterZ()), pixel);

            int y =  pixel.y() + add + 2;

            QMapIterator<QString, QString> itAttr(attrMap);
            while (itAttr.hasNext())
            {
                itAttr.next();
                painter.drawText(pixel.x(), y, tr("%1 = %2").arg(itAttr.key()).arg(itAttr.value()));
                y += add;
            }

        }
    }
}

CT_AbstractAction* ONF_ActionManualInventory::copy() const
{
    return new ONF_ActionManualInventory(_selectedDbh, _availableDbh, _preferredDbh, _trashedScenes, _sceneDTMValues, _paramData, _suppAttributes);
}

void ONF_ActionManualInventory::chooseForDbh(const QPoint &point)
{
    Eigen::Vector3d origin, direction;
    graphicsView()->convertClickToLine(point, origin, direction);

    double minDist = std::numeric_limits<double>::max();
    CT_Circle* pickedItem = NULL;
    CT_Scene*  pickedItemScene = NULL;

    QMapIterator<const CT_Scene*, QMultiMap<double, const CT_Circle*> > it(*_availableDbh);
    while (it.hasNext())
    {
        it.next();
        CT_Scene* scene = (CT_Scene*) it.key();
        const QMultiMap<double, const CT_Circle*> &circleMap = it.value();

        QMapIterator<double, const CT_Circle*> itC(circleMap);
        while (itC.hasNext())
        {
            itC.next();
            CT_Circle* circle = (CT_Circle*) itC.value();

            double dist = CT_MathPoint::distancePointLine(circle->getCenterCoordinate(), direction, origin);

            if (dist < minDist)
            {
                minDist = dist;
                pickedItem = circle;
                pickedItemScene = scene;
            }
        }
    }

    if (pickedItem != NULL && pickedItemScene != NULL)
    {
        _selectedDbh->insert(pickedItemScene, pickedItem);
    }

    if (pickedItemScene != _currentScene)
    {
        _currentCircle = pickedItem;
        _currentScene = pickedItemScene;
        visibilityChanged();

    } else if (pickedItem != _currentCircle)
    {
        CT_Circle* oldCircle = _currentCircle;
        _currentCircle = pickedItem;

        updateVisibility(_currentScene, oldCircle);
        updateVisibility(_currentScene, _currentCircle);
        document()->redrawGraphics();
    }
}

void ONF_ActionManualInventory::chooseForAttributes(const QPoint &point)
{
    selectActiveScene(point);

    setAttributes(_currentScene);
    document()->redrawGraphics();
}

void ONF_ActionManualInventory::selectActiveScene(const QPoint &point)
{
    Eigen::Vector3d origin, direction;
    graphicsView()->convertClickToLine(point, origin, direction);

    double minDist = std::numeric_limits<double>::max();
    CT_Circle* pickedItem = NULL;
    CT_Scene*  pickedItemScene = NULL;

    QMapIterator<const CT_Scene*, const CT_Circle*> it(*_selectedDbh);
    while (it.hasNext())
    {
        it.next();
        CT_Scene* scene = (CT_Scene*) it.key();
        CT_Circle* circle = (CT_Circle*) it.value();

        double dist = CT_MathPoint::distancePointLine(circle->getCenterCoordinate(), direction, origin);

        if (dist < minDist)
        {
            minDist = dist;
            pickedItem = circle;
            pickedItemScene = scene;
        }
    }

    if (pickedItemScene != _currentScene)
    {
        _currentCircle = pickedItem;
        _currentScene = pickedItemScene;
        visibilityChanged();
    }
}


void ONF_ActionManualInventory::setAttributes(const CT_Scene* scene)
{
    ONF_ActionManualInventoryOptions *option = (ONF_ActionManualInventoryOptions*)optionAt(0);

    QMap<QString, QString> &attrMap = (QMap<QString, QString>&) _suppAttributes->value(scene);

    ONF_ActionManualInventoryAttributesDialog dialog(_paramData, attrMap);
    if (dialog.exec() == QDialog::Accepted)
    {        
        QMutableMapIterator<QString, QString>it(attrMap);
        while (it.hasNext())
        {
            it.next();
            const QString &name = it.key();
            QString &value  = it.value();

            value = dialog.getValueForAttr(name);
        }

        if (option->isAutoValidateChecked() && !_validatedScenes.contains(scene))
        {
            _validatedScenes.append(scene);
            visibilityChanged();
        }
    }
}

void ONF_ActionManualInventory::selectUpperCircle()
{
    if (_currentScene != NULL && _currentCircle != NULL)
    {
        const QMultiMap<double, const CT_Circle*> &circleMap = _availableDbh->value(_currentScene);

        QMapIterator<double, const CT_Circle*> itC(circleMap);

        if (itC.findNext(_currentCircle) && itC.hasNext())
        {
            CT_Circle* oldCircle = _currentCircle;
            itC.next();
            _currentCircle = (CT_Circle*) itC.value();

            _selectedDbh->insert(_currentScene, _currentCircle);

            updateVisibility(_currentScene, oldCircle);
            updateVisibility(_currentScene, _currentCircle);
            document()->redrawGraphics();
        }
    }
}

void ONF_ActionManualInventory::selectLowerCircle()
{
    if (_currentScene != NULL && _currentCircle != NULL)
    {
        const QMultiMap<double, const CT_Circle*> &circleMap = _availableDbh->value(_currentScene);

        QMapIterator<double, const CT_Circle*> itC(circleMap);
        itC.toBack();

        if (itC.findPrevious(_currentCircle) && itC.hasPrevious())
        {
            CT_Circle* oldCircle = _currentCircle;
            itC.previous();
            _currentCircle = (CT_Circle*) itC.value();

            _selectedDbh->insert(_currentScene, _currentCircle);

            updateVisibility(_currentScene, oldCircle);
            updateVisibility(_currentScene, _currentCircle);
            document()->redrawGraphics();
        }
    }
}

void ONF_ActionManualInventory::sendToTrash()
{
    if (_currentScene != NULL && !_trashedScenes->contains(_currentScene))
    {
        _trashedScenes->append(_currentScene);
        visibilityChanged();
    }
}

void ONF_ActionManualInventory::retreiveFromTrash()
{
    if (_currentScene != NULL && _trashedScenes->contains(_currentScene))
    {
        _trashedScenes->removeOne(_currentScene);
        visibilityChanged();
    }
}

void ONF_ActionManualInventory::sendToValidated()
{
    if (_currentScene != NULL)
    {
        if (_validatedScenes.contains(_currentScene))
        {
            _validatedScenes.removeOne(_currentScene);
            visibilityChanged();
        } else {
            _validatedScenes.append(_currentScene);
            visibilityChanged();
        }
    }
}

void ONF_ActionManualInventory::visibilityChanged()
{
    QMapIterator<const CT_Scene*, QMultiMap<double, const CT_Circle*> > it(*_availableDbh);
    while (it.hasNext())
    {
        it.next();
        CT_Scene* scene = (CT_Scene*) it.key();
        const QMultiMap<double, const CT_Circle*> &circleMap = it.value();

        QMapIterator<double, const CT_Circle*> itC(circleMap);
        while (itC.hasNext())
        {
            itC.next();
            CT_Circle* circle = (CT_Circle*) itC.value();
            updateVisibility(scene, circle);
        }
    }

    document()->redrawGraphics();
}

void ONF_ActionManualInventory::updateVisibility(CT_Scene* scene, CT_Circle* circle)
{
    ONF_ActionManualInventoryOptions *option = (ONF_ActionManualInventoryOptions*)optionAt(0);
    if (_trashedScenes->contains(scene))
    {
        if (option->isShowTrashChecked())
        {
            document()->setVisible(circle, true);

            document()->setVisible(scene, option->isShowTrashedScenesChecked());

            if (_selectedDbh->values().contains(circle))
            {
                document()->setColor(circle, _trashActiveCircleColor);
            } else {
                document()->setColor(circle, _trashOtherCircleColor);
            }
        } else {
            document()->setVisible(circle, false);
        }
    } else {
        if (scene == _currentScene)
        {
            if (option->isShowActiveCirclesChecked() || circle == _currentCircle)
            {
                document()->setVisible(circle, true);

                if (circle == _currentCircle)
                {
                    document()->setColor(circle, _currentCircleColor);
                } else {

                    if (_preferredDbh->contains(circle))
                    {
                        if (_validatedScenes.contains(scene))
                        {
                            document()->setColor(circle, _validatedActiveCircleColor);
                        } else {
                            document()->setColor(circle, _activeSceneCirclesLightColor);
                        }
                    } else {
                        document()->setColor(circle, _activeSceneCirclesColor);
                    }
                }
            } else {
                document()->setVisible(circle, false);
            }

            document()->setVisible(scene, option->isShowActiveSceneChecked());
        } else {
            if (option->isShowOtherCirclesChecked())
            {
                document()->setVisible(circle, true);

                if (_selectedDbh->values().contains(circle))
                {
                    if (_validatedScenes.contains(scene))
                    {
                        document()->setColor(circle, _validatedActiveCircleColor);
                    } else {
                        document()->setColor(circle, _othersCircleColor);
                    }
                } else {
                    if (_validatedScenes.contains(scene))
                    {
                        document()->setColor(circle, _validatedOtherCircleColor);
                    } else {
                        if (_preferredDbh->contains(circle))
                        {
                            document()->setColor(circle, _othersScenesCirclesLightColor);
                        } else {
                            document()->setColor(circle, _othersScenesCirclesColor);
                        }
                    }
                }
            }
            else {
                document()->setVisible(circle, false);
            }

            document()->setVisible(scene, option->isShowOtherScenesChecked());
        }
    }


}



