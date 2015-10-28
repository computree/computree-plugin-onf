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

#include "onf_actionmodifyclustersgroups02.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QIcon>
#include <QPainter>
#include <math.h>

#include "views/actions/onf_actionmodifyclustersgroupsoptions02.h"

#include "ct_math/ct_mathpoint.h"

ONF_ActionModifyClustersGroups02::ONF_ActionModifyClustersGroups02(QMap<const CT_Point2D *, QPair<CT_PointCloudIndexVector *, QList<const CT_PointCluster *> *> > *map, QMultiMap<CT_PointCluster *, CT_PointCluster *> *clToCl, QList<CT_PointCluster *> *trash) : CT_AbstractActionForGraphicsView()
{
    _positionToCluster = map;
    _clusterToCluster = clToCl;
    _trashClusterList = trash;

    m_status = 0;
    m_mousePressed = false;
    m_selectionMode = GraphicsViewInterface::SELECT_ONE;

    _automaticColorList.append(QColor(255,255,200)); // Jaune Clair
    _automaticColorList.append(QColor(255,200,255)); // Magenta Clair
    _automaticColorList.append(QColor(200,255,255)); // Cyan Clair
    _automaticColorList.append(QColor(200,200,255)); // Mauve Clair
    _automaticColorList.append(QColor(200,255,200)); // Vert Clair
    _automaticColorList.append(QColor(255,200,200)); // Rouge Clair
    _automaticColorList.append(QColor(255,200,150)); // Orange clair
    _automaticColorList.append(QColor(150,200,255)); // Bleu Clair
    _automaticColorList.append(QColor(200,255,150)); // Vert-Jaune Clair
    _automaticColorList.append(QColor(150,255,200)); // Turquoise Clair
    _automaticColorList.append(QColor(255,150,200)); // Rose Clair
    _automaticColorList.append(QColor(200,150,255)); // Violet Clair
    _automaticColorList.append(QColor(255,0  ,255)); // Magenta
    _automaticColorList.append(QColor(0  ,0  ,255)); // Bleu
    _automaticColorList.append(QColor(0  ,0  ,255)); // Mauve
    _automaticColorList.append(QColor(255,150,0  )); // Orange
    _automaticColorList.append(QColor(150,255,0  )); // Vert-Jaune
    _automaticColorList.append(QColor(0  ,255,150)); // Turquoise
    _automaticColorList.append(QColor(255,0  ,150)); // Rose
    _automaticColorList.append(QColor(150,0  ,255)); // Violet

    _colorA         = QColor(255,255,0  ); // Jaune
    _colorB         = QColor(0  ,255,255); // Cyan
    _colorTmp       = QColor(255,255,255); // Blanc
    _colorTrash     = QColor(125,125,125); // Grey
    _validatedColor = QColor(0  ,255,0  ); // Vert

    _positionsChanged = true;
}

QString ONF_ActionModifyClustersGroups02::uniqueName() const
{
    return "ONF_ActionModifyClustersGroups02";
}

QString ONF_ActionModifyClustersGroups02::title() const
{
    return tr("Sélection");
}

QString ONF_ActionModifyClustersGroups02::description() const
{
    return tr("Sélection d'éléments");
}

QIcon ONF_ActionModifyClustersGroups02::icon() const
{
    return QIcon(":/icons/cursor.png");
}

QString ONF_ActionModifyClustersGroups02::type() const
{
    return CT_AbstractAction::TYPE_SELECTION;
}

