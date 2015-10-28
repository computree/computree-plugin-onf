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

#include "actions/onf_actionsegmentgaps.h"
#include "ct_global/ct_context.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QIcon>
#include <QPainter>
#include <QDebug>

#include <limits>

#include "ct_shape2ddata/ct_polygon2ddata.h"


ONF_ActionSegmentGaps::UndoRedoContent::UndoRedoContent()
{
    _useNewClustersMap = true;
}

ONF_ActionSegmentGaps::UndoRedoContent::UndoRedoContent(const int newCluster, const QMap<size_t, int> &indices)
{
    _newCluster = newCluster;
    _oldClusters = indices;
    _useNewClustersMap = false;
}

ONF_ActionSegmentGaps::UndoRedoContent::UndoRedoContent(const int newCluster)
{
    _newCluster = newCluster;
    _useNewClustersMap = false;
}

void ONF_ActionSegmentGaps::UndoRedoContent::addIndice(const size_t indice, const int oldCluster)
{
    _oldClusters.insert(indice, oldCluster);
    _useNewClustersMap = false;
}

void ONF_ActionSegmentGaps::UndoRedoContent::addIndice(const size_t indice, const int oldCluster, const int newCluster)
{
    _oldClusters.insert(indice, oldCluster);
    _newClusters.insert(indice, newCluster);
    _useNewClustersMap = true;
}

ONF_ActionSegmentGaps::ONF_ActionSegmentGaps(const CT_Grid2DXY<int> *densityGrid, const CT_Grid2DXY<double> *mnsGrid, CT_Grid2DXY<int> *clustersGrid, bool keepOnlyConvexHull) : CT_AbstractActionForGraphicsView()
{
    _densityGrid = densityGrid;
    _mnsGrid = mnsGrid;
    _clustersGrid = clustersGrid;
    _lastCluster = 0;
    _lastIndex = 0;

    _keepOnlyConvexHull = keepOnlyConvexHull;

     _whiteColor = new QColor(255,255,255);

    _activeCol = 0;
    _activeRow = 0;

    // Creation de la liste de couleurs de clusters
    _colorList.append(QColor(255,255,200)); // Jaune Clair
    _colorList.append(QColor(255,200,255)); // Magenta Clair
    _colorList.append(QColor(200,255,255)); // Cyan Clair
    _colorList.append(QColor(200,200,255)); // Mauve Clair
    _colorList.append(QColor(200,255,200)); // Vert Clair
    _colorList.append(QColor(255,200,200)); // Rouge Clair
    _colorList.append(QColor(255,200,150)); // Orange clair
    _colorList.append(QColor(150,200,255)); // Bleu Clair
    _colorList.append(QColor(200,255,150)); // Vert-Jaune Clair
    _colorList.append(QColor(150,255,200)); // Turquoise Clair
    _colorList.append(QColor(255,150,200)); // Rose Clair
    _colorList.append(QColor(200,150,255)); // Violet Clair
    _colorList.append(QColor(255,255,0  )); // Jaune
    _colorList.append(QColor(255,0  ,255)); // Magenta
    _colorList.append(QColor(0  ,255,255)); // Cyan
    _colorList.append(QColor(0  ,0  ,255)); // Mauve
    _colorList.append(QColor(0  ,255,0  )); // Vert
    _colorList.append(QColor(255,150,0  )); // Orange
    _colorList.append(QColor(0  ,150,255)); // Bleu
    _colorList.append(QColor(150,255,0  )); // Vert-Jaune
    _colorList.append(QColor(0  ,255,150)); // Turquoise
    _colorList.append(QColor(255,0  ,150)); // Rose
    _colorList.append(QColor(150,0  ,255)); // Violet

    _nextColor = 0;
}

ONF_ActionSegmentGaps::~ONF_ActionSegmentGaps()
{
    delete _whiteColor;

    qDeleteAll(_undoList);
    qDeleteAll(_redoList);
}

QString ONF_ActionSegmentGaps::uniqueName() const
{
    return "ONF_ActionSegmentGaps";
}

QString ONF_ActionSegmentGaps::title() const
{
    return "Segment crowns";
}

QString ONF_ActionSegmentGaps::description() const
{
    return "Segmente les houppiers";
}

QIcon ONF_ActionSegmentGaps::icon() const
{
    return QIcon(":/icons/limits.png");
}

QString ONF_ActionSegmentGaps::type() const
{
    return CT_AbstractAction::TYPE_MODIFICATION;
}

