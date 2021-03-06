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
 * \file TYBatimentModelerFrame.cpp
 * \brief Modeler specialisee pour l'edition des b�timents
 */


#include <qlayout.h>
#include <qcursor.h>
#include <qpushbutton.h>
#include <qcursor.h>
#include <qmessagebox.h>

#include "Tympan/models/business/OLocalizator.h"
#include "Tympan/models/business/TYPreferenceManager.h"
#include "Tympan/gui/app/TYEtageEditor.h"
#include "Tympan/gui/app/TYSilosEditor.h"
#include "Tympan/gui/app/TYSourceEditor.h"
#include "Tympan/gui/app/TYPickEditor.h"
#include "Tympan/gui/app/TYAbstractSceneEditor.h"
#include "Tympan/gui/app/TYApplication.h"
#include "TYBatimentModelerFrame.h"


#define TR(id) OLocalizator::getString("TYBatimentModelerFrame", (id))
#define IMG(id) OLocalizator::getPicture("TYSiteModelerFrame", (id))


int TYBatimentModelerFrame::_nbInstance = 0;


TYBatimentModelerFrame::TYBatimentModelerFrame(LPTYBatiment pBatiment, QWidget* parent, const char* name, Qt::WFlags f) :
    TYModelerFrame(parent, name, f)
{
    _nbInstance++;
    setWindowTitle(TR("id_caption") + " " + QString("%1").arg(_nbInstance));

    _pCtrlLayout->addSpacing(10);

    // Btn Calculer
    QPushButton* pCalculBtn = new QPushButton(QPixmap(IMG("id_icon_calculalti_btn")),  "",/*TR("id_calcul_btn"),*/ this);
    pCalculBtn->setFixedSize(24, 24);
    _pCtrlLayout->addWidget(pCalculBtn, 0);
    _pCtrlLayout->addStretch(1);
    connect(pCalculBtn, SIGNAL(clicked()), this, SLOT(calculDistribution()));


    // Editors
    _pEtageEditor = new TYEtageEditor(this);
    _pSilosEditor = new TYSilosEditor(this);
    _pSourceEditor = new TYSourceEditor(this);

    // Cadrage
    _pOGLCameras[TopView]->setDefaultZoomFactor(0.3);
    _pOGLCameras[FrontView]->setDefaultZoomFactor(0.3);
    _pOGLCameras[LeftView]->setDefaultZoomFactor(0.3);
    _pOGLCameras[PerspView]->setDefaultZoomFactor(0.7);

    if (pBatiment)
    {
        setBatiment(pBatiment);
    }
    else
    {
        setBatiment(new TYBatiment());
    }

    // Vue de dessus
    setViewType(TopView);
    updatePreferences();
}

TYBatimentModelerFrame::~TYBatimentModelerFrame()
{
    _nbInstance--;

    if (_pBatiment)
    {
        _pBatiment->drawGraphic(false);
    }

    delete _pEtageEditor;
    delete _pSilosEditor;
    delete _pSourceEditor;
    getView()->updateGL();
}

bool TYBatimentModelerFrame::close()
{
    return TYModelerFrame::close();
}

void TYBatimentModelerFrame::setBatiment(LPTYBatiment pBatiment)
{
    QString caption(TR("id_caption") + " " + QString("%1").arg(_nbInstance));

    if (_pBatiment)
    {
        // On recherche si ce batiment fait parti d'un site
        if (_pBatiment->getParent())
        {
            TYSiteNode* pSite = TYSiteNode::safeDownCast(_pBatiment->getParent()->getParent());
            if (pSite)
            {
                // Dans ce cas on supprime le plan de masse du site
#ifdef _DEBUG
                //getRenderer()->RemoveActor(pSite->getGraphicObject()->getProp());//az-- : revoir (ne devait pas marcher en OpenGL)
#endif
            }
        }

        _pBatiment->drawGraphic(false);
    }

    _pBatiment = pBatiment;

    if (_pBatiment)
    {
        if (!_pBatiment->getName().isEmpty())
        {
            caption += QString(" : %1").arg(_pBatiment->getName());
        }

        // On affiche le batiment
        _pBatiment->drawGraphic();

        // On recherche si ce batiment fait parti d'un site
        if (_pBatiment->getParent())
        {
            TYSiteNode* pSite = TYSiteNode::safeDownCast(_pBatiment->getParent()->getParent());
            if (pSite)
            {
                // Dans ce cas on affiche le plan de masse du site
                //              getRenderer()->AddActor(pSite->getGraphicObject()->getActor()); // Mise en Commentaire DT le 08/07/03
            }
        }

        // On recadre
        fit();
    }

    _pElement = _pBatiment;
    setWindowTitle(caption);

    _pView->getRenderer()->setElement((LPTYElement&)_pBatiment);
    getView()->getRenderer()->updateDisplayList();
}