void ONF_ActionModifyClustersGroups02::init()
{
    CT_AbstractActionForGraphicsView::init();

    if(nOptions() == 0)
    {
        // create the option widget if it was not already created
        ONF_ActionModifyClustersGroupsOptions02 *option = new ONF_ActionModifyClustersGroupsOptions02(this);

        // add the options to the graphics view
        graphicsView()->addActionOptions(option);

        // register the option to the superclass, so the hideOptions and showOptions
        // is managed automatically
        registerOption(option);

        document()->removeAllItemDrawable();

        int colorNum = 0;
        int posNum = 0;
        // Initialisation de _clusterToPosition
        QMapIterator<const CT_Point2D*, QPair<CT_PointCloudIndexVector*, QList<const CT_PointCluster*>* > > it(*_positionToCluster);
        while (it.hasNext())
        {
            it.next();
            const CT_Point2D* position = it.key();

            // Création de la liste des couleurs de base (hors groupes A et B)
            _positionsBaseColors.insert(position, _automaticColorList.at(colorNum++));
            if (colorNum >= _automaticColorList.size()) {colorNum = 0;}

            // Choix par défaut des scènes A et B (initialisation)
            if (posNum == 0)
            {
                _positionA = (CT_Point2D*) position;
            } else if (posNum == 1)
            {
                _positionB = (CT_Point2D*) position;
            }


            const QPair<CT_PointCloudIndexVector*, QList<const CT_PointCluster*>* > &pair = it.value();
            for (int i = 0 ; i < pair.second->size() ; i++)
            {
                CT_PointCluster* cluster = (CT_PointCluster*) pair.second->at(i);
                _clusterToPosition.insert(cluster, position);
            }

            posNum++;
        }

        updateAllClustersColors();

        document()->beginAddMultipleItemDrawable();

        QMapIterator<const CT_PointCluster*, const CT_Point2D*> itC(_clusterToPosition);
        while(itC.hasNext())
        {
            document()->addItemDrawable(*(CT_PointCluster*)itC.next().key());
        }

        for (int i = 0 ; i < _trashClusterList->size() ; i++)
        {
            document()->addItemDrawable(*(_trashClusterList->at(i)));
        }

        document()->endAddMultipleItemDrawable();

        option->selectColorA(_colorA);
        option->selectColorB(_colorB);

        document()->redrawGraphics(DocumentInterface::RO_WaitForConversionCompleted);

        connect(option, SIGNAL(setColorA(QColor)), this, SLOT(setColorA(QColor)));
        connect(option, SIGNAL(setColorB(QColor)), this, SLOT(setColorB(QColor)));
        connect(option, SIGNAL(selectPositionA()), this, SLOT(selectPositionA()));
        connect(option, SIGNAL(selectPositionB()), this, SLOT(selectPositionB()));
        connect(option, SIGNAL(visibilityChanged()), this, SLOT(visibilityChanged()));
        connect(option, SIGNAL(affectClusterToA()), this, SLOT(affectClusterToA()));
        connect(option, SIGNAL(affectClusterToB()), this, SLOT(affectClusterToB()));
        connect(option, SIGNAL(affectClusterToTMP()), this, SLOT(affectClusterToTMP()));
        connect(option, SIGNAL(affectClusterToTrash()), this, SLOT(affectClusterToTrash()));
        connect(option, SIGNAL(extend()), this, SLOT(extend()));
        connect(option, SIGNAL(validatePosition()), this, SLOT(validatePosition()));

        _positionsChanged = true;

        dynamic_cast<GraphicsViewInterface*>(document()->views().first())->camera()->fitCameraToVisibleItems();
    }

}

void ONF_ActionModifyClustersGroups02::updateAllClustersColors()
{
    const QList<const CT_PointCluster*>* listA = _positionToCluster->value(_positionA).second;
    const QList<const CT_PointCluster*>* listB = _positionToCluster->value(_positionB).second;

    QMapIterator<const CT_Point2D*, QPair<CT_PointCloudIndexVector*, QList<const CT_PointCluster*>* > > it(*_positionToCluster);
    while (it.hasNext())
    {
        it.next();
        const CT_Point2D* position = it.key();
        const QPair<CT_PointCloudIndexVector*, QList<const CT_PointCluster*>* > &pair = it.value();

        for (int i = 0 ; i < pair.second->size() ; i++)
        {
            CT_PointCluster* cluster = (CT_PointCluster*) pair.second->at(i);


            if (listA->contains(cluster))
            {
                document()->setColor(cluster, _colorA);
            } else if (listB->contains(cluster))
            {
                document()->setColor(cluster, _colorB);
            } else if (_trashClusterList->contains(cluster))
            {
                document()->setColor(cluster, _colorTrash);
            } else {
                if (_validatedPositions.contains(position))
                {
                    document()->setColor(cluster, _validatedColor);
                } else {
                    document()->setColor(cluster, _positionsBaseColors.value(position, _colorTmp));
                }
            }
        }
    }

    for (int i = 0 ; i < _trashClusterList->size() ; i++)
    {
        document()->setColor(_trashClusterList->at(i), _colorTrash);
    }
}