const QColor* ONF_ActionSegmentGaps::getNextColor()
{
    int indice = _nextColor++;
    if (_nextColor >= _colorList.size()) {_nextColor = 0;}
    return &(_colorList.at(indice));
}


void ONF_ActionSegmentGaps::init()
{
    CT_AbstractActionForGraphicsView::init();

    m_status = 0;

    if(nOptions() == 0)
    {
        // create the option widget if it was not already created
        ONF_ActionSegmentGapsOptions *option = new ONF_ActionSegmentGapsOptions(this, _mnsGrid->level());

        option->setMode(ONF_ActionSegmentGapsOptions::FREEMOVE);
        option->setDrawingGrid(ONF_ActionSegmentGapsOptions::CLUSTERS);

        // add the options to the graphics view
        graphicsView()->addActionOptions(option);

        connect(option, SIGNAL(askForClusterCreation()), this, SLOT(addCluster()));
        connect(option, SIGNAL(parametersChanged()), this, SLOT(redrawOverlayAnd3D()));
        connect(option, SIGNAL(activeClusterChanged()), this, SLOT(changeActiveClusterColor()));
        connect(option, SIGNAL(undo()), this, SLOT(undo()));
        connect(option, SIGNAL(redo()), this, SLOT(redo()));
        connect(option, SIGNAL(askForMerging(int)), this, SLOT(mergeClusters(int)));

        // register the option to the superclass, so the hideOptions and showOptions
        // is managed automatically
        registerOption(option);

        initClusters();

        option->setActiveCluster(0);
        changeActiveClusterColor();

        redrawOverlayAnd3D();
    }
}

void ONF_ActionSegmentGaps::redrawOverlay()
{
    document()->redrawGraphics();
}

void ONF_ActionSegmentGaps::redrawOverlayAnd3D()
{
    setDrawing3DChanged();
}

void ONF_ActionSegmentGaps::initClusters()
{
    PS_LOG->addMessage(LogInterface::info, LogInterface::action, QString("ONF_ActionSegmentGaps : Initialisation des clusters"));

    ONF_ActionSegmentGapsOptions *option = (ONF_ActionSegmentGapsOptions*)optionAt(0);

    _clustersGrid->initGridWithValue(-1);

    _clusters.clear();
    _lastCluster = -1;

    _clusters.insert(++_lastCluster, _whiteColor); // cluster 0 : couleur unique = blanc

    size_t colDim = _clustersGrid->colDim();
    size_t linDim = _clustersGrid->linDim();

    if (_keepOnlyConvexHull)
    {
        QList<Eigen::Vector2d*> filledCells;

        // add all filled cell coordinates in a list (for convex hull computation)
        for (size_t cx = 0 ; cx < colDim ; cx++)
        {
            for (size_t ly = 0 ; ly < linDim ; ly++)
            {
                int value = _densityGrid->value(cx, ly);
                if (value > 0)
                {
                    filledCells.append(new Eigen::Vector2d(_densityGrid->getCellCenterColCoord(cx), _densityGrid->getCellCenterLinCoord(ly)));
                }
            }
        }

        // compute convex hull
        CT_Polygon2DData* polygonData = CT_Polygon2DData::createConvexHull(filledCells);

        // exclude au cells outside of the convex hull
        for (size_t cx = 0 ; cx < colDim ; cx++)
        {
            double xx = _clustersGrid->getCellCenterColCoord(cx);

            for (size_t ly = 0 ; ly < linDim ; ly++)
            {
                double yy = _clustersGrid->getCellCenterLinCoord(ly);

                if (!polygonData->contains(xx, yy))
                {
                    _clustersGrid->setValue(cx, ly, -2);
                }
            }
        }
    }

    for (size_t cx = 0 ; cx < colDim ; cx++)
    {
        for (size_t ly = 0 ; ly < linDim ; ly++)
        {
            int cluster = _clustersGrid->value(cx, ly);

            if (cluster == -1)
            {
                QList<size_t> liste = computeColonize(cx, ly);

                if (liste.size() > 0)
                {
                    addCluster(false);
                    fillCellsInList(liste, _lastCluster, false);
                }
            }
        }
    }




    option->setClusterNumber(_lastCluster + 1);
    option->setActiveCluster(_lastCluster);

    redrawOverlayAnd3D();
}

