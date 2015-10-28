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

#ifndef ONF_ACTIONSEGMENTCROWNS_H
#define ONF_ACTIONSEGMENTCROWNS_H


#include "views/actions/onf_actionsegmentcrownsoptions.h"
#include "ct_actions/abstract/ct_abstractactionforgraphicsview.h"
#include "ct_itemdrawable/ct_grid2dxy.h"

#include <QRect>

class ONF_ActionSegmentCrowns : public CT_AbstractActionForGraphicsView
{
    Q_OBJECT
public:

    class UndoRedoContent
    {
    public:
        UndoRedoContent();
        UndoRedoContent(const int newCluster, const QMap<size_t, int> &indices);
        UndoRedoContent(const int newCluster);
        void addIndice(const size_t indice, const int oldCluster);
        void addIndice(const size_t indice, const int oldCluster, const int newCluster);
        int                 _newCluster;
        QMap<size_t, int>   _oldClusters;
        QMap<size_t, int>   _newClusters;
        bool                _useNewClustersMap;
    };

    ONF_ActionSegmentCrowns(const CT_Grid2DXY<int>* densityGrid, const CT_Grid2DXY<double>* mnsGrid, CT_Grid2DXY<int>* clustersGrid);

    ~ONF_ActionSegmentCrowns();

    QString uniqueName() const;
    QString title() const;
    QString description() const;
    QIcon icon() const;
    QString type() const;

    const QColor *getNextColor();

    void init();    
    void initClusters();

    void drawLimit(ONF_ActionSegmentCrownsOptions *option, size_t maxCol, size_t maxRow);

    void fillCellsInList(QList<size_t> &liste, const int cluster, bool record = true);

    QList<size_t> computeColonize(size_t originColumn, size_t originRow);
    void appendIfNotNulValue(QList<size_t> &result, size_t col, size_t lin);

    void fillCluter(const size_t col, const size_t row, const int fillingCluster);
    void fillAllCluterPixels(const size_t col, const size_t row, const int fillingCluster);
    void appendIfSameCluster(QList<size_t> &result, size_t col, size_t lin, int cluster);

    bool getCoordsForMousePosition(const QMouseEvent *e, double &x, double &y);

    bool mousePressEvent(QMouseEvent *e);
    bool mouseMoveEvent(QMouseEvent *e);
    bool mouseReleaseEvent(QMouseEvent *e);
    bool wheelEvent(QWheelEvent *e);

    bool keyPressEvent(QKeyEvent *e);
    bool keyReleaseEvent(QKeyEvent *e);

    void draw(GraphicsViewInterface &view, PainterInterface &painter);
    void drawOverlay(GraphicsViewInterface &view, QPainter &painter);

    void drawPencil(PainterInterface &painter, ONF_ActionSegmentCrownsOptions *option, const double &resolution, const double &z_val);

    CT_AbstractAction* copy() const;

    void addUndoContent(UndoRedoContent* content);

public slots:
    void redrawOverlay();
    void redrawOverlayAnd3D();
    void addCluster(bool updateUi = true);
    void changeActiveClusterColor();
    void undo();
    void redo();
    void mergeClusters(int pixelNumber);

private:
    int                                     m_status;
    QList<UndoRedoContent*>                  _undoList;
    QList<UndoRedoContent*>                  _redoList;

    size_t                                  _lastIndex;
    const CT_Grid2DXY<int>*                 _densityGrid;
    const CT_Grid2DXY<double>*               _mnsGrid;
    CT_Grid2DXY<int>*                       _clustersGrid;
    size_t                                  _activeCol;
    size_t                                  _activeRow;

    QMap<int, const QColor*>                _clusters;
    int                                     _lastCluster;
    QList<QColor>                           _colorList;
    int                                     _nextColor;

    QColor*                                 _whiteColor;

};


#endif // ONF_ACTIONSEGMENTCROWNS_H