void ONF_ActionModifyClustersGroups02::updateColorForOneCluster(const CT_Point2D* position)
{
    QColor clusterColor = _positionsBaseColors.value(position, _colorTmp);

    if (_validatedPositions.contains(position)) {clusterColor = _validatedColor;}

    if (position == _positionA) {clusterColor = _colorA;}
    if (position == _positionB) {clusterColor = _colorB;}

    const QPair<CT_PointCloudIndexVector*, QList<const CT_PointCluster*>* > &pair = _positionToCluster->value(position);

    for (int i = 0 ; i < pair.second->size() ; i++)
    {
        CT_PointCluster* cluster = (CT_PointCluster*) pair.second->at(i);
        document()->setColor(cluster, clusterColor);
    }
}

bool ONF_ActionModifyClustersGroups02::mouseDoubleClickEvent(QMouseEvent *e)
{
    m_mousePressed = false;
    m_status = 0;
    m_selectionRectangle.setSize(QSize(0,0));

    GraphicsViewInterface *view = graphicsView();

    if (e->button() == Qt::LeftButton)
    {
        document()->setSelectAllItemDrawable(false);
        view->setSelectionMode(GraphicsViewInterface::SELECT_ONE);
        view->select(e->pos());
        selectPositionA();
        document()->setSelectAllItemDrawable(false);
        return true;
    } else if (e->button() == Qt::RightButton)
    {
        document()->setSelectAllItemDrawable(false);
        view->setSelectionMode(GraphicsViewInterface::SELECT_ONE);
        view->select(e->pos());
        selectPositionB();
        document()->setSelectAllItemDrawable(false);
        return true;
    }
    return false;
}

bool ONF_ActionModifyClustersGroups02::mousePressEvent(QMouseEvent *e)
{
    if(e->button() != Qt::LeftButton)
        return false;

    GraphicsViewInterface *view = graphicsView();

    view->setSelectionMode(selectionMode());

    GraphicsViewInterface::SelectionMode mode = selectionModeToBasic(view->selectionMode());

    m_mousePressed = true;
    m_status = 1;
    m_selectionRectangle.setSize(QSize(0,0));

    if(((mode == GraphicsViewInterface::SELECT)
        || (mode == GraphicsViewInterface::ADD)
        || (mode == GraphicsViewInterface::REMOVE)))
    {
        m_selectionRectangle = QRect(e->pos(), e->pos());

        return true;
    }

    return false;
}

bool ONF_ActionModifyClustersGroups02::mouseMoveEvent(QMouseEvent *e)
{
    if(m_status > 0)
    {
        GraphicsViewInterface *view = graphicsView();

        GraphicsViewInterface::SelectionMode mode = selectionModeToBasic(view->selectionMode());

        if((mode == GraphicsViewInterface::ADD_ONE)
                || (mode == GraphicsViewInterface::REMOVE_ONE)
                || (mode == GraphicsViewInterface::SELECT_ONE))
        {
            view->setSelectionMode(GraphicsViewInterface::NONE);

            m_status = 0;
            return false;
        }

        if(mode != GraphicsViewInterface::NONE)
        {
            m_selectionRectangle.setBottomRight(e->pos());
            document()->redrawGraphics();

            return true;
        }
    }

    return false;
}

