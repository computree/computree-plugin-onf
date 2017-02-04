#include "onf_adjustplotpositioncylinderdrawmanager.h"


#include <QObject>


ONF_AdjustPlotPositionCylinderDrawManager::ONF_AdjustPlotPositionCylinderDrawManager(QString drawConfigurationName) : CT_StandardAbstractShapeDrawManager(drawConfigurationName.isEmpty() ? CT_Cylinder::staticName() : drawConfigurationName)
{
    _color = Qt::red;
    _transX = 0;
    _transY = 0;
    _circles = false;
    _fixedH = false;
    _h = 3.0;
    _selectedCylinder = NULL;
}

ONF_AdjustPlotPositionCylinderDrawManager::~ONF_AdjustPlotPositionCylinderDrawManager()
{
}

void ONF_AdjustPlotPositionCylinderDrawManager::draw(GraphicsViewInterface &view, PainterInterface &painter, const CT_AbstractItemDrawable &itemDrawable) const
{
    CT_StandardAbstractShapeDrawManager::draw(view, painter, itemDrawable);

    const CT_Cylinder &item = dynamic_cast<const CT_Cylinder&>(itemDrawable);
    CT_Cylinder* cyl = (CT_Cylinder*) &item;

    Eigen::Vector3d center = item.getCenter();
    Eigen::Vector3d direction = item.getDirection();
    direction.normalize();

    center(0) += _transX;
    center(1) += _transY;

    double height = _h;

    if (!_fixedH)
    {
        height = item.getHeight();
    }

    Eigen::Vector3d bottom = center - (direction * item.getHeight()/2.0);


    QColor color = painter.getColor();
    if (&item == _selectedCylinder)
    {
        painter.setColor(_selectionColor);

    } else if (_highlightedCylinders.contains(cyl))
    {
        painter.setColor(_highlightColor);
    } else {
        painter.setColor(_color);
    }

    if(_circles)
    {
        Eigen::Vector3d level = bottom;

        for (double h = 0 ; h < height ; h += 0.50)
        {
            level(2) = h;
            painter.drawCircle3D(level, direction, item.getRadius());
        }
    } else {
        painter.drawCylinder3D(bottom, direction, item.getRadius(), height);
        painter.drawCircle3D(bottom, direction, item.getRadius());
    }

    painter.setColor(color);
}

void ONF_AdjustPlotPositionCylinderDrawManager::setTranslation(double x, double y)
{
    _transX = x;
    _transY = y;
}

void ONF_AdjustPlotPositionCylinderDrawManager::setParameters(bool circles, bool fixedH, double h)
{
    _circles = circles;
    _fixedH = fixedH;
    _h = h;
}

void ONF_AdjustPlotPositionCylinderDrawManager::setColor(QColor color)
{
    _color = color;
}

void ONF_AdjustPlotPositionCylinderDrawManager::setSelectionColor(QColor color)
{
    _selectionColor = color;
}

void ONF_AdjustPlotPositionCylinderDrawManager::setHighlightColor(QColor color)
{
    _highlightColor = color;
}

void ONF_AdjustPlotPositionCylinderDrawManager::setselectedCylinder(CT_Cylinder *selectedCylinder)
{
    _selectedCylinder = selectedCylinder;
}

void ONF_AdjustPlotPositionCylinderDrawManager::setHighlightedCylinder(QList<CT_Cylinder *> selectedCylinder)
{
    _highlightedCylinders.clear();
    _highlightedCylinders.append(selectedCylinder);
}


CT_ItemDrawableConfiguration ONF_AdjustPlotPositionCylinderDrawManager::createDrawConfiguration(QString drawConfigurationName) const
{
    CT_ItemDrawableConfiguration item = CT_ItemDrawableConfiguration(drawConfigurationName);

    item.addAllConfigurationOf(CT_StandardAbstractShapeDrawManager::createDrawConfiguration(drawConfigurationName));

    return item;
}