void ONF_ActionSegmentGaps::drawLimit(ONF_ActionSegmentGapsOptions *option, size_t maxCol, size_t maxRow)
{
    int activeCluster = option->getActiveCluster();
    UndoRedoContent *content = new UndoRedoContent(activeCluster);

    for (size_t c = _activeCol ; c <= maxCol ; c++)
    {
        for (size_t r = _activeRow ; r <= maxRow ; r++)
        {
            size_t index;
            _clustersGrid->index(c, r, index);
            int oldCluster = _clustersGrid->valueAtIndex(index);
            if (oldCluster >= 0)
            {
                _clustersGrid->setValueAtIndex(index, activeCluster);
                content->addIndice(index, oldCluster);
            }
        }
    }

    if (content->_oldClusters.isEmpty())
    {
        delete content;
    } else {
        addUndoContent(content);
    }
}


void ONF_ActionSegmentGaps::fillCellsInList(QList<size_t> &liste, const int cluster, bool record)
{
    if (liste.isEmpty()) {return;}

    UndoRedoContent* content;
    if (record) {content = new UndoRedoContent(cluster);}

    for (int i = 0 ; i < liste.size() ; i++)
    {
        size_t index = liste.at(i);        
        if (record) { content->addIndice(index, _clustersGrid->valueAtIndex(index));}
        _clustersGrid->setValueAtIndex(index, cluster);
    }    
    if (record) {addUndoContent(content);}
}

QList<size_t> ONF_ActionSegmentGaps::computeColonize(size_t originColumn, size_t originRow)
{
    QList<size_t> result;
    size_t index;

    if (!_clustersGrid->index(originColumn, originRow, index))
    {
        return result;
    }

    if (_densityGrid->valueAtIndex(index) == _densityGrid->NA() && (_clustersGrid->valueAtIndex(index) != -2)) {result.append(index);}

    int i = 0;
    while (i < result.size())
    {
        size_t current_col, current_row;
        _clustersGrid->indexToGrid(result.at(i), current_col, current_row);

        appendIfNotNulValue(result, current_col - 1, current_row);
        appendIfNotNulValue(result, current_col, current_row - 1);
        appendIfNotNulValue(result, current_col + 1, current_row);
        appendIfNotNulValue(result, current_col, current_row + 1);

        ++i;
    }

    return result;
}

void ONF_ActionSegmentGaps::appendIfNotNulValue(QList<size_t> &result, size_t col, size_t lin)
{
    size_t index;
    if (_clustersGrid->index(col, lin, index))
    {
        if (_densityGrid->valueAtIndex(index) == _densityGrid->NA()  && (_clustersGrid->valueAtIndex(index) != -2) && !result.contains(index))
        {
            result.append(index);
        }
    }
}

void ONF_ActionSegmentGaps::fillCluter(const size_t col, const size_t row, const int fillingCluster)
{
    QList<size_t> result;
    size_t index;

    if (!_clustersGrid->index(col, row, index)) {return;}

    int cluster = _clustersGrid->valueAtIndex(index);

    if (cluster >= 0 && cluster <= _lastCluster && cluster != fillingCluster)
    {
        result.append(index);
    }

    int i = 0;
    while (i < result.size())
    {
        size_t current_col, current_row;
        _clustersGrid->indexToGrid(result.at(i), current_col, current_row);

        appendIfSameCluster(result, current_col - 1, current_row, cluster);
        appendIfSameCluster(result, current_col, current_row - 1, cluster);
        appendIfSameCluster(result, current_col + 1, current_row, cluster);
        appendIfSameCluster(result, current_col, current_row + 1, cluster);

        ++i;
    }

    if (result.size() > 0)
    {
        fillCellsInList(result, fillingCluster);
    }
}

void ONF_ActionSegmentGaps::fillAllCluterPixels(const size_t col, const size_t row, const int fillingCluster)
{
    size_t index;
    if (!_clustersGrid->index(col, row, index)) {return;}

    int cluster = _clustersGrid->valueAtIndex(index);

    UndoRedoContent *content = new UndoRedoContent(fillingCluster);

    if (cluster >= 0 && cluster <= _lastCluster && cluster != fillingCluster)
    {
        size_t ncells = _clustersGrid->nCells();
        for (size_t i = 0 ; i < ncells ; i++)
        {
            int oldCluster = _clustersGrid->valueAtIndex(i);
            if (oldCluster == cluster)
            {
                _clustersGrid->setValueAtIndex(i, fillingCluster);
                content->addIndice(i, oldCluster);
            }
        }
    }

    if (content->_oldClusters.size() > 0)
    {
        addUndoContent(content);
    } else {
        delete content;
    }
}