bool ONF_ActionModifyClustersGroups02::mouseReleaseEvent(QMouseEvent *e)
{
    GraphicsViewInterface *view = graphicsView();

    GraphicsViewInterface::SelectionMode mode = selectionModeToBasic(view->selectionMode());

    if(e->button() == Qt::LeftButton)
        m_mousePressed = false;

    if((m_status > 0)
            && (e->button() == Qt::LeftButton))
    {
        m_status = 0;

        if(mode != GraphicsViewInterface::NONE)
        {
            if(view->mustSelectPoints())
                document()->constructOctreeOfPoints();

            if((mode == GraphicsViewInterface::ADD_ONE)
                    || (mode == GraphicsViewInterface::REMOVE_ONE)
                    || (mode == GraphicsViewInterface::SELECT_ONE))
            {
                view->setSelectRegionWidth(3);
                view->setSelectRegionHeight(3);

                view->select(e->pos());
            }
            else
            {
                m_selectionRectangle = m_selectionRectangle.normalized();

                // Define selection window dimensions
                view->setSelectRegionWidth(m_selectionRectangle.width());
                view->setSelectRegionHeight(m_selectionRectangle.height());
                // Compute rectangle center and perform selection
                view->select(m_selectionRectangle.center());

                document()->redrawGraphics();

                return true;
            }
        }
    }

    document()->redrawGraphics();

    return false;
}

bool ONF_ActionModifyClustersGroups02::keyPressEvent(QKeyEvent *e)
{   
    ONF_ActionModifyClustersGroupsOptions02 *option = (ONF_ActionModifyClustersGroupsOptions02*)optionAt(0);

    if((e->key() == Qt::Key_Control) && !e->isAutoRepeat())
    {
        option->setMultiSelect(true);
        setSelectionMode(option->selectionMode());
        return true;
    }

    if((e->key() == Qt::Key_Shift) && !e->isAutoRepeat())
    {
        extend();
        return true;
    }

    if((e->key() == Qt::Key_A) && !e->isAutoRepeat())
    {
        affectClusterToA();
        return true;
    }

    if((e->key() == Qt::Key_Z) && !e->isAutoRepeat())
    {
        affectClusterToB();
        return true;
    }

    if((e->key() == Qt::Key_E) && !e->isAutoRepeat())
    {
        affectClusterToTMP();
        return true;
    }

    if((e->key() == Qt::Key_R) && !e->isAutoRepeat())
    {
        affectClusterToTrash();
        return true;
    }

    if((e->key() == Qt::Key_Space) && !e->isAutoRepeat())
    {
        option->toggleOthersVisible();
        return true;
    }

    if((e->key() == Qt::Key_V) && !e->isAutoRepeat())
    {
        validatePosition();
        return true;
    }

    return false;
}

bool ONF_ActionModifyClustersGroups02::keyReleaseEvent(QKeyEvent *e)
{
    ONF_ActionModifyClustersGroupsOptions02 *option = (ONF_ActionModifyClustersGroupsOptions02*)optionAt(0);

    if((e->key() == Qt::Key_Control) && !e->isAutoRepeat())
    {
        option->setMultiSelect(false);
        setSelectionMode(option->selectionMode());
        return true;
    }
    return false;
}

void ONF_ActionModifyClustersGroups02::drawOverlay(GraphicsViewInterface &view, QPainter &painter)
{
    Q_UNUSED(view)

    if(m_status > 0)
    {
        painter.save();
        painter.setPen(QColor(102,102,127,127));
        painter.setBrush(QColor(0,0,73,73));
        painter.drawRect(m_selectionRectangle);
        painter.restore();
    }
}

CT_AbstractAction* ONF_ActionModifyClustersGroups02::copy() const
{
    return new ONF_ActionModifyClustersGroups02(_positionToCluster, _clusterToCluster, _trashClusterList);
}

bool ONF_ActionModifyClustersGroups02::setSelectionMode(GraphicsViewInterface::SelectionMode mode)
{
    if(!m_mousePressed)
    {
        m_selectionMode = mode;
        return true;
    }

    return false;
}

GraphicsViewInterface::SelectionMode ONF_ActionModifyClustersGroups02::selectionMode() const
{
    return m_selectionMode;
}

void ONF_ActionModifyClustersGroups02::setColorA(QColor color)
{
    _colorA = color;
    updateColorForOneCluster(_positionA);
    document()->redrawGraphics();
}

void ONF_ActionModifyClustersGroups02::setColorB(QColor color)
{
    _colorB = color;
    updateColorForOneCluster(_positionB);
    document()->redrawGraphics();
}

