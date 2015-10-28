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


#ifndef ONF_LAI2000DATA_H
#define ONF_LAI2000DATA_H

#include "ct_itemdrawable/abstract/ct_abstractitemdrawablewithoutpointcloud.h"
#include "math.h"


#define ANGLE1b (0*M_PI/180.0)
#define ANGLE1m (7*M_PI/180.0)
#define ANGLE1e (13*M_PI/180.0)
#define W1 (0.034)

#define ANGLE2b (16*M_PI/180.0)
#define ANGLE2m (23*M_PI/180.0)
#define ANGLE2e (28*M_PI/180.0)
#define W2 (0.104)

#define ANGLE3b (32*M_PI/180.0)
#define ANGLE3m (38*M_PI/180.0)
#define ANGLE3e (43*M_PI/180.0)
#define W3 (0.160)
#define ANGLE4b (47*M_PI/180.0)
#define ANGLE4m (53*M_PI/180.0)
#define ANGLE4e (58*M_PI/180.0)
#define W4 (0.208)

#define ANGLE5b (61*M_PI/180.0)
#define ANGLE5m (68*M_PI/180.0)
#define ANGLE5e (74*M_PI/180.0)
#define W5 (0.494)


/*!
 * \class ONF_Lai2000Data
 * \ingroup Items_OE
 * \brief <b>Store data produced by step ONF_StepComputeLAI2000Data.</b>
 *
 * \param _fieldNames Optional fields names
 * \param _t1 Transmitance in first LAI2000 ring (zenithal one)
 * \param _t2 Transmitance in second LAI2000 ring
 * \param _t3 Transmitance in third LAI2000 ring
 * \param _t4 Transmitance in fourth LAI2000 ring
 * \param _t5 Transmitance in fith LAI2000 ring
 * \param _LAI3A LAI computed on 3 first rings
 */
class ONF_Lai2000Data : public CT_AbstractItemDrawableWithoutPointCloud
{
    Q_OBJECT

public:
    ONF_Lai2000Data();

    /**
      * \brief Contructeur
      */
    ONF_Lai2000Data(const CT_OutAbstractSingularItemModel *model, const CT_AbstractResult *result, double t1, double t2, double t3, double t4, double t5);

    ONF_Lai2000Data(const QString &modelName, const CT_AbstractResult *result, double t1, double t2, double t3, double t4, double t5);

    /**
      * ATTENTION : ne pas oublier de redéfinir ces deux méthodes si vous héritez de cette classe.
      */
    virtual QString getType() const;
    static QString staticGetType();

    virtual int nDataAvalaible() const;
    virtual QVariant data(int column) const;

    virtual CT_AbstractItemDrawable* copy(const CT_OutAbstractItemModel *model, const CT_AbstractResult *result, CT_ResultCopyModeList copyModeList);

private:
    double _t1;
    double _t2;
    double _t3;
    double _t4;
    double _t5;
    double _LAI3A;

};

#endif // ONF_LAI2000DATA_H