void ONF_ActionSegmentGaps::appendIfSameCluster(QList<size_t> &result, size_t col, size_t lin, int cluster)
{
    size_t index;
    if (_clustersGrid->index(col, lin, index))
    {
        int current_cluster = _clustersGrid->valueAtIndex(index);
        if (current_cluster == cluster &&
                !result.contains(index))
        {
            result.append(index);
        }
    }
}

void ONF_ActionSegmentGaps::addCluster(bool updateUi)
{
    _clusters.insert(++_lastCluster, getNextColor());

    if (updateUi)
    {
        ONF_ActionSegmentGapsOptions *option = (ONF_ActionSegmentGapsOptions*)optionAt(0);
        option->setActiveCluster(_lastCluster);
        option->setClusterNumber(_lastCluster + 1);
    }
}

void ONF_ActionSegmentGaps::changeActiveClusterColor()
{
    ONF_ActionSegmentGapsOptions *option = (ONF_ActionSegmentGapsOptions*)optionAt(0);
    int cluster = option->getActiveCluster();
    const QColor *color = _clusters.value(cluster);    
    option->setActiveClusterColor(color);
}

void ONF_ActionSegmentGaps::mergeClusters(int pixelNumber)
{
    QMap<int, size_t> clustersSize;
    QMap<int, double> clustersX;
    QMap<int, double> clustersY;

    size_t ncells = _clustersGrid->nCells();
    for (size_t i = 0 ; i < ncells ; i++)
    {
        int cluster = _clustersGrid->valueAtIndex(i);
        if (cluster >= 0)
        {
            size_t col, row;
            _clustersGrid->indexToGrid(i, col, row);
            clustersX.insert(cluster, clustersX.value(cluster, 0) + _clustersGrid->getCellCenterColCoord(col));
            clustersY.insert(cluster, clustersY.value(cluster, 0) + _clustersGrid->getCellCenterLinCoord(row));
            clustersSize.insert(cluster, clustersSize.value(cluster, 0) + 1);
        }
    }

    QList<int> smallClusters;
    QList<int> bigClusters;

    QMutableMapIterator<int, size_t> itN(clustersSize);
    QMutableMapIterator<int, double> itX(clustersX);
    QMutableMapIterator<int, double> itY(clustersY);

    while (itX.hasNext())
    {
        itX.next();
        itY.next();
        itN.next();

        size_t size = itN.value();

        itX.setValue(itX.value() / size);
        itY.setValue(itY.value() / size);

        if (size <= pixelNumber)
        {
            smallClusters.append(itN.key());
        } else {
            bigClusters.append(itN.key());
        }
    }

    QMap<int, int> correspondances;

    for (int s = 0 ; s < smallClusters.size() ; s++)
    {
        int smallI = smallClusters.at(s);
        double smallX = clustersX.value(smallI);
        double smallY = clustersY.value(smallI);
        int newCluster = -1;
        double smallestDistance = std::numeric_limits<double>::max();

        for (int b = 0 ; b < bigClusters.size() ; b++)
        {
            int big = bigClusters.at(b);
            double bigX = clustersX.value(big);
            double bigY = clustersY.value(big);

            double distance = pow(smallX - bigX, 2) + pow(smallY - bigY, 2);
            if (distance < smallestDistance)
            {
                smallestDistance = distance;
                newCluster = big;
            }
        }
        if (newCluster >= 0)
        {
            correspondances.insert(smallI, newCluster);
        }
    }

    if (correspondances.size() > 0)
    {
        // TODO : gérer le REDO ...
        UndoRedoContent *content = new UndoRedoContent();
        for (size_t i = 0 ; i < ncells ; i++)
        {
            int cluster = _clustersGrid->valueAtIndex(i);
            if (cluster >= 0)
            {
                int newCluster = correspondances.value(cluster, -1);
                if (newCluster >= 0)
                {
                    _clustersGrid->setValueAtIndex(i, newCluster);

                    content->addIndice(i, cluster, newCluster);
                }
            }
        }
        addUndoContent(content);
    }

    redrawOverlayAnd3D();
}