void ONF_ActionModifyClustersGroups02::selectPositionA()
{
    _positionsChanged = true;
    QList<CT_AbstractItemDrawable*> selectedItems = document()->getSelectedItemDrawable();

    int cpt = 0;
    while (selectedItems.size() > cpt)
    {
        CT_AbstractItemDrawable* item = selectedItems.at(cpt++);
        CT_PointCluster* cluster = dynamic_cast<CT_PointCluster*>(item);

        if (cluster != NULL)
        {
            const CT_Point2D* position = _clusterToPosition.value(cluster, NULL);

            if (position != NULL)
            {
                if (position == _positionB)
                {
                    swapAandB();
                    cpt = selectedItems.size();
                } else
                {
                    CT_Point2D* oldPositionA = _positionA;
                    _positionA = (CT_Point2D*) position;

                    updateColorForOneCluster(oldPositionA);
                    updateColorForOneCluster(_positionA);
                    document()->redrawGraphics();
                    cpt = selectedItems.size();
                }
            }
        }
    }
}

void ONF_ActionModifyClustersGroups02::selectPositionB()
{
    _positionsChanged = true;
    QList<CT_AbstractItemDrawable*> selectedItems = document()->getSelectedItemDrawable();

    int cpt = 0;
    while (selectedItems.size() > cpt)
    {
        CT_AbstractItemDrawable* item = selectedItems.at(cpt++);
        CT_PointCluster* cluster = dynamic_cast<CT_PointCluster*>(item);

        if (cluster != NULL)
        {
            const CT_Point2D* position = _clusterToPosition.value(cluster, NULL);

            if (position != NULL)
            {
                if (position == _positionA)
                {
                    swapAandB();
                    cpt = selectedItems.size();
                } else
                {
                    CT_Point2D* oldPositionB = _positionB;
                    _positionB = (CT_Point2D*) position;

                    updateColorForOneCluster(oldPositionB);
                    updateColorForOneCluster(_positionB);
                    document()->redrawGraphics();
                    cpt = selectedItems.size();
                }
            }
        }
    }
}

void ONF_ActionModifyClustersGroups02::swapAandB()
{
    CT_Point2D* oldPositionB = _positionB;
    _positionB = _positionA;
    _positionA = oldPositionB;
    updateColorForOneCluster(_positionA);
    updateColorForOneCluster(_positionB);
    document()->redrawGraphics();
}

void ONF_ActionModifyClustersGroups02::visibilityChanged()
{
    updateVisiblePositions();
}

void ONF_ActionModifyClustersGroups02::addToA(CT_PointCluster* cluster)
{
    const CT_Point2D* position = _clusterToPosition.value(cluster, NULL);

    if (position != _positionA)
    {
        // Suppression
        if (position != NULL)
        {
            QList<const CT_PointCluster*> *list = _positionToCluster->value(position).second;
            if (list != NULL)
            {
                list->removeOne(cluster);
            } else {PS_LOG->addMessage(LogInterface::info, LogInterface::action, tr("Pas de liste pour la position"));}
            _clusterToPosition.remove(cluster);
        }

        // Ajout
        QList<const CT_PointCluster*> *list2 = _positionToCluster->value(_positionA).second;
        if (list2 != NULL)
        {
            list2->append(cluster);
        } else {PS_LOG->addMessage(LogInterface::info, LogInterface::action, tr("Pas de liste pour la position"));}

        // Suppression
        _temporaryClusterList.removeOne(cluster);
        _trashClusterList->removeOne(cluster);

        // Ajout
        _clusterToPosition.insert(cluster, _positionA);

        // Couleur
        document()->setColor(cluster, _colorA);
    }

}