void TYBatimentModelerFrame::setEditorMode(int mode)
{
    if (!_editorModeAccepted)
    {
        getPickEditor()->usePopup(true);
        getPickEditor()->useHighlight(false);

        if (_pCurrentEditor)
        {
            _pCurrentEditor->disconnect();
            _pCurrentEditor->close();
        }

        _editorModeAccepted = true;

        switch (mode)
        {
            case EtageMode:
                _pCurrentEditor = _pEtageEditor;
                break;
            case SilosMode:
                _pCurrentEditor = _pSilosEditor;
                break;
                //      case MachineMode:
                //          _pDataBaseEditor->setElementTypeName("TYMachine");
                //          _pCurrentEditor = _pDataBaseEditor;
                //          break;
            case SourceMode:
                _pCurrentEditor = _pSourceEditor;
                break;
            default:
                _editorModeAccepted = false;
        }
    }

    TYModelerFrame::setEditorMode(mode);
}

void TYBatimentModelerFrame::closeEvent(QCloseEvent* pEvent)
{
	TYPreferenceManager::saveGeometryToPreferences(metaObject()->className(), this);
	// If there is no volume in the modeler
	if(_pBatiment->getNbChild()==0)
	{
		// Displaying a warning message
		QMessageBox::StandardButton msgBox = QMessageBox::warning(this,"","Le modeleur est vide. Etes-vous s�r de vouloir le fermer ?", QMessageBox::Yes|QMessageBox::No);
		switch(msgBox)
		{
			// The user aborts the closing event
			case QMessageBox::No:
				pEvent->ignore();
				break;
			// The user ignores the warning
			case QMessageBox::Yes:
				emit frameResized();
				emit aboutToClose();
				break;
			default:
				pEvent->ignore();
				return;
		}
	}
	// If there are volumes, the signal is emited and the event accepted
	else
	{
		pEvent->accept();
		emit frameResized();
		emit aboutToClose(); 
	}
}

void TYBatimentModelerFrame::calculDistribution()
{
    if (_pBatiment)
    {
        getTYApp()->getCalculManager()->updateAcoustic(_pBatiment);
    }
}

void TYBatimentModelerFrame::updatePreferences()
{
    // Grille
    _gridDimX = 100.0f;
    if (TYPreferenceManager::exists(TYDIRPREFERENCEMANAGER, "GridDimXBatiment"))
    {
        _gridDimX = TYPreferenceManager::getFloat(TYDIRPREFERENCEMANAGER, "GridDimXBatiment");
    }
    else
    {
        TYPreferenceManager::setFloat(TYDIRPREFERENCEMANAGER, "GridDimXBatiment", _gridDimX);
    }

    _gridDimY = 100.0f;
    if (TYPreferenceManager::exists(TYDIRPREFERENCEMANAGER, "GridDimYBatiment"))
    {
        _gridDimY = TYPreferenceManager::getFloat(TYDIRPREFERENCEMANAGER, "GridDimYBatiment");
    }
    else
    {
        TYPreferenceManager::setFloat(TYDIRPREFERENCEMANAGER, "GridDimYBatiment", _gridDimY);
    }

    _gridStep = 5.0f;
    if (TYPreferenceManager::exists(TYDIRPREFERENCEMANAGER, "GridStepBatiment"))
    {
        _gridStep = TYPreferenceManager::getFloat(TYDIRPREFERENCEMANAGER, "GridStepBatiment");
    }
    else
    {
        TYPreferenceManager::setFloat(TYDIRPREFERENCEMANAGER, "GridStepBatiment", _gridStep);
    }

    _gridMagnStep = 2.0f;
    if (TYPreferenceManager::exists(TYDIRPREFERENCEMANAGER, "GridMagnStepBatiment"))
    {
        _gridMagnStep = TYPreferenceManager::getFloat(TYDIRPREFERENCEMANAGER, "GridMagnStepBatiment");
    }
    else
    {
        TYPreferenceManager::setFloat(TYDIRPREFERENCEMANAGER, "GridMagnStepBatiment", _gridMagnStep);
    }

    resizeGrid();
    setGridLinesActorsVisibility((_curViewType == FrontView), (_curViewType == TopView) || (_curViewType == PerspView), (_curViewType == LeftView));

    TYModelerFrame::updatePreferences();
}
