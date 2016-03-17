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

#include "onf_filterbyreturntype.h"
#include "ct_pointcloudindex/ct_pointcloudindexvector.h"
#include "ct_iterator/ct_pointiterator.h"
#include "ct_view/ct_genericconfigurablewidget.h"

#define checkAndSetValue(ATT, NAME, TYPE) if((value = settings->firstValueByTagName(NAME)) == NULL) { return false; } else { ATT = value->value().value<TYPE>(); }
#define checkAndSetValueSpecial(ATT, NAME, TYPE, STYPE) if((value = settings->firstValueByTagName(NAME)) == NULL) { return false; } else { ATT = (STYPE)value->value().value<TYPE>(); }
#define checkAndInsertValue(ATT, NAME, TYPE) if((value = group->firstValueByTagName(NAME)) == NULL) { return false; } else { ATT.insert(value->value().value<TYPE>()); }

ONF_FilterByReturnType::ONF_FilterByReturnType() : CT_AbstractFilter_LAS()
{
    _type = ONF_FilterByReturnType::All;
    _typeAsString = "All";

    _filterByClassif = false;
    _keepVegetation = true;
    _keepGround = false;
    _keepNotClassified = false;
    _keepBuldings = false;
    _keepWater = false;
    _keepValues = "";
}

ONF_FilterByReturnType::ONF_FilterByReturnType(const ONF_FilterByReturnType &other) : CT_AbstractFilter_LAS(other)
{
    _type = other._type;
    _typeAsString = getStringForType(_type);

    _classifToKeep = other._classifToKeep;
    _filterByClassif = other._filterByClassif;
    _keepVegetation = other._keepVegetation;
    _keepGround = other._keepGround;
    _keepNotClassified = other._keepNotClassified;
    _keepBuldings = other._keepBuldings;
    _keepWater = other._keepWater;
    _keepValues = other._keepValues;
}

QString ONF_FilterByReturnType::getDetailledDisplayableName()
{
    QString result = getStringForType(_type);
    if (_filterByClassif)
    {
        if (_keepVegetation) {result.append("Veg");}
        if (_keepGround) {result.append("Gnd");}
        if (_keepNotClassified) {result.append("Ncl");}
        if (_keepBuldings) {result.append("Bul");}
        if (_keepWater) {result.append("Wat");}
        if (!_keepValues.isEmpty())
        {
            result.append("Clas");
            result.append(_keepValues.replace(";", "."));
        }
    }

    return result;
}

CT_AbstractConfigurableWidget* ONF_FilterByReturnType::createConfigurationWidget()
{
    CT_GenericConfigurableWidget* configDialog = new CT_GenericConfigurableWidget();

    QStringList typesList;

    for (int type = ONF_FilterByReturnType::ReturnType_begin ; type <= ONF_FilterByReturnType::ReturnType_end ; type++)
        typesList.append(getStringForType((ONF_FilterByReturnType::ReturnType) type));

    _typeAsString = getStringForType(_type);

    configDialog->addStringChoice(tr("Type de retours à conserver"), "", typesList, _typeAsString, getDetailledDescription());
    configDialog->addEmpty();
    configDialog->addBool(tr("Filter selon la classification"), "", tr("Conserver les classifications suivantes :"), _filterByClassif);
    configDialog->addBool("", "", tr("Végétation (3,4,5)"), _keepVegetation);
    configDialog->addBool("", "", tr("Sol (2)"), _keepGround);
    configDialog->addBool("", "", tr("Non classés (0,1)"), _keepNotClassified);
    configDialog->addBool("", "", tr("Constructions (6)"), _keepBuldings);
    configDialog->addBool("", "", tr("Eau (9)"), _keepWater);
    configDialog->addString(tr("Autres valeurs à conserver (séparées par des ;)"), "", _keepValues);

    return configDialog;
}