void ONF_ActionModifyClustersGroups02::addToB(CT_PointCluster* cluster)
{
    const CT_Point2D* position = _clusterToPosition.value(cluster, NULL);

    if (position != _positionB)
    {
        // Suppression
        if (position != NULL)
        {
            QList<const CT_PointCluster*> *list = _positionToCluster->value(position).second;
            if (list != NULL)
            {
                list->removeOne(cluster);
            } else {PS_LOG->addMessage(LogInterface::info, LogInterface::action, tr("Pas de liste pour la position"));}
            _clusterToPosition.remove(cluster);
        }

        // Ajout
        QList<const CT_PointCluster*> *list2 = _positionToCluster->value(_positionB).second;
        if (list2 != NULL)
        {
            list2->append(cluster);
        } else {PS_LOG->addMessage(LogInterface::info, LogInterface::action, tr("Pas de liste pour la position"));}

        // Suppression
        _temporaryClusterList.removeOne(cluster);
        _trashClusterList->removeOne(cluster);

        // Ajout
        _clusterToPosition.insert(cluster, _positionB);

        // Couleur
        document()->setColor(cluster, _colorB);
    }

}

void ONF_ActionModifyClustersGroups02::affectClusterToA()
{
    QList<CT_AbstractItemDrawable*> selected = document()->getSelectedItemDrawable();

    for (int i = 0 ; i < selected.size() ; i++)
    {
        CT_PointCluster* cluster = dynamic_cast<CT_PointCluster*>(selected[i]);

        if (cluster  != NULL)
        {
            addToA(cluster);
        }
    }
    document()->setSelectAllItemDrawable(false);
    document()->redrawGraphics();
}


void ONF_ActionModifyClustersGroups02::affectClusterToB()
{
    QList<CT_AbstractItemDrawable*> selected = document()->getSelectedItemDrawable();

    for (int i = 0 ; i < selected.size() ; i++)
    {
        CT_PointCluster* cluster = dynamic_cast<CT_PointCluster*>(selected[i]);

        if (cluster  != NULL)
        {
            addToB(cluster);
        }
    }
    document()->setSelectAllItemDrawable(false);
    document()->redrawGraphics();
}

void ONF_ActionModifyClustersGroups02::affectClusterToTMP()
{
    QList<CT_AbstractItemDrawable*> selected = document()->getSelectedItemDrawable();

    for (int i = 0 ; i < selected.size() ; i++)
    {
        CT_PointCluster* cluster = dynamic_cast<CT_PointCluster*>(selected[i]);

        if (cluster  != NULL)
        {
            // Suppression
            const CT_Point2D* position = _clusterToPosition.value(cluster, NULL);
            if (position != NULL)
            {
                QList<const CT_PointCluster*> *list = _positionToCluster->value(position).second;
                if (list != NULL)
                {
                    list->removeOne(cluster);
                } else {PS_LOG->addMessage(LogInterface::info, LogInterface::action, tr("Pas de liste pour la position"));}
                _clusterToPosition.remove(cluster);
            }
            _trashClusterList->removeOne(cluster);

            // Ajout
            if (!_temporaryClusterList.contains(cluster))
            {
                _temporaryClusterList.append(cluster);
                document()->setColor(cluster, _colorTmp);
            }
        }
    }
    document()->setSelectAllItemDrawable(false);
    document()->redrawGraphics();
}

void ONF_ActionModifyClustersGroups02::affectClusterToTrash()
{
    QList<CT_AbstractItemDrawable*> selected = document()->getSelectedItemDrawable();

    for (int i = 0 ; i < selected.size() ; i++)
    {
        CT_PointCluster* cluster = dynamic_cast<CT_PointCluster*>(selected[i]);

        if (cluster  != NULL)
        {
            // Suppression
            const CT_Point2D* position = _clusterToPosition.value(cluster, NULL);
            if (position != NULL)
            {
                QList<const CT_PointCluster*> *list = _positionToCluster->value(position).second;
                if (list != NULL)
                {
                    list->removeOne(cluster);
                } else {PS_LOG->addMessage(LogInterface::info, LogInterface::action, tr("Pas de liste pour la position"));}
                _clusterToPosition.remove(cluster);
            }
            _temporaryClusterList.removeOne(cluster);

            // Ajout
            if (!_trashClusterList->contains(cluster))
            {
                _trashClusterList->append(cluster);
                document()->setColor(cluster, _colorTrash);
            }
        }
    }
    document()->setSelectAllItemDrawable(false);
    document()->redrawGraphics();
}

