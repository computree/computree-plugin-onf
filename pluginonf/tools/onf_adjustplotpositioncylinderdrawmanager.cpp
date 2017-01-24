#include "onf_adjustplotpositioncylinderdrawmanager.h"

#include "ct_itemdrawable/ct_cylinder.h"

#include <QObject>

const QString ONF_AdjustPlotPositionCylinderDrawManager::INDEX_CONFIG_DRAW_CYLINDER = ONF_AdjustPlotPositionCylinderDrawManager::staticInitConfigDrawCylinder();
const QString ONF_AdjustPlotPositionCylinderDrawManager::INDEX_CONFIG_DRAW_AXE = ONF_AdjustPlotPositionCylinderDrawManager::staticInitConfigDrawAxe();
const QString ONF_AdjustPlotPositionCylinderDrawManager::INDEX_CONFIG_DRAW_BASE_CIRCLE = ONF_AdjustPlotPositionCylinderDrawManager::staticInitConfigDrawBaseCircle();
const QString ONF_AdjustPlotPositionCylinderDrawManager::INDEX_CONFIG_DRAW_SET_FIXED_HEIGHT = ONF_AdjustPlotPositionCylinderDrawManager::staticInitConfigSetFixedHeight();
const QString ONF_AdjustPlotPositionCylinderDrawManager::INDEX_CONFIG_HEIGHT = ONF_AdjustPlotPositionCylinderDrawManager::staticInitConfigHeight();
const QString ONF_AdjustPlotPositionCylinderDrawManager::INDEX_CONFIG_TRANSPARENCY_LEVEL = ONF_AdjustPlotPositionCylinderDrawManager::staticInitConfigTransparencyLevel();

ONF_AdjustPlotPositionCylinderDrawManager::ONF_AdjustPlotPositionCylinderDrawManager(QString drawConfigurationName) : CT_StandardAbstractShapeDrawManager(drawConfigurationName.isEmpty() ? CT_Cylinder::staticName() : drawConfigurationName)
{
    _color = Qt::red;
    _transX = 0;
    _transY = 0;
}

ONF_AdjustPlotPositionCylinderDrawManager::~ONF_AdjustPlotPositionCylinderDrawManager()
{
}

void ONF_AdjustPlotPositionCylinderDrawManager::draw(GraphicsViewInterface &view, PainterInterface &painter, const CT_AbstractItemDrawable &itemDrawable) const
{
    CT_StandardAbstractShapeDrawManager::draw(view, painter, itemDrawable);

    const CT_Cylinder &item = dynamic_cast<const CT_Cylinder&>(itemDrawable);

    Eigen::Vector3d center = item.getCenter();
    Eigen::Vector3d direction = item.getDirection();
    direction.normalize();

    center(0) += _transX;
    center(1) += _transY;

    bool fixedHeight = getDrawConfiguration()->getVariableValue(INDEX_CONFIG_DRAW_SET_FIXED_HEIGHT).toBool();
    double height = getDrawConfiguration()->getVariableValue(INDEX_CONFIG_HEIGHT).toDouble();

    if (!fixedHeight)
    {
        height = item.getHeight();
    }

    Eigen::Vector3d bottom = center - (direction * item.getHeight()/2.0);

    int transparence = getDrawConfiguration()->getVariableValue(INDEX_CONFIG_TRANSPARENCY_LEVEL).toInt();
    if (transparence < 0) {transparence = 0;}
    if (transparence > 255) {transparence = 255;}

    QColor color = painter.getColor();
    painter.setColor(QColor(_color.red(), _color.green(), _color.blue(), transparence));

    if(getDrawConfiguration()->getVariableValue(INDEX_CONFIG_DRAW_CYLINDER).toBool())
    {
        painter.drawCylinder3D(bottom, direction, item.getRadius(), height);
    }

    if(getDrawConfiguration()->getVariableValue(INDEX_CONFIG_DRAW_AXE).toBool())
    {
        painter.drawLine(bottom.x(), bottom.y(), bottom.z(), height*direction.x()+bottom.x(), height*direction.y()+bottom.y(), height*direction.z()+bottom.z());
    }

    if(getDrawConfiguration()->getVariableValue(INDEX_CONFIG_DRAW_BASE_CIRCLE).toBool())
    {
        painter.drawCircle3D(bottom, direction, item.getRadius());
    }

    painter.setColor(color);
}

void ONF_AdjustPlotPositionCylinderDrawManager::setTranslation(double x, double y)
{
    _transX = x;
    _transY = y;
}

void ONF_AdjustPlotPositionCylinderDrawManager::setColor(QColor color)
{
    _color = color;
}


CT_ItemDrawableConfiguration ONF_AdjustPlotPositionCylinderDrawManager::createDrawConfiguration(QString drawConfigurationName) const
{
    CT_ItemDrawableConfiguration item = CT_ItemDrawableConfiguration(drawConfigurationName);

    item.addAllConfigurationOf(CT_StandardAbstractShapeDrawManager::createDrawConfiguration(drawConfigurationName));
    item.addNewConfiguration(ONF_AdjustPlotPositionCylinderDrawManager::staticInitConfigDrawCylinder() ,QObject::tr("Dessiner le cylindre"), CT_ItemDrawableConfiguration::Bool, true);
    item.addNewConfiguration(ONF_AdjustPlotPositionCylinderDrawManager::staticInitConfigDrawAxe() ,QObject::tr("Dessiner l'axe"), CT_ItemDrawableConfiguration::Bool, false);
    item.addNewConfiguration(ONF_AdjustPlotPositionCylinderDrawManager::staticInitConfigDrawBaseCircle() , QObject::tr("Dessiner le cercle de base"), CT_ItemDrawableConfiguration::Bool, false);
    item.addNewConfiguration(ONF_AdjustPlotPositionCylinderDrawManager::staticInitConfigSetFixedHeight() , QObject::tr("Hauteur fixe"), CT_ItemDrawableConfiguration::Bool, false);
    item.addNewConfiguration(ONF_AdjustPlotPositionCylinderDrawManager::staticInitConfigHeight() , QObject::tr("Hauteur"), CT_ItemDrawableConfiguration::Double, 5.0);
    item.addNewConfiguration(ONF_AdjustPlotPositionCylinderDrawManager::staticInitConfigTransparencyLevel() , QObject::tr("Niveau de transparence [0;255]"), CT_ItemDrawableConfiguration::Double, 250);

    return item;
}

// PROTECTED //

QString ONF_AdjustPlotPositionCylinderDrawManager::staticInitConfigDrawCylinder()
{
    return "CYLONF2_DC";
}

QString ONF_AdjustPlotPositionCylinderDrawManager::staticInitConfigDrawAxe()
{
    return "CYLONF2_DA";
}

QString ONF_AdjustPlotPositionCylinderDrawManager::staticInitConfigDrawBaseCircle()
{
    return "CYLONF2_DMC";
}

QString ONF_AdjustPlotPositionCylinderDrawManager::staticInitConfigSetFixedHeight()
{
    return "CYLONF2_FH";
}

QString ONF_AdjustPlotPositionCylinderDrawManager::staticInitConfigHeight()
{
    return "CYLONF2_HH";
}

QString ONF_AdjustPlotPositionCylinderDrawManager::staticInitConfigTransparencyLevel()
{
    return "CYLONF2_TL";
}