void ONF_ActionSegmentGaps::addUndoContent(UndoRedoContent* content)
{
    ONF_ActionSegmentGapsOptions *option = (ONF_ActionSegmentGapsOptions*)optionAt(0);

    _undoList.append(content);
    qDeleteAll(_redoList);
    _redoList.clear();
    option->setUndoRedo(!_undoList.isEmpty(), !_redoList.isEmpty());
}

void ONF_ActionSegmentGaps::undo()
{
    ONF_ActionSegmentGapsOptions *option = (ONF_ActionSegmentGapsOptions*)optionAt(0);

    if (!_undoList.isEmpty())
    {
        UndoRedoContent *undoContent = _undoList.takeLast();

        QMapIterator<size_t, int> it(undoContent->_oldClusters);
        while (it.hasNext())
        {
            it.next();
            _clustersGrid->setValueAtIndex(it.key(), it.value());
        }
        _redoList.append(undoContent);        
        option->setUndoRedo(!_undoList.isEmpty(), !_redoList.isEmpty());
        redrawOverlayAnd3D();
    }
}

void ONF_ActionSegmentGaps::redo()
{
    ONF_ActionSegmentGapsOptions *option = (ONF_ActionSegmentGapsOptions*)optionAt(0);

    if (!_redoList.isEmpty())
    {
        UndoRedoContent *redoContent = _redoList.takeLast();

        if (redoContent->_useNewClustersMap)
        {
            QMapIterator<size_t, int> it(redoContent->_newClusters);
            while (it.hasNext())
            {
                it.next();
                _clustersGrid->setValueAtIndex(it.key(), it.value());
            }
        } else {
            QMapIterator<size_t, int> it(redoContent->_oldClusters);
            while (it.hasNext())
            {
                it.next();
                _clustersGrid->setValueAtIndex(it.key(), redoContent->_newCluster);
            }
        }

        _undoList.append(redoContent);
        option->setUndoRedo(!_undoList.isEmpty(), !_redoList.isEmpty());
        redrawOverlayAnd3D();
    }
}

bool ONF_ActionSegmentGaps::getCoordsForMousePosition(const QMouseEvent *e, double &x, double &y)
{
    ONF_ActionSegmentGapsOptions *option = (ONF_ActionSegmentGapsOptions*)optionAt(0);

    Eigen::Vector3d origin, direction;
    GraphicsViewInterface *view = graphicsView();
    view->convertClickToLine(e->pos(), origin, direction);

    if (direction.z() == 0) {return false;}

    double coef = (option->getHeight() - _clustersGrid->resolution()/2.0 - origin.z())/direction.z();

    x = origin.x() + coef*direction.x();
    y = origin.y() + coef*direction.y();

    size_t index;
    return _clustersGrid->indexAtCoords(x, y, index);
}

bool ONF_ActionSegmentGaps::mousePressEvent(QMouseEvent *e)
{
    ONF_ActionSegmentGapsOptions *option = (ONF_ActionSegmentGapsOptions*)optionAt(0);

    if ((e->buttons() & Qt::LeftButton))
    {
        double x, y;
        if (getCoordsForMousePosition(e, x, y))
        {
            _clustersGrid->indexAtCoords(x, y, _lastIndex);

            if (option->getMode() == ONF_ActionSegmentGapsOptions::CHOOSECLUSTER)
            {
                int cluster = _clustersGrid->valueAtXY(x, y);
                if (cluster >= 0)
                {
                    option->setActiveCluster(cluster);
                }
                redrawOverlayAnd3D();
                return true;
            } else if (option->getMode() == ONF_ActionSegmentGapsOptions::CHANGECENTERCELL)
            {
                CameraInterface* camera = graphicsView()->camera();
                camera->setCX(x);
                camera->setCY(y);
                camera->setCZ(option->getHeight());

                QString baseStr = tr("Caméra centrée en");
                PS_LOG->addMessage(LogInterface::info, LogInterface::action, QString("%1 x=%2, y=%3, z=%4").arg(baseStr).arg(x).arg(y).arg(option->getHeight()));

                option->setMode(ONF_ActionSegmentGapsOptions::FREEMOVE);

                return true;
            } else
            {
                _clustersGrid->col(x, _activeCol);
                _clustersGrid->lin(y, _activeRow);

                int size = option->getPencilSize();
                size_t maxCol = _activeCol + size - 1;
                size_t maxRow = _activeRow + size - 1;

                if (maxCol >= _clustersGrid->colDim()) {maxCol = _clustersGrid->colDim() - 1;}
                if (maxRow >= _clustersGrid->linDim()) {maxRow = _clustersGrid->linDim() - 1;}

                if (option->getMode() == ONF_ActionSegmentGapsOptions::DRAWLIMITS)
                {
                    drawLimit(option, maxCol, maxRow);
                    redrawOverlayAnd3D();
                    return true;
                } else if (option->getMode() == ONF_ActionSegmentGapsOptions::FILLAREA)
                {
                    for (size_t c = _activeCol ; c <= maxCol ; c++)
                    {
                        for (size_t r = _activeRow ; r <= maxRow ; r++)
                        {
                            if (option->isFillModeFull())
                            {
                                fillAllCluterPixels(c, r, option->getActiveCluster());
                            } else {
                                fillCluter(c, r, option->getActiveCluster());
                            }
                        }
                    }

                    redrawOverlayAnd3D();
                    return true;
                }
            }
        }
    }
    return false;
}