void ONF_ActionModifyClustersGroups02::extend()
{
    QList<CT_AbstractItemDrawable*> selected = document()->getSelectedItemDrawable();

    QList<CT_PointCluster*> toSelect;

    for (int i = 0 ; i < selected.size() ; i++)
    {
        CT_PointCluster* cluster = dynamic_cast<CT_PointCluster*>(selected[i]);

        if (cluster  != NULL)
        {
            QList<CT_PointCluster*> linked = _clusterToCluster->values(cluster);
            QListIterator<CT_PointCluster*> it(linked);
            while (it.hasNext())
            {
                CT_PointCluster* clust = it.next();
                if (!toSelect.contains(clust))
                {
                    toSelect.append(clust);
                    clust->setSelected(true);
                }
            }
        }
    }

    int i = 0;
    while (i < toSelect.size())
    {
        CT_PointCluster* cluster = toSelect[i++];

        QList<CT_PointCluster*> linked = _clusterToCluster->values(cluster);
        QListIterator<CT_PointCluster*> it2(linked);
        while (it2.hasNext())
        {
            CT_PointCluster* clust = it2.next();
            if (!toSelect.contains(clust))
            {
                toSelect.append(clust);
                clust->setSelected(true);
            }
        }
    }

    document()->redrawGraphics();
}

void ONF_ActionModifyClustersGroups02::validatePosition()
{
    if (_positionB != NULL)
    {
        if (_validatedPositions.contains(_positionB))
        {
            _validatedPositions.removeOne(_positionB);
        } else {
            _validatedPositions.append(_positionB);
        }
    }
}

void ONF_ActionModifyClustersGroups02::updateVisiblePositions()
{
    ONF_ActionModifyClustersGroupsOptions02 *option = (ONF_ActionModifyClustersGroupsOptions02*)optionAt(0);

    QMapIterator<const CT_Point2D*, QPair<CT_PointCloudIndexVector*, QList<const CT_PointCluster*>* > > it(*_positionToCluster);
    while (it.hasNext())
    {
        it.next();
        const QPair<CT_PointCloudIndexVector*, QList<const CT_PointCluster*>* > &pair = it.value();
        for (int i = 0 ; i < pair.second->size() ; i++)
        {
            CT_PointCluster* cluster = (CT_PointCluster*) pair.second->at(i);
            document()->setVisible(cluster, option->isOthersVisible() && (option->isValidatedVisible() || !_validatedPositions.contains(it.key())));
        }
    }

    const QPair<CT_PointCloudIndexVector*, QList<const CT_PointCluster*>* > &pairA = _positionToCluster->value(_positionA);
    for (int i = 0 ; i < pairA.second->size() ; i++)
    {
        CT_PointCluster* cluster = (CT_PointCluster*) pairA.second->at(i);
        document()->setVisible(cluster, option->isAVisible());
    }

    const QPair<CT_PointCloudIndexVector*, QList<const CT_PointCluster*>* > &pairB = _positionToCluster->value(_positionB);
    for (int i = 0 ; i < pairB.second->size() ; i++)
    {
        CT_PointCluster* cluster = (CT_PointCluster*) pairB.second->at(i);
        document()->setVisible(cluster, option->isBVisible());
    }

    for (int i = 0 ; i < _temporaryClusterList.size() ; i++)
    {
        document()->setVisible(_temporaryClusterList.at(i), option->isTMPVisible());
    }

    for (int i = 0 ; i < _trashClusterList->size() ; i++)
    {
        document()->setVisible(_trashClusterList->at(i), option->isTrashVisible());
    }

    document()->redrawGraphics();
}



GraphicsViewInterface::SelectionMode ONF_ActionModifyClustersGroups02::selectionModeToBasic(GraphicsViewInterface::SelectionMode mode) const
{
    int m = mode;

    while(m > GraphicsViewInterface::REMOVE_ONE)
        m -= GraphicsViewInterface::REMOVE_ONE;

    return (GraphicsViewInterface::SelectionMode)m;
}