void ONF_FilterByReturnType::postConfigure()
{
    _type = getTypeForString(_typeAsString);

    _classifToKeep.clear();

    if (_filterByClassif)
    {
        if (_keepNotClassified) {_classifToKeep.insert(0);_classifToKeep.insert(1);}
        if (_keepGround) {_classifToKeep.insert(2);}
        if (_keepVegetation) {_classifToKeep.insert(3); _classifToKeep.insert(4); _classifToKeep.insert(5);}
        if (_keepBuldings) {_classifToKeep.insert(6);}
        if (_keepWater) {_classifToKeep.insert(9);}

        QStringList vals = _keepValues.split(";", QString::SkipEmptyParts);
        for (int i =  0 ; i < vals.size() ; i++)
        {
            bool ok = false;
            int val = vals.at(i).toInt(&ok);

            if (ok && val >=0 && val <= 255)
                _classifToKeep.insert(val);
        }
    }
}

/*void ONF_FilterByReturnType::updateParamtersAfterConfiguration()
{
    _type = getTypeForString(_typeAsString);

    _classifToKeep.clear();
    if (_filterByClassif)
    {
        if (_keepNotClassified) {_classifToKeep.insert(0);_classifToKeep.insert(1);}
        if (_keepGround) {_classifToKeep.insert(2);}
        if (_keepVegetation) {_classifToKeep.insert(3); _classifToKeep.insert(4); _classifToKeep.insert(5);}
        if (_keepBuldings) {_classifToKeep.insert(6);}
        if (_keepWater) {_classifToKeep.insert(9);}

        QStringList vals = _keepValues.split(";", QString::SkipEmptyParts);
        for (int i =  0 ; i < vals.size() ; i++)
        {
            bool ok = false;
            int val = vals.at(i).toInt(&ok);

            if (ok && val >=0 && val <= 255)
            {
                _classifToKeep.insert(val);
            }
        }
    }
}

QString ONF_FilterByReturnType::getParametersAsString() const
{
    QString result = "";
    result.append(getStringForType(_type));

    if (_filterByClassif)
    {
        result.append(";");
        QSetIterator<quint8> it(_classifToKeep);
        while (it.hasNext())
        {
            result.append(QString("%1;").arg(it.next()));
        }
    }

    return result;
}

bool ONF_FilterByReturnType::setParametersFromString(QString parameters)
{
    _classifToKeep.clear();

    QStringList params = parameters.split(";", QString::SkipEmptyParts);
    if (params.size() > 0)
    {
        _type = getTypeForString(params.at(0));
    }

    if (params.size() > 1)
    {
        _filterByClassif = true;
        for (int i = 1 ; i < params.size() ; i++)
        {
            bool ok = false;
            int val = params.at(i).toInt(&ok);

            if (ok && val >=0 && val <= 255)
            {
                _classifToKeep.insert(val);
            }
        }
    }


    _keepNotClassified = _classifToKeep.contains(0) || _classifToKeep.contains(1);
    _keepGround = _classifToKeep.contains(2);
    _keepVegetation = _classifToKeep.contains(3) || _classifToKeep.contains(4) || _classifToKeep.contains(5);
    _keepBuldings = _classifToKeep.contains(6);
    _keepWater = _classifToKeep.contains(9);

    QSetIterator<quint8> it(_classifToKeep);
    while (it.hasNext())
    {
        quint8 val = it.next();
        if (val > 6 && val != 9)
        {

        }
        _keepValues.append(QString("%1;").arg(val));
    }

    return true;
}*/

SettingsNodeGroup *ONF_FilterByReturnType::getAllSettings() const
{
    SettingsNodeGroup *root = new SettingsNodeGroup("ONF_FilterByReturnType");
    root->addValue(new SettingsNodeValue("type", (int)_type));
    root->addValue(new SettingsNodeValue("filterByClassif", _filterByClassif));
    root->addValue(new SettingsNodeValue("keepVegetation", _keepVegetation));
    root->addValue(new SettingsNodeValue("keepGround", _keepGround));
    root->addValue(new SettingsNodeValue("keepNotClassified", _keepNotClassified));
    root->addValue(new SettingsNodeValue("keepBuldings", _keepBuldings));
    root->addValue(new SettingsNodeValue("keepWater", _keepWater));
    root->addValue(new SettingsNodeValue("keepValues", _keepValues));

    return root;
}