bool ONF_ActionSegmentGaps::mouseMoveEvent(QMouseEvent *e)
{
    ONF_ActionSegmentGapsOptions *option = (ONF_ActionSegmentGapsOptions*)optionAt(0);

    double x, y;
    if (getCoordsForMousePosition(e, x, y))
    {
        size_t index;
        _clustersGrid->indexAtCoords(x, y, index);

        if (index != _lastIndex)
        {
            _clustersGrid->col(x, _activeCol);
            _clustersGrid->lin(y, _activeRow);

            if ((e->buttons() & Qt::LeftButton))
            {

                int size = option->getPencilSize();
                size_t maxCol = _activeCol + size - 1;
                size_t maxRow = _activeRow + size - 1;

                if (maxCol >= _clustersGrid->colDim()) {maxCol = _clustersGrid->colDim() - 1;}
                if (maxRow >= _clustersGrid->linDim()) {maxRow = _clustersGrid->linDim() - 1;}

                if (option->getMode() == ONF_ActionSegmentGapsOptions::DRAWLIMITS)
                {
                    drawLimit(option, maxCol, maxRow);
                    redrawOverlayAnd3D();
                    return true;
                } else if (option->getMode() == ONF_ActionSegmentGapsOptions::FILLAREA)
                {
                    for (size_t c = _activeCol ; c <= maxCol ; c++)
                    {
                        for (size_t r = _activeRow ; r <= maxRow ; r++)
                        {
                            if (option->isFillModeFull())
                            {
                                fillAllCluterPixels(c, r, option->getActiveCluster());
                            } else {
                                fillCluter(c, r, option->getActiveCluster());
                            }
                        }
                    }

                    redrawOverlayAnd3D();
                    return true;
                }
            }
            redrawOverlay();
        }
    }
    return false;
}

bool ONF_ActionSegmentGaps::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    return false;
}

bool ONF_ActionSegmentGaps::wheelEvent(QWheelEvent *e)
{
    ONF_ActionSegmentGapsOptions *option = (ONF_ActionSegmentGapsOptions*)optionAt(0);

    if (e->modifiers()  & Qt::ControlModifier)
    {
        if (e->delta()>0)
        {
            option->increaseHValue();
        } else {
            option->decreaseHValue();
        }
        return true;
    } else if (e->modifiers()  & Qt::ShiftModifier)
    {
        if (e->delta()>0)
        {
            option->increasePencilSizeValue();
        } else {
            option->decreasePencilSizeValue();
        }
        return true;
    }

    return false;
}

bool ONF_ActionSegmentGaps::keyPressEvent(QKeyEvent *e)
{
    ONF_ActionSegmentGapsOptions *option = (ONF_ActionSegmentGapsOptions*)optionAt(0);

    if((e->key() == Qt::Key_S) && !e->isAutoRepeat())
    {
        option->setMode(ONF_ActionSegmentGapsOptions::CHOOSECLUSTER);
        return true;
    } else if((e->key() == Qt::Key_D) && !e->isAutoRepeat())
    {
        option->setMode(ONF_ActionSegmentGapsOptions::DRAWLIMITS);
        return true;
    } else if((e->key() == Qt::Key_F) && !e->isAutoRepeat())
    {
        option->setMode(ONF_ActionSegmentGapsOptions::FILLAREA);
        return true;
    } else if((e->key() == Qt::Key_G) && !e->isAutoRepeat())
    {
        option->setMode(ONF_ActionSegmentGapsOptions::FREEMOVE);
        return true;
    } else if ((e->key() == Qt::Key_Z) && (e->modifiers()  & Qt::ControlModifier))
    {
        undo();
        return true;
    } else if ((e->key() == Qt::Key_Y) && (e->modifiers()  & Qt::ControlModifier))
    {
        redo();
        return true;
    }

    return false;
}

