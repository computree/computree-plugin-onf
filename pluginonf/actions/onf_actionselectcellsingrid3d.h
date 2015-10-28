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

#ifndef ONF_ACTIONSELECTCELLSINGRID3D_H
#define ONF_ACTIONSELECTCELLSINGRID3D_H


#include "views/actions/onf_actionselectcellsingrid3doptions.h"
#include "ct_actions/abstract/ct_abstractactionforgraphicsview.h"

#include "ct_itemdrawable/abstract/ct_abstractgrid3d.h"
#include "ct_itemdrawable/ct_grid3d.h"

#include <QRect>

class ONF_ActionSelectCellsInGrid3D : public CT_AbstractActionForGraphicsView
{
    Q_OBJECT
public:
    ONF_ActionSelectCellsInGrid3D(const CT_AbstractGrid3D* refGrid, CT_Grid3D<bool>* boolGrid);

    QString uniqueName() const;
    QString title() const;
    QString description() const;
    QIcon icon() const;
    QString type() const;

    void init();

    void initActiveGridForLevel();

    bool mousePressEvent(QMouseEvent *e);
    bool mouseMoveEvent(QMouseEvent *e);
    bool mouseReleaseEvent(QMouseEvent *e);
    bool wheelEvent(QWheelEvent *e);

    bool keyPressEvent(QKeyEvent *e);
    bool keyReleaseEvent(QKeyEvent *e);

    void draw(GraphicsViewInterface &view, PainterInterface &painter);
    void drawOverlay(GraphicsViewInterface &view, QPainter &painter);

    QList<size_t> computeExtends(size_t originColumn, size_t originRow);
    QList<size_t> computeColonize(size_t originColumn, size_t originRow);

    QList<size_t> computeExtends3D(size_t index);
    QList<size_t> computeColonize3D(size_t index);

    CT_AbstractAction* copy() const;

public slots:
    void updateLevel(int level);
    void updateThickness(int thickness);
    void updateDrawing();
    void updateCumulMode(ONF_ActionSelectCellsInGrid3DOptions::CumulateMode mode);
    void updateSelectionMode(ONF_ActionSelectCellsInGrid3DOptions::GridSelectionMode mode);
    void copyUpwards();
    void copyFull();
    void change2D3DMode(bool mode3D);
    void changeCoef(double coef);
    void changeGridColor(QColor color);
    void selectNotEmptyCells();
    void createConvexHull();

private:
    int                                     m_status;
    size_t                                  _lastIndex;
    bool                                    _validLastIndex;
    ONF_ActionSelectCellsInGrid3DOptions::GridSelectionMode  _previewMode;
    ONF_ActionSelectCellsInGrid3DOptions::CumulateMode       _cumulMode;
    QList<size_t>                           _candidates;
    bool                                    _refState;
    bool                                    _mode3D;


    const CT_AbstractGrid3D*                _refGrid;
    CT_Grid3D<bool>*                        _boolGrid;
    std::vector<float>                      _active2DGrid;
    float                                   _activeMin;
    float                                   _activeMax;
    size_t                                  _level;
    size_t                                  _thickness;
    float                                   _reductionCoef;
    QColor                                  _gridColor;

    bool getCoordsInActiveLevelForMousePosition(const QMouseEvent *e, float &x, float &y, float &z);
    bool getIndexInGridForMousePosition(const QMouseEvent *e, size_t &index);
    void updateSelection(const float x, const float y, const float z);
    void updateSelection(const size_t index);
    void copyLevel(size_t refLevel, size_t first, size_t last);
    void draw2D(GraphicsViewInterface &view, PainterInterface &painter);
    void draw3D(GraphicsViewInterface &view, PainterInterface &painter);
    void appendIfNotNulValue2D(QList<size_t> &result, size_t col, size_t lin);
    void appendIfCoherentRefState(QList<size_t> &result, size_t col, size_t lin, size_t lev);
    void appendIfNotNulValue3D(QList<size_t> &result, size_t col, size_t lin, size_t lev);


};


#endif // ONF_ACTIONSELECTCELLSINGRID3D_H