bool ONF_FilterByReturnType::setAllSettings(const SettingsNodeGroup *settings)
{
    if((settings == NULL) || (settings->name() != "ONF_FilterByReturnType"))
        return false;

    SettingsNodeValue *value = NULL;

    checkAndSetValueSpecial(_type, "type", int, ONF_FilterByReturnType::ReturnType);
    _typeAsString = getStringForType(_type);
    checkAndSetValue(_filterByClassif, "filterByClassif", bool);
    checkAndSetValue(_keepVegetation, "keepVegetation", bool);
    checkAndSetValue(_keepGround, "keepGround", bool);
    checkAndSetValue(_keepNotClassified, "keepNotClassified", bool);
    checkAndSetValue(_keepBuldings, "keepBuldings", bool);
    checkAndSetValue(_keepWater, "keepWater", bool);
    checkAndSetValue(_keepValues, "keepValues", QString);

    return true;
}

QString ONF_FilterByReturnType::getShortDescription() const
{
    return tr("Filter by return type and Classification");
}

QString ONF_FilterByReturnType::getDetailledDescription() const
{
    return tr("Filter by return type. You can specify following types:\n"
              "- First: first returns\n"
              "- Last: last returns\n"
              "- LastAndOnly: last and only returns\n"
              "- Intermediare: Returns which are not first neither last\n"
              "- Only: first returns if no other return for the ray\n"
              "- All : don't filter on return type\n"
              "\n"
              "You can also choose which classifications to keep.");
}

CT_AbstractConfigurableElement *ONF_FilterByReturnType::copy() const
{
    return new ONF_FilterByReturnType(*this);
}

bool ONF_FilterByReturnType::validatePoint(const CT_PointIterator &pointIt, const CT_LASData &LASData)
{   
    Q_UNUSED(pointIt)

    // Test for the type of return
    if (_type == ONF_FilterByReturnType::First && LASData._Return_Number != 1) {return false;}
    if (_type == ONF_FilterByReturnType::Last && (LASData._Return_Number != LASData._Number_of_Returns || LASData._Return_Number == 1)) {return false;}
    if (_type == ONF_FilterByReturnType::LastAndOnly && LASData._Return_Number != LASData._Number_of_Returns) {return false;}
    if (_type == ONF_FilterByReturnType::Intermediate && (LASData._Return_Number == 1 || LASData._Return_Number == LASData._Number_of_Returns)) {return false;}
    if (_type == ONF_FilterByReturnType::Only && (LASData._Return_Number != 1 || LASData._Number_of_Returns != 1)) {return false;}

    // test for classification
    if (_filterByClassif && !_classifToKeep.contains(LASData._Classification)) {return false;}

    return true;
}

QString ONF_FilterByReturnType::getStringForType(ONF_FilterByReturnType::ReturnType returnType) const
{
    switch (returnType)
    {
        case ONF_FilterByReturnType::First : return tr("first");
        case ONF_FilterByReturnType::Last : return tr("last");
        case ONF_FilterByReturnType::LastAndOnly : return tr("last and only");
        case ONF_FilterByReturnType::Intermediate : return tr("intermerdiate");
        case ONF_FilterByReturnType::Only : return tr("only");
    }

    return "all";
}

ONF_FilterByReturnType::ReturnType ONF_FilterByReturnType::getTypeForString(QString returnTypeAsString) const
{
    if (returnTypeAsString == "first") {return ONF_FilterByReturnType::First;}
    if (returnTypeAsString == "last") {return ONF_FilterByReturnType::Last;}
    if (returnTypeAsString == "lastAndOnly") {return ONF_FilterByReturnType::LastAndOnly;}
    if (returnTypeAsString == "int") {return ONF_FilterByReturnType::Intermediate;}
    if (returnTypeAsString == "only") {return ONF_FilterByReturnType::Only;}

    return ONF_FilterByReturnType::All;
}
