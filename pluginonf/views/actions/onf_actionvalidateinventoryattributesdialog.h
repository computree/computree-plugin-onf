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

#ifndef ONF_ACTIONVALIDATEINVENTORYATTRIBUTESDIALOG_H
#define ONF_ACTIONVALIDATEINVENTORYATTRIBUTESDIALOG_H

#include <QDialog>

namespace Ui {
class ONF_ActionValidateInventoryAttributesDialog;
}

class ONF_ActionValidateInventoryAttributesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ONF_ActionValidateInventoryAttributesDialog(const QStringList &speciesList, const QString &speciesValue, const QString &idValue, QWidget *parent = 0);
    ~ONF_ActionValidateInventoryAttributesDialog();

    QString getSpecies();
    QString getId();

private:
    Ui::ONF_ActionValidateInventoryAttributesDialog *ui;
};

#endif // ONF_ACTIONVALIDATEINVENTORYATTRIBUTESDIALOG_H
