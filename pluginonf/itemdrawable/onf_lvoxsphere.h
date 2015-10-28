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

#ifndef ONF_LVOXSPHERE_H
#define ONF_LVOXSPHERE_H

#include "ct_itemdrawable/abstract/ct_abstractitemdrawablewithoutpointcloud.h"

/*!
 * \class ONF_LvoxSphere
 * \ingroup Items_OE
 * \brief <b>Store data produced by step ONF_StepComputeSphereVoxels.</b>
 *
 * \param _name Name of the sphere
 * \param _xs X coordinate of the sphere center
 * \param _ys Y coordinate of the sphere center
 * \param _zs Z coordinate of the sphere center
 * \param _rs Radius of the sphere
 * \param _re Exclusion radius of the sphere
 * \param _ni Number of hits
 * \param _nb Number of before (rays intercepted before the sphere)
 * \param _ne Number of excluded points (contained in _re)
 * \param _nt Number of theorical rays trversing the sphere
 */
class ONF_LvoxSphere : public CT_AbstractItemDrawableWithoutPointCloud
{
    // IMPORTANT pour avoir le nom de l'ItemDrawable
    Q_OBJECT

public:

    ONF_LvoxSphere();

    /**
      * \brief Contructeur
      */
    ONF_LvoxSphere(const CT_OutAbstractSingularItemModel *model,
                  const CT_AbstractResult *result,
                  QString name,
                  float xs,
                  float ys,
                  float zs,
                  float rs,
                  float re);

    /**
      * \brief Contructeur
      */
    ONF_LvoxSphere(const QString &modelName,
                  const CT_AbstractResult *result,
                  QString name,
                  float xs,
                  float ys,
                  float zs,
                  float rs,
                  float re);

    /**
      * ATTENTION : ne pas oublier de redéfinir ces deux méthodes si vous héritez de cette classe.
      */
    virtual QString getType() const;
    static QString staticGetType();

    virtual CT_AbstractItemDrawable* copy(const CT_OutAbstractItemModel *model, const CT_AbstractResult *result, CT_ResultCopyModeList copyModeList);

    void setNi(int ni);
    void setNb(int nb);
    void setNt(float nt);
    void setNe(int ne);

    QString getName() const;
    float getXs() const;
    float getYs() const;
    float getZs() const;
    float getRs() const;
    float getRe() const;
    int getNi() const;
    int getNb() const;
    float getNt() const;
    int getNe() const;


private:
    CT_DEFAULT_IA_BEGIN(ONF_LvoxSphere)
    CT_DEFAULT_IA_V2(ONF_LvoxSphere, CT_AbstractCategory::staticInitDataDisplayableName(), &ONF_LvoxSphere::getName, QObject::tr("Name"))
    CT_DEFAULT_IA_V2(ONF_LvoxSphere, CT_AbstractCategory::staticInitDataX(), &ONF_LvoxSphere::getXs, QObject::tr("Xs"))
    CT_DEFAULT_IA_V2(ONF_LvoxSphere, CT_AbstractCategory::staticInitDataY(), &ONF_LvoxSphere::getYs, QObject::tr("Ys"))
    CT_DEFAULT_IA_V2(ONF_LvoxSphere, CT_AbstractCategory::staticInitDataZ(), &ONF_LvoxSphere::getZs, QObject::tr("Zs"))
    CT_DEFAULT_IA_V2(ONF_LvoxSphere, CT_AbstractCategory::staticInitDataRadius(), &ONF_LvoxSphere::getRs, QObject::tr("Radius"))
    CT_DEFAULT_IA_V2(ONF_LvoxSphere, CT_AbstractCategory::staticInitDataRadius(), &ONF_LvoxSphere::getRe, QObject::tr("ExclusionRadius"))
    CT_DEFAULT_IA_V2(ONF_LvoxSphere, CT_AbstractCategory::staticInitDataNumber(), &ONF_LvoxSphere::getNi, QObject::tr("Ni"))
    CT_DEFAULT_IA_V2(ONF_LvoxSphere, CT_AbstractCategory::staticInitDataNumber(), &ONF_LvoxSphere::getNb, QObject::tr("Nb"))
    CT_DEFAULT_IA_V2(ONF_LvoxSphere, CT_AbstractCategory::staticInitDataNumber(), &ONF_LvoxSphere::getNt, QObject::tr("Nt"))
    CT_DEFAULT_IA_V2(ONF_LvoxSphere, CT_AbstractCategory::staticInitDataNumber(), &ONF_LvoxSphere::getNe, QObject::tr("N_excluded"))
    CT_DEFAULT_IA_END(ONF_LvoxSphere)

    QString          _name;
    float            _xs;
    float            _ys;
    float            _zs;
    float            _rs;
    float            _re;
    int              _ni;
    int              _nb;
    int              _ne;
    float            _nt;

};

#endif // ONF_LVOXSPHERE_H
