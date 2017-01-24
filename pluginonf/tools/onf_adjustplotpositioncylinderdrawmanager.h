#ifndef ONF_ADJUSTPLOTPOSITIONCYLINDERDRAWMANAGER_H
#define ONF_ADJUSTPLOTPOSITIONCYLINDERDRAWMANAGER_H

#include "ct_itemdrawable/tools/drawmanager/ct_standardabstractshapedrawmanager.h"

class ONF_AdjustPlotPositionCylinderDrawManager : public CT_StandardAbstractShapeDrawManager
{
public:
    ONF_AdjustPlotPositionCylinderDrawManager(QString drawConfigurationName = "");
    virtual ~ONF_AdjustPlotPositionCylinderDrawManager();

    virtual void draw(GraphicsViewInterface &view, PainterInterface &painter, const CT_AbstractItemDrawable &itemDrawable) const;

    void setTranslation(double x, double y);
    void setColor(QColor color);

private:
    double  _transX;
    double  _transY;
    QColor  _color;

protected:

    const static QString INDEX_CONFIG_DRAW_CYLINDER;
    const static QString INDEX_CONFIG_DRAW_AXE;
    const static QString INDEX_CONFIG_DRAW_BASE_CIRCLE;
    const static QString INDEX_CONFIG_DRAW_SET_FIXED_HEIGHT;
    const static QString INDEX_CONFIG_HEIGHT;
    const static QString INDEX_CONFIG_TRANSPARENCY_LEVEL;

    static QString staticInitConfigDrawCylinder();
    static QString staticInitConfigDrawAxe();
    static QString staticInitConfigDrawBaseCircle();
    static QString staticInitConfigSetFixedHeight();
    static QString staticInitConfigHeight();
    static QString staticInitConfigTransparencyLevel();

    virtual CT_ItemDrawableConfiguration createDrawConfiguration(QString drawConfigurationName) const;
};

#endif // ONF_ADJUSTPLOTPOSITIONCYLINDERDRAWMANAGER_H