bool ONF_ActionSegmentGaps::keyReleaseEvent(QKeyEvent *e)
{
    Q_UNUSED(e);
    return false;
}

void ONF_ActionSegmentGaps::draw(GraphicsViewInterface &view, PainterInterface &painter)
{
    Q_UNUSED(view)

    ONF_ActionSegmentGapsOptions *option = (ONF_ActionSegmentGapsOptions*)optionAt(0);

    painter.save();

    double z_val = option->getHeight();
    double resolution = _clustersGrid->resolution();

    double min = 0;
    double amplitude = 0;

    if (option->getDrawingGrid() == ONF_ActionSegmentGapsOptions::DENSITY)
    {
        min = _densityGrid->dataMin();
        amplitude = _densityGrid->dataMax() - min;
    }
    else {
        min = _mnsGrid->dataMin();
        amplitude = _mnsGrid->dataMax() - min;
    }

    for (size_t cx = 0 ; cx < _clustersGrid->colDim() ; cx++)
    {
        for (size_t ly = 0 ; ly < _clustersGrid->linDim() ; ly++)
        {
            double x = (cx + 0.5) * resolution + _clustersGrid->minX();
            double y = (ly + 0.5) * resolution + _clustersGrid->minY();
            int cluster = _clustersGrid->value(cx, ly);


            double value = 0;

            if (option->getDrawingGrid() == ONF_ActionSegmentGapsOptions::DENSITY)
            {
                value = _densityGrid->value(cx, ly);
            }
            else if (option->getDrawingGrid() == ONF_ActionSegmentGapsOptions::HEIGHT) {
                value = _mnsGrid->value(cx, ly);
            }

            if (value <= 0)
            {
                if (cluster < 0 || cluster > _lastCluster)
                {
                    painter.setColor(QColor(0, 0, 0));
                } else if (option->redForSelection() && (cluster == option->getActiveCluster()))
                {
                    painter.setColor(QColor(255, 0, 0));
                } else
                {
                    const QColor* color = _clusters.value(cluster);
                    painter.setColor(*color);
                }
            } else {
                int colorLevel = ((value - min) / amplitude)*255;

                if (cluster == option->getActiveCluster())
                {
                    painter.setColor(QColor(255, colorLevel, colorLevel));
                } else
                {
                    painter.setColor(QColor(colorLevel, colorLevel, colorLevel));
                }
            }

            if (!option->getShowClustersOnly() || (cluster >= 0 && cluster <= _lastCluster))
            {
                Eigen::Vector2d topLeft(x - 0.5*resolution, y + 0.5*resolution);
                Eigen::Vector2d bottomRight(x + 0.5*resolution, y - 0.5*resolution);
                painter.fillRectXY(topLeft, bottomRight, z_val);
            }
        }
    }

    if (option->getMode() == ONF_ActionSegmentGapsOptions::DRAWLIMITS ||
            option->getMode() == ONF_ActionSegmentGapsOptions::FILLAREA)
    {
        painter.setColor(QColor(255, 0, 0));
        drawPencil(painter, option, resolution, z_val);
    }

    painter.restore();
}

void ONF_ActionSegmentGaps::drawPencil(PainterInterface &painter, ONF_ActionSegmentGapsOptions *option, const double &resolution, const double &z_val)
{
    double delta = (0.5 + option->getPencilSize() / 2.0)*resolution;

    double x = _clustersGrid->getCellCenterColCoord(_activeCol);
    double y = _clustersGrid->getCellCenterLinCoord(_activeRow);

    Eigen::Vector2d topLeft(x - delta, y + delta);
    Eigen::Vector2d bottomRight(x + delta, y - delta);

    painter.drawRectXY(topLeft, bottomRight, z_val + 0.0001);
}

void ONF_ActionSegmentGaps::drawOverlay(GraphicsViewInterface &view, QPainter &painter)
{
    Q_UNUSED(view);
    Q_UNUSED(painter);
}

CT_AbstractAction *ONF_ActionSegmentGaps::copy() const
{
    return new ONF_ActionSegmentGaps(_densityGrid, _mnsGrid, _clustersGrid, _keepOnlyConvexHull);
}
