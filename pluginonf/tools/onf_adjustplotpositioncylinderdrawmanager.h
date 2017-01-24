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
    void setParameters(bool circles, bool fixedH, double h);
    void setColor(QColor color);

private:
    double  _transX;
    double  _transY;
    QColor  _color;
    bool _circles;
    bool _fixedH;
    double _h;

protected:

    virtual CT_ItemDrawableConfiguration createDrawConfiguration(QString drawConfigurationName) const;
};

#endif // ONF_ADJUSTPLOTPOSITIONCYLINDERDRAWMANAGER_H
