/*
 * Copyright (C) <2012> <EDF-R&D> <FRANCE>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/**
 * \file TYAcousticRectangleWidget.cpp
 * \brief outil IHM pour un rectangle acoustique
 */


//Added by qt3to4:
#include <QGridLayout>

#include "Tympan/models/business/OLocalizator.h"
#include "Tympan/models/business/geoacoustic/TYAcousticRectangle.h"
#include "Tympan/gui/widgets/TYAcousticSurfaceWidget.h"
#include "TYAcousticRectangleWidget.h"

#define TR(id) OLocalizator::getString("TYAcousticRectangleWidget", (id))


TYAcousticRectangleWidget::TYAcousticRectangleWidget(TYAcousticRectangle* pElement, QWidget* _pParent /*=NULL*/):
    TYWidget(pElement, _pParent)
{

    resize(300, 640);
    setWindowTitle(TR("id_caption"));
    _acousticRectangleLayout = new QGridLayout();
    setLayout(_acousticRectangleLayout);

    _elmW = new TYAcousticSurfaceWidget(pElement, this);
    _acousticRectangleLayout->addWidget(_elmW, 0, 0);
}

TYAcousticRectangleWidget::~TYAcousticRectangleWidget()
{
}

void TYAcousticRectangleWidget::updateContent()
{
    _elmW->updateContent();
}

void TYAcousticRectangleWidget::apply()
{
    _elmW->apply();

    emit modified();
}
