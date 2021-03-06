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

#include <cstdlib>
#include <cassert>
#include<locale.h>

#if TY_USE_IHM
#include "Tympan/gui/widgets/TYSiteNodeWidget.h"
#include "Tympan/gui/gl/TYSiteNodeGraphic.h"
#endif

#include "Tympan/models/business/altimetry_file_reader_impl.h"
#include "Tympan/models/business/TYPreferenceManager.h"
#include "Tympan/models/business/TYXMLManager.h"
#include "Tympan/models/business/TYProgressManager.h"
#include "Tympan/models/business/OLocalizator.h"
#include "Tympan/models/business/infrastructure/TYEcran.h"
#include "Tympan/models/business/xml_project_util.h"
#include "Tympan/models/business/subprocess_util.h"
#include "Tympan/core/logging.h"
#include "Tympan/core/config.h"
#include <QDir>


#include "TYSiteNode.h"

#define TR(id) OLocalizator::getString("OMessageManager", (id))

TY_EXTENSION_INST(TYSiteNode);
TY_EXT_GRAPHIC_INST(TYSiteNode);

bool almost_equal(double a, double b, double precision);

/*static*/ const std::string& TYSiteNode::getTopoFilePath()
{
    if (_topoFilePath)
    {
        return *_topoFilePath;
    }
    else
    {
        _topoFilePath = new std::string(".");
        return *_topoFilePath;
    }
}

/*static*/ void TYSiteNode::setTopoFilePath(const std::string& path)
{
    if (_topoFilePath)
    {
        *_topoFilePath = path;
    }
    else
    {
        _topoFilePath = new std::string(path);
    }
}

std::string* TYSiteNode::_topoFilePath = NULL;

TYSiteNode::TYSiteNode() :  _pProjet(NULL),
    _bEmpriseAsCrbNiv(false),
    _isTopoFileModified(false),
    _echelle(1.0f),
    _useTopoFile(0),
    _topoFileName(""),
    _topoFileExtension(""),
    _nbFaceInfra(0),
    _altiEmprise(0.0),
    _root(false),
    _SIGType(TYMPAN),
    _SIG_X(0.0),
    _SIG_Y(0.0),
    _SIG_OFFSET(0.0)
{
    _name = TYNameManager::get()->generateName(getClassName());

#ifdef _WIN32
    // CLM-NT35: Pb en debug avec string cast
    std::string id = getStringID().toAscii();
    _topoFile = getTopoFilePath() + "/image_" + id.data();
#else
    _topoFile = getTopoFilePath() + "/image_" + getStringID().toAscii().data();
#endif

    _pTopographie = new TYTopographie();
    _pTopographie->setParent(this);
    _pInfrastructure = new TYInfrastructure();
    _pInfrastructure->setParent(this);
}

TYSiteNode::TYSiteNode(const TYSiteNode& other)
{
    *this = other;
}

TYSiteNode::~TYSiteNode()
{
    _listSiteNode.clear();
}

TYSiteNode& TYSiteNode::operator=(const TYSiteNode& other)
{
    if (this != &other)
    {
        TYElement::operator =(other);
        _echelle = other._echelle;
        _bEmpriseAsCrbNiv = other._bEmpriseAsCrbNiv;
        _altiEmprise = other._altiEmprise;
        _pTopographie = other._pTopographie;
        _topoFile = other._topoFile;
        _topoFileName = other._topoFileName;
        _topoFileExtension = other._topoFileExtension;
        _useTopoFile = other._useTopoFile;
        _isTopoFileModified = other._isTopoFileModified;
        _pInfrastructure = other._pInfrastructure;
        _orientation = other._orientation;
        _position = other._position;
        _root = other._root;
        _listSiteNode = other._listSiteNode;
        _SIGType = other._SIGType;
        _SIG_X = other._SIG_X;
        _SIG_Y = other._SIG_Y;
        _SIG_OFFSET = other._SIG_OFFSET;
    }
    return *this;
}

bool TYSiteNode::operator==(const TYSiteNode& other) const
{
    if (this != &other)
    {
        if (TYElement::operator !=(other)) { return false; }
        if (_echelle != other._echelle) { return false; }
        if (_bEmpriseAsCrbNiv != other._bEmpriseAsCrbNiv) { return false; }
        if (_altiEmprise != other._altiEmprise) { return false; }
        if (_pTopographie != other._pTopographie) { return false; }
        if (_topoFile != other._topoFile) { return false; }
        if (_topoFileName != other._topoFileName) { return false; }
        if (_topoFileExtension != other._topoFileExtension) { return false; }
        if (_useTopoFile != other._useTopoFile) { return false; }
        if (_isTopoFileModified != other._isTopoFileModified) { return false; }
        if (_pInfrastructure != other._pInfrastructure) { return false; }
        if (_orientation != other._orientation) { return false; }
        if (_position != other._position) { return false; }
        if (_root != other._root) { return false; }
        if (!(_listSiteNode == other._listSiteNode)) { return false; }
        if (_SIGType != other._SIGType) { return false; }
        if (_SIG_X != other._SIG_X) { return false; }
        if (_SIG_Y != other._SIG_Y) { return false; }
        if (_SIG_OFFSET != other._SIG_OFFSET) { return false; }
    }
    return true;
}

bool TYSiteNode::operator!=(const TYSiteNode& other) const
{
    return !operator==(other);
}

bool TYSiteNode::deepCopy(const TYElement* pOther, bool copyId /*=true*/)
{
    if (!TYElement::deepCopy(pOther, copyId)) { return false; }

    TYSiteNode* pOtherSite = (TYSiteNode*) pOther;

    _echelle = pOtherSite->_echelle;
    _bEmpriseAsCrbNiv = pOtherSite->_bEmpriseAsCrbNiv;
    _altiEmprise = pOtherSite->_altiEmprise;
    _topoFile = pOtherSite->_topoFile;
    _topoFileName = pOtherSite->_topoFileName;
    _topoFileExtension = pOtherSite->_topoFileExtension;
    _useTopoFile = pOtherSite->_useTopoFile;
    _isTopoFileModified = pOtherSite->_isTopoFileModified;
    _pTopographie->deepCopy(pOtherSite->_pTopographie, copyId);
    _pTopographie->setParent(this);
    _pInfrastructure->deepCopy(pOtherSite->_pInfrastructure, copyId);
    _pInfrastructure->setParent(this);
    _orientation.deepCopy(&pOtherSite->_orientation, copyId);
    _position.deepCopy(&pOtherSite->_position, copyId);
    _root = pOtherSite->_root;
    _SIGType = pOtherSite->_SIGType;
    _SIG_X = pOtherSite->_SIG_X;
    _SIG_Y = pOtherSite->_SIG_Y;
    _SIG_OFFSET = pOtherSite->_SIG_OFFSET;

    _listSiteNode.clear();
    for (unsigned int i = 0; i < pOtherSite->_listSiteNode.size(); i++)
    {
        LPTYSiteNodeGeoNode pSiteNodeGeoNode = new TYSiteNodeGeoNode(NULL, this);
        pSiteNodeGeoNode->deepCopy(pOtherSite->_listSiteNode[i], copyId);
        pSiteNodeGeoNode->getElement()->setParent(this);
        pSiteNodeGeoNode->setParent(this);
        _listSiteNode.push_back(pSiteNodeGeoNode);
    }

    return true;
}

std::string TYSiteNode::toString() const
{
    return "TYSiteNode";
}

DOM_Element TYSiteNode::toXML(DOM_Element& domElement)
{
    DOM_Element domNewElem = TYElement::toXML(domElement);

    TYXMLTools::addElementFloatValue(domNewElem, "echelle", _echelle);
    TYXMLTools::addElementBoolValue(domNewElem, "useEmpriseAsCrbNiv", _bEmpriseAsCrbNiv);
    TYXMLTools::addElementBoolValue(domNewElem, "useTopoFile", _useTopoFile);

    if (TYXMLManager::getSavedFileName() == QString(""))
    {
        TYXMLTools::addElementStringValue(domNewElem, "topoFile", _topoFileName.c_str());
    }
    else //si non, on ecrit le chemin relatif
    {
        QString xmlFile = TYXMLManager::getSavedFileName().replace('\\', '/');
        QDir xmlFileDir = QDir(xmlFile.left(xmlFile.lastIndexOf('/')));
        if (xmlFileDir.exists())
        {
            TYXMLTools::addElementStringValue(domNewElem, "topoFile", xmlFileDir.relativeFilePath(QString(_topoFileName.c_str())).toAscii().constData());
        }
        else
        {
            TYXMLTools::addElementStringValue(domNewElem, "topoFile", _topoFileName.c_str());
        }
    }

    TYXMLTools::addElementDoubleValue(domNewElem, "altiEmprise", _altiEmprise);

    _orientation.toXML(domNewElem);
    _position.toXML(domNewElem);

    _pTopographie->toXML(domNewElem);
    _pInfrastructure->toXML(domNewElem);

    TYXMLTools::addElementIntValue(domNewElem, "root", _root);
    TYXMLTools::addElementIntValue(domNewElem, "repere", _SIGType);
    TYXMLTools::addElementDoubleValue(domNewElem, "SIG_X", _SIG_X);
    TYXMLTools::addElementDoubleValue(domNewElem, "SIG_Y", _SIG_Y);
    TYXMLTools::addElementDoubleValue(domNewElem, "SIG_OFFSET", _SIG_OFFSET);

    for (unsigned int i = 0; i < _listSiteNode.size(); i++)
    {
        _listSiteNode[i]->toXML(domNewElem);
    }

    return domNewElem;
}

int TYSiteNode::fromXML(DOM_Element domElement)
{
    TYElement::fromXML(domElement);

    bool echelleOk = false;
    QString topoFile;
    bool topoFileOk = false;
    bool empriseAsCrbNivOk = false;
    bool altiEmpriseOk = false;
    bool useTopoFileOk = false;
    DOM_Element elemCur;

    QDomNodeList childs = domElement.childNodes();
    unsigned int childcount = childs.length();
    for (unsigned int i = 0; i < childcount; i++)
    {
        elemCur = childs.item(i).toElement();
        OMessageManager::get()->info("Charge element de site %d/%d.", i + 1, childcount);

        TYXMLTools::getElementFloatValue(elemCur, "echelle", _echelle, echelleOk);
        TYXMLTools::getElementBoolValue(elemCur, "useEmpriseAsCrbNiv", _bEmpriseAsCrbNiv, empriseAsCrbNivOk);
        TYXMLTools::getElementBoolValue(elemCur, "useTopoFile", _useTopoFile, useTopoFileOk);
        TYXMLTools::getElementStringValue(elemCur, "topoFile", topoFile, topoFileOk);
        TYXMLTools::getElementDoubleValue(elemCur, "altiEmprise", _altiEmprise, altiEmpriseOk);

        _orientation.callFromXMLIfEqual(elemCur);
        _position.callFromXMLIfEqual(elemCur);

        _pTopographie->callFromXMLIfEqual(elemCur);
        _pInfrastructure->callFromXMLIfEqual(elemCur);
    }


    purge(); // On vide le tableau des sous-sites

    bool rootOk = false;
    bool repereOk = false;
    bool SIG_XOk = false;
    bool SIG_YOk = false;
    bool SIG_OFFSETOk = false;
    int SIGType = 0;

    LPTYSiteNodeGeoNode pSiteNodeGeoNode = new TYSiteNodeGeoNode(NULL, this);
    //  DOM_Element elemCur;

    //  QDomNodeList childs = domElement.childNodes();
    for (unsigned int i = 0; i < childs.length(); i++)
    {
        elemCur = childs.item(i).toElement();
        TYXMLTools::getElementBoolValue(elemCur, "root", _root, rootOk);
        TYXMLTools::getElementIntValue(elemCur, "repere", SIGType, repereOk);
        TYXMLTools::getElementDoubleValue(elemCur, "SIG_X", _SIG_X, SIG_XOk);
        TYXMLTools::getElementDoubleValue(elemCur, "SIG_Y", _SIG_Y, SIG_YOk);
        TYXMLTools::getElementDoubleValue(elemCur, "SIG_OFFSET", _SIG_OFFSET, SIG_OFFSETOk);

        if (pSiteNodeGeoNode->callFromXMLIfEqual(elemCur))
        {
            addSiteNode(pSiteNodeGeoNode);
            pSiteNodeGeoNode = new TYSiteNodeGeoNode(NULL, this);
        }
    }

    _SIGType = (systemSIG)SIGType;

    if (_useTopoFile && topoFileOk)
    {
        _topoFileName = topoFile.toStdString();
        setTopoFileName(_topoFileName);
        loadTopoFile(_topoFileName);
    }

    return 1;
}

void TYSiteNode::getChilds(LPTYElementArray& childs, bool recursif /*=true*/)
{
    TYElement::getChilds(childs, recursif);

    childs.push_back(_pTopographie);
    childs.push_back(_pInfrastructure);

    if (recursif)
    {
        _pTopographie->getChilds(childs, recursif);
        _pInfrastructure->getChilds(childs, recursif);
    }

    for (unsigned int i = 0; i < _listSiteNode.size(); i++)
    {
        childs.push_back(_listSiteNode[i]);
        childs.push_back(_listSiteNode[i]->getElement());
    }

    if (recursif)
    {
        for (unsigned int i = 0; i < _listSiteNode.size(); i++)
        {
            _listSiteNode[i]->getChilds(childs, recursif);
        }
    }
}

void TYSiteNode::setIsGeometryModified(bool isModified)
{
    TYElement::setIsGeometryModified(isModified);

    if (!_root && _pParent) { _pParent->setIsGeometryModified(isModified); }
}

bool TYSiteNode::addToCalcul()
{
    bool res = _pInfrastructure->addToCalcul(); // Ajoute les elements du site lui-meme dans le calcul
    res = res && (_pInfrastructure->addToCalcul());

    if (res && _listSiteNode.size())
    {
        for (unsigned int i = 0; i < _listSiteNode.size(); i++)
        {
            res = res && (TYSiteNode::safeDownCast(_listSiteNode[i]->getElement()))->addToCalcul();
        }
    }

    return res;
}

bool TYSiteNode::remFromCalcul()
{
    bool res = _pInfrastructure->remFromCalcul();
    res = res && _pInfrastructure->remFromCalcul();;

    if (res && _listSiteNode.size())
    {
        for (unsigned int i = 0; i < _listSiteNode.size(); i++)
        {
            res = res && (TYSiteNode::safeDownCast(_listSiteNode[i]->getElement()))->remFromCalcul();
        }
    }

    return res;
}

void TYSiteNode::updateCurrentCalcul(TYListID& listID, bool recursif)//=true
{
    if (recursif) // On parcours les enfants si besoin est...
    {
        // Collecte des childs
        LPTYElementArray childs;
        getChilds(childs, false);
        for (int i = 0; i < childs.size(); i++)
        {
            childs[i]->updateCurrentCalcul(listID, recursif);
        }
    }

    TYElement::updateCurrentCalcul(listID, false);
}

void TYSiteNode::setProjet(TYProjet* pProjet)
{
    _pProjet = pProjet;
    for (unsigned int i = 0; i < _listSiteNode.size(); i++)
    {
        TYSiteNode::safeDownCast(_listSiteNode[i]->getElement())->setProjet(pProjet);
    }
}

void TYSiteNode::reparent()
{
    _pTopographie->reparent();
    _pInfrastructure->reparent();

    for (unsigned int i = 0; i < _listSiteNode.size(); i++)
    {
        TYSiteNode::safeDownCast(_listSiteNode[i]->getElement())->reparent();
    }
}

void TYSiteNode::loadTopoFile(std::string fileName)
{
    _topoFileName = fileName;
    loadTopoFile();
}

void TYSiteNode::loadTopoFile()
{
    FILE* streamSrc = NULL;
    FILE* streamDest = NULL;
    unsigned char c;

    // Ouverture du fichier source
    if ((streamSrc = fopen(_topoFileName.c_str(), "rb")) == NULL)
    {
        return;
    }

    // Creation d'un copie
    if ((streamDest = fopen(_topoFile.c_str(), "w+b")) == NULL)
    {
        return;
    }

    // Lecture et copie
    while (!feof(streamSrc))
    {
        fread(&c, sizeof(unsigned char), 1, streamSrc);
        fwrite(&c, sizeof(unsigned char), 1, streamDest);
    }

    // Fermeture des fichiers
    fclose(streamSrc);
    fclose(streamDest);

    // Mise a jour du flag.
    _isTopoFileModified = true;

 // On conserve l'extension de l'image = son type
    size_t pointAt = _topoFileName.find_last_of(".");

    if (pointAt == -1)
    {
        _topoFileExtension = "";
    }
    else
    {
        _topoFileExtension = _topoFileName.substr(pointAt);
    }

    setIsGeometryModified(true);
}

/*virtual*/ bool TYSiteNode::updateAltimetrie()
{
    ostringstream msg;
    OMessageManager& logger =  *OMessageManager::get();
    try
    {
        do_updateAltimetrie();
        return true;
    }
    catch (const tympan::exception& exc)
    {
        msg << boost::diagnostic_information(exc);
        logger.error("An error prevented to update the altimetry (set log level to debug for diagnostic)");
        logger.debug(msg.str().c_str());
        return false;
    }
}

/*virtual*/ void TYSiteNode::do_updateAltimetrie()
{
    OMessageManager& logger = *OMessageManager::get();

    // disables element names automatic generation (subsites fusion etc)
    TYNameManager::get()->enable(false);

    logger.info("Mise a jour altimetrie...");

    // Is the debug option "TYMPAN_DEBUG=keep_tmp_files" enabled?
    bool keep_tmp_files = must_keep_tmp_files();
    // Will be used to export the current site topography/infrastructure
    QTemporaryFile current_project;
    current_project.setFileTemplate(QDir::tempPath() + QString("/XXXXXX.xml"));
    // Here will go the mesh result in a PLY Polygon formatted file
    //(see http://www.cs.virginia.edu/~gfx/Courses/2001/Advanced.spring.01/plylib/Ply.txt)
    QTemporaryFile result_mesh;
    result_mesh.setFileTemplate(QDir::tempPath() + QString("/XXXXXX.ply"));
    if (!init_tmp_file(current_project, keep_tmp_files)
            || !init_tmp_file(result_mesh, keep_tmp_files))
    {
        logger.error("Creation de fichier temporaire impossible. Veuillez verifier l'espace disque disponible.");
        throw tympan::exception() << tympan_source_loc;
    }
    try
    {
        tympan::save_project(current_project.fileName().toUtf8().data(), _pProjet);
    }
    catch(const tympan::invalid_data& exc)
    {
        std::ostringstream msg;
        msg << boost::diagnostic_information(exc);
        logger.error(
                "Impossible d'exporter le projet courant pour calculer l'altimetrie.");
        TYNameManager::get()->enable(true);
        throw;
    }
    if(keep_tmp_files)
    {
        logger.debug(
                "Le calcul va s'executer en mode debug.\nLes fichiers temporaires ne seront pas supprimes une fois le calcul termine.\nProjet courant non calcule: %s  Projet avec les resultats du calcul: %s.",
                current_project.fileName().toStdString().c_str(),
                result_mesh.fileName().toStdString().c_str());
    }

    // Call python script "process_site_altimetry.py" with: the name of the file
    // containing the site description, and the name of the file where to record
    // the result
    QStringList args;
    QString absolute_pyscript_path (QCoreApplication::applicationDirPath());
    absolute_pyscript_path.append("/");
    absolute_pyscript_path.append(ALTIMETRY_PYSCRIPT);
    args << absolute_pyscript_path << current_project.fileName()
        << result_mesh.fileName();
    logger.info(
            "Lancement d'un sous-processus python pour calculer l'altimetrie avec le script: %s",
            absolute_pyscript_path.toStdString().c_str());
    string error_msg;
    if (!python(args, error_msg))
    {
        logger.error("Echec du calcul de l'altimetrie: %s", error_msg.c_str());
        TYNameManager::get()->enable(true);
        throw tympan::exception() << tympan_source_loc;
    }
    std::deque<OPoint3D> points;
    std::deque<OTriangle> triangles;
    std::deque<LPTYSol> materials;
    readMesh(points, triangles, materials, result_mesh.fileName());
    getAltimetry()->plugBackTriangulation(points, triangles, materials);
    setIsGeometryModified(false);  // L'altimetrie est a jour
    OMessageManager::get()->info("Mise a jour altimetrie terminee.");
    TYNameManager::get()->enable(true);
}

void TYSiteNode::readMesh(std::deque<OPoint3D>& points, std::deque<OTriangle>& triangles,
        std::deque<LPTYSol>& materials, const QString& filename)
{
    // CAUTION: reader uses rply C library which calls strtod (stdlib) to read float
    // and double values. strtod is locale dependent. It means that if decimal
    // separator is set to ',' instead of '.' in LC_NUMERIC, float values from
    // the ply file won't be read. To make sure this doesn't happen, temporarily
    // set the locale and then put back the original value after file reading.
    char *saved_locale = setlocale(LC_NUMERIC, "C");
    tympan::AltimetryPLYReader reader(filename.toStdString());
    reader.read();
    setlocale(LC_NUMERIC, saved_locale);
    points = reader.points();
    triangles = reader.faces();
    std::deque<std::string> material_ids = reader.materials();
    uuid2tysol(material_ids, materials);
}

void TYSiteNode::uuid2tysol(const std::deque<std::string>& material_ids, std::deque<LPTYSol>& materials)
{
    OMessageManager& logger = *OMessageManager::get();
    TYSol *ground;
    for (int i = 0; i < material_ids.size(); i++)
    {
        ground = dynamic_cast<TYSol *>(TYElement::getInstance(OGenID(QString(material_ids[i].c_str()))));
        if (ground != NULL)
        {
            materials.push_back(ground);
        }
        else
        {
            logger.debug(
                    "Unknown material retrieved from altimetry mesh: id = %s. Using default material instead",
                    material_ids[i].c_str());
            materials.push_back(_pTopographie->getDefTerrain()->getSol());
        }
    }
}

// TODO : Split the huge method based on the type of infrastructure
// See https://extranet.logilab.fr/ticket/1508248
void TYSiteNode::updateAltiInfra()
{
    TYNameManager::get()->enable(false);

    TYAltimetrie* pAlti = getAltimetry();
    bool modified = false;
    OPoint3D pt;
    register unsigned int i, j;

#if TY_USE_IHM
    size_t totalSteps = _pInfrastructure->getListRoute().size() +
                        _pInfrastructure->getListResTrans().size() +
                        _pInfrastructure->getListBatiment().size() +
                        _pInfrastructure->getListMachine().size() +
                        _pInfrastructure->getSrcs().size() +
                        _pTopographie->getListCrsEau().size() +
                        _pTopographie->getListTerrain().size();
    TYProgressManager::setMessage("Mise a jour de l'altimetrie des infrastructures");
    TYProgressManager::set(static_cast<int>(totalSteps));
#endif // TY_USE_IHM

    bool cancel = false;
    bool bNoPbAlti = true;

    // If the site node isn't a root site, compute its global transform matrix
    // to look for the altitudes at the right place
    OMatrix globalMatrix = getGlobalMatrix();

    // Mise a jour de l'altitude pour les points des routes
    for (j = 0; j < _pInfrastructure->getListRoute().size() && !cancel; j++)
    {
#if TY_USE_IHM
        TYProgressManager::step(cancel);
        if (cancel) { break; }
#endif // TY_USE_IHM

        // La route
        LPTYRouteGeoNode pGeoNode = _pInfrastructure->getListRoute()[j];
        TYRoute* pRoute = _pInfrastructure->getRoute(j);

        bNoPbAlti &= pRoute->updateAltitudes(*pAlti, pGeoNode, globalMatrix);
        modified = true; // As long as there is a road, it will be updated anyways.
    }

    // Mise a jour de l'altitude pour les points des reseaux transport
    for (j = 0; j < _pInfrastructure->getListResTrans().size() && !cancel; j++)
    {
        TYProgressManager::step(cancel);
        if (cancel) { break; }

        TYReseauTransport* pResTrans = _pInfrastructure->getResTrans(j);

        // Hauteur au sol du reseau de transport
        double hauteur = pResTrans->getHauteurMoyenne();

        // Matrice pour la position de cette element
        OMatrix matrix = globalMatrix * _pInfrastructure->getListResTrans()[j]->getMatrix();
        OMatrix matrixinv = matrix.getInvert();

        for (i = 0; i < pResTrans->getTabPoint().size(); i++)
        {
            // Passage au repere du site
            pt = matrix * pResTrans->getTabPoint()[i];

            // Init
            pt._z = 0.0;

            // Recherche de l'altitude
            bNoPbAlti &= pAlti->updateAltitude(pt);

            // Ajout de la hauteur du reseau de transport
            pt._z += hauteur;

            // Retour au repere d'origine
            pResTrans->getTabPoint()[i] = matrixinv * pt;

            modified = true;
        }

        pResTrans->setIsGeometryModified(false);
    }

    // Mise a jour de l'altitude pour les batiments
    for (j = 0; j < _pInfrastructure->getListBatiment().size() && !cancel; j++)
    {
#if TY_USE_IHM
        TYProgressManager::step(cancel);
        if (cancel) { break; }
#endif // TY_USE_IHM

        TYBatimentGeoNode* pBatGeoNode = _pInfrastructure->getListBatiment()[j];
        TYBatiment* pBat = TYBatiment::safeDownCast(pBatGeoNode->getElement());

        // Recuperation de l'origine de l'element
        pt = globalMatrix * pBatGeoNode->getORepere3D()._origin;

        // Hauteur par rapport au sol
        double hauteur = pBatGeoNode->getHauteur();

        // Init
        pt._z = 0.0;

        // Recherche de l'altitude
        bNoPbAlti &= pAlti->updateAltitude(pt);

        pBatGeoNode->getORepere3D()._origin._z = pt._z + hauteur;

        pBat->setIsGeometryModified(false);
        pBat = NULL;
        modified = true;
    }

    // Mise a jour de l'altitude pour les machines
    for (j = 0; j < _pInfrastructure->getListMachine().size() && !cancel; j++)
    {
#if TY_USE_IHM
        TYProgressManager::step(cancel);
        if (cancel) { break; }
#endif // TY_USE_IHM

        TYMachineGeoNode* pMachineGeoNode = _pInfrastructure->getListMachine()[j];
        TYMachine* pMachine = TYMachine::safeDownCast(pMachineGeoNode->getElement());

        // Recuperation de l'origine de l'element
        pt = globalMatrix * pMachineGeoNode->getORepere3D()._origin;

        // Hauteur par rapport au sol
        double hauteur = pMachineGeoNode->getHauteur();

        // Init
        pt._z = 0.0;

        // Recherche de l'altitude
        bNoPbAlti &= pAlti->updateAltitude(pt);

        pMachineGeoNode->getORepere3D()._origin._z = pt._z + hauteur;

        pMachine->setIsGeometryModified(false);
        pMachine = NULL;
        modified = true;
    }

    // Mise a jour de l'altitude pour les sources utilisateur
    for (j = 0; j < _pInfrastructure->getSrcs().size() && !cancel; j++)
    {
#if TY_USE_IHM
        TYProgressManager::step(cancel);
        if (cancel) { break; }
#endif // TY_USE_IHM

        // La source
        LPTYUserSourcePonctuelle pSrc = TYUserSourcePonctuelle::safeDownCast(_pInfrastructure->getSrc(j)->getElement());

        // Matrice pour la position de cette element
        OMatrix matrix = globalMatrix * _pInfrastructure->getSrcs()[j]->getMatrix();
        OMatrix matrixinv = matrix.getInvert();

        // Passage au repere du site
        pt = matrix * *pSrc->getPos();

        // Init
        pt._z = 0.0;

        // Recherche de l'altitude
        bNoPbAlti &= pAlti->updateAltitude(pt);

        // Ajout de la hauteur
        pt._z += pSrc->getHauteur();

        // Retour au repere d'origine
        pt = matrixinv * pt;
        pSrc->getPos()->_z = pt._z;

        // On va modifier la route (l'altitude seulement)
        pSrc->setIsGeometryModified(false);

        modified = true;
    }

    // Mise a jour de l'altitude pour les points des cours d'eau
    for (j = 0; j < _pTopographie->getListCrsEau().size() && !cancel; j++)
    {
#if TY_USE_IHM
        TYProgressManager::step(cancel);
        if (cancel) { break; }
#endif // TY_USE_IHM

        TYCoursEau* pCrsEau = _pTopographie->getCrsEau(j);

        // Matrice pour la position de cette element
        OMatrix matrix = globalMatrix * _pTopographie->getListCrsEau()[j]->getMatrix();
        OMatrix matrixinv = matrix.getInvert();

        for (i = 0; i < pCrsEau->getTabPoint().size(); i++)
        {
            // Passage au repere du site
            pt = matrix * pCrsEau->getTabPoint()[i];

            // Init
            pt._z = 0.0;

            // Recherche de l'altitude
            bNoPbAlti &= pAlti->updateAltitude(pt);

            // Retour au repere d'origine
            pCrsEau->getTabPoint()[i] = matrixinv * pt;

            modified = true;
        }

        pCrsEau->setIsGeometryModified(false);
    }

    // Mise a jour de l'altitude pour les points des terrains
    for (j = 0; j < _pTopographie->getListTerrain().size() && !cancel; j++)
    {
#if TY_USE_IHM
        TYProgressManager::step(cancel);
        if (cancel) { break; }
#endif // TY_USE_IHM

        TYTerrain* pTerrain = _pTopographie->getTerrain(j);

        // Matrice pour la position de cette element
        OMatrix matrix = globalMatrix * _pTopographie->getListTerrain()[j]->getMatrix();
        OMatrix matrixinv = matrix.getInvert();

        for (i = 0; i < pTerrain->getListPoints().size(); i++)
        {
            // Passage au repere du site
            pt = matrix * pTerrain->getListPoints()[i];

            // Init
            pt._z = 0.0;

            // Recherche de l'altitude
            bNoPbAlti &= pAlti->updateAltitude(pt);

            // Retour au repere d'origine
            pTerrain->getListPoints()[i] = matrixinv * pt;

            modified = true;
        }

        pTerrain->setIsGeometryModified(false);
    }

#if TY_USE_IHM
    TYProgressManager::stepToEnd();
#endif // TY_USE_IHM

    // Warning if an object is not correctly altimetrized
    if (!bNoPbAlti) { OMessageManager::get()->info(TR("msg_pbalti")); }

    if (modified)
    {
        _pInfrastructure->setIsGeometryModified(false);
        _pTopographie->setIsGeometryModified(false);
        setIsGeometryModified(false);
    }

    OMessageManager::get()->info("Mise a jour altimetrie des infrastructures terminee.");

    TYNameManager::get()->enable(true);
}

void TYSiteNode::updateAcoustique(const bool& force)
{
    if (_pProjet)
    {
        TYCalcul* pCalcul = _pProjet->getCurrentCalcul()._pObj;
        assert(pCalcul);
        _pInfrastructure->updateAcoustic(pCalcul, force);
    }

    for (unsigned short i = 0; i < _listSiteNode.size(); i++)
    {
        TYSiteNode* pSite = TYSiteNode::safeDownCast(_listSiteNode[i]->getElement());

        if (pSite && pSite->isInCurrentCalcul()) { pSite->updateAcoustique(force); }
    }
}

double TYSiteNode::getDelaunay()
{
    if (_pProjet) { return _pProjet->getDelaunayTolerence(); }

    double delaunay(0.0001);
#if TY_USE_IHM
    if (TYPreferenceManager::exists(TYDIRPREFERENCEMANAGER, "DelaunayTolerance"))
    {
        delaunay = TYPreferenceManager::getDouble(TYDIRPREFERENCEMANAGER, "DelaunayTolerance");
    }
    else
    {
        TYPreferenceManager::setDouble(TYDIRPREFERENCEMANAGER, "DelaunayTolerance", delaunay);
    }
#endif

    delaunay = delaunay <= 0.0 ? 0.0001 : delaunay;
    return delaunay > 0.05 ? 0.05 : delaunay;
}

//az++ (revoir les faces des ecrans; il vaudrait mieux en ajouter proprement):
// tableau d'index des faces ecrans
// According to ticket https://extranet.logilab.fr/ticket/1459658 the notion
// of ecran is obsolete
// TODO remove cleanly related stuff
vector<bool> EstUnIndexDeFaceEcran;

void TYSiteNode::getListFacesWithoutFloor(TYTabAcousticSurfaceGeoNode& tabFaces, unsigned int& nbFaceInfra, std::vector<bool>& EstUnIndexDeFaceEcran, std::vector<std::pair<int, int> >& indices, std::vector<int>& etages) const
{
    std::ofstream file;
    file.open("logsChargement.txt", ios::out | ios::trunc);
    file << "Chargement de la liste des faces." << endl;

    EstUnIndexDeFaceEcran.clear();

    unsigned int j, i;
    int compteurFace = 0;
    int compteurInfra = 0;
    TYTabAcousticSurfaceGeoNode tabTmp;

    tabFaces.clear();

    // Batiments
    for (i = 0; i < _pInfrastructure->getListBatiment().size(); i++)
    {
        file << "Chargement du batiment " << i << endl;
        // Si ce batiment est actif pour le calcul
        LPTYBatiment pBatiment = TYBatiment::safeDownCast(_pInfrastructure->getBatiment(i)->getElement());

        if (pBatiment && pBatiment->isInCurrentCalcul())
        {
            tabTmp.clear();

            // Matrice de changement de repere pour ce batiment
            OMatrix matrix = _pInfrastructure->getListBatiment()[i]->getMatrix();

            for (j = 0; j < pBatiment->getTabAcousticVol().size(); j++)
            {
                //Attempt to cast volume to a TYEtage
                LPTYEtage pEtage = TYEtage::safeDownCast(pBatiment->getAcousticVol(j));
                OMatrix matriceEtage = pBatiment->getTabAcousticVol().at(j)->getMatrix();
                if (pEtage)
                {
                    //Récupération des faces des murs
                    for (unsigned int k = 0; k < pEtage->getTabMur().size(); k++)
                    {
                        TYMur* mur = TYMur::safeDownCast(pEtage->getTabMur().at(k)->getElement());
                        OMatrix matriceMur = pEtage->getTabMur().at(k)->getMatrix();
                        if (mur)
                        {
                            file << "Récupération d'un mur rectangulaire." << endl;
                            TYAcousticRectangle* pRect = TYAcousticRectangle::safeDownCast(mur->getTabAcousticSurf().at(0)->getElement());
                            if (pRect)
                            {
                                //Conversion de la face du mur en AcousticSurfaceGeoNode
                                file << "Récupération d'un rectangle." << endl;
                                file << "Ajout de la face " << compteurFace  << ", etage " << j << ", batiment " << i << endl;
                                LPTYAcousticSurfaceGeoNode newNode = LPTYAcousticSurfaceGeoNode(new TYAcousticSurfaceGeoNode(pRect, matriceEtage * matriceMur));
                                tabTmp.push_back(newNode);
                                indices.push_back(std::pair<int, int>(compteurFace++, compteurInfra));
                                etages.push_back(j);

                            }
                        }
                    }

                    //Recovery of the upper floor only
                    TYAcousticPolygon* poly = TYAcousticPolygon::safeDownCast(pEtage->getPlafond());
                    LPTYAcousticSurfaceGeoNode newNode = LPTYAcousticSurfaceGeoNode(new TYAcousticSurfaceGeoNode(poly, matriceEtage));
                    tabTmp.push_back(newNode);
                    indices.push_back(std::pair<int, int>(compteurFace++, (int)i));
                    etages.push_back(j);

                }
                else
                {
                    // try to cast as a screen (TYEcran)
                    LPTYEcran pEcran = TYEcran::safeDownCast(pBatiment->getAcousticVol(j));

                    if (pEcran)
                    {
                        TYTabAcousticSurfaceGeoNode tabTmp2;
                        tabTmp2 = pEcran->acousticFaces();
                        for (unsigned k = 0; k < tabTmp2.size(); k++)
                        {
                            tabTmp2[k]->setMatrix(matriceEtage * tabTmp2[k]->getMatrix());
                            tabTmp.push_back(tabTmp2[k]);
                            indices.push_back(std::pair<int, int>(compteurFace++, compteurInfra));
                            etages.push_back(j);
                        }
                    }
                }
            }

            LPTYEtage pEtage = TYEtage::safeDownCast(pBatiment->getAcousticVol(0));
            bool bEtageEcran = false;
            if (pEtage)
            {
                bEtageEcran = !pEtage->getClosed();
            }
            if (bEtageEcran)
            {
                pEtage->setacousticFacesPourCalcul(true);
            }

            // L'ensemble des faces de ce batiment
            //tabTmp = TYBatiment::safeDownCast(_pInfrastructure->getBatiment(i)->getElement())->acousticFaces();

            bool bEcran = false; // element de type TYEcran
            // Next 3 lines commented, may be invalid (a building can't be a floor)
            //if (_pInfrastructure->getBatiment(i)->getElement()->isA("TYEcran"))
            //{
            //    bEcran = true;
            //}

            // Pour chacune de ces faces
            for (j = 0; j < tabTmp.size(); j++)
            {
                // On concatene les matrices
                tabTmp[j]->setMatrix(matrix * tabTmp[j]->getMatrix());

                // Ajout de la face
                tabFaces.push_back(tabTmp[j]);
                EstUnIndexDeFaceEcran.push_back(bEtageEcran || bEcran);
            }
            if (bEtageEcran)
            {
                pEtage->setacousticFacesPourCalcul(false);
            }
        }

        compteurInfra++;
    }

    // Machines
    for (i = 0; i < _pInfrastructure->getListMachine().size(); i++)
    {
        // Si cette machine est active pour le calcul
        LPTYMachine pMachine = TYMachine::safeDownCast(_pInfrastructure->getMachine(i)->getElement());
        if (pMachine && pMachine->isInCurrentCalcul())
        {
            tabTmp.clear();

            // Matrice de changement de repere pour cette machine
            OMatrix matrix = _pInfrastructure->getListMachine()[i]->getMatrix();

            // L'ensemble des faces de cette machine
            tabTmp = TYMachine::safeDownCast(_pInfrastructure->getMachine(i)->getElement())->acousticFaces();

            // Pour chacune de ces faces
            for (j = 0; j < tabTmp.size(); j++)
            {
                // On concatene les matrices
                tabTmp[j]->setMatrix(matrix * tabTmp[j]->getMatrix());

                // Ajout de la face
                tabFaces.push_back(tabTmp[j]);
                indices.push_back(std::pair<int, int>(compteurFace++, compteurInfra));
                EstUnIndexDeFaceEcran.push_back(false);
                etages.push_back(0);
            }
        }
        compteurInfra++;
    }

    nbFaceInfra = static_cast<uint32>(tabFaces.size()); // Determination du nombre de faces de l'infrastructure;

    // Les faces de la topographie (altimetrie) sont transformee en faces acoustiques
    // avec des proprietes acoustiques nulles
    //EstUnIndexDeFaceEcran n'est pas a affecter, car les faces d'infrastructures sont separees de celles de l'alti,
    //donc sachant ou commence les faces d'alti, le test "est un ecran" n'a pas de sens pour ces dernieres


    // WIP here : the materials {c/sh}ould be stored in the TYAcousticPolygon
    //            and thus be stored or exracted from the Altimetry ?
    //            or is this data pulling from the solver to be replaced by data
    //            pushing from the site to the model
    LPTYAltimetrie pAlti = getAltimetry();
    TYTabLPPolygon& listFacesAlti = pAlti->getListFaces();
    unsigned int nbFacesAlti = static_cast<uint32>(listFacesAlti.size());

    for (i = 0; i < nbFacesAlti; i++)
    {
        LPTYAcousticPolygon pAccPolygon = new TYAcousticPolygon();
        pAccPolygon->setParent(pAlti);

        // Geomtrie
        *pAccPolygon->getPolygon() = *listFacesAlti.at(i);

        // Ajout
        LPTYAcousticSurfaceGeoNode pNode = new TYAcousticSurfaceGeoNode((LPTYElement) pAccPolygon);
        tabFaces.push_back(pNode);
        indices.push_back(std::pair<int, int>(compteurFace++, -1));
        etages.push_back(-1);
    }

    file.close();

}

LPTYAltimetrie TYSiteNode::getAltimetry() const
{
    // As there is one only altimetry for all the subsites of the root site,
    // retrieve the altimetry from the root site and not from the current site.
    TYSiteNode const* rootsite = this;
    while(rootsite != nullptr && !rootsite->getRoot())
    {
        rootsite = dynamic_cast<TYSiteNode*> (rootsite->getParent());
    }
    if (rootsite != nullptr)
        return rootsite->getTopographie()->getAltimetrie();
    throw tympan::invalid_data("No root site node in current TYMPAN objects hierarchy.");
}

OMatrix TYSiteNode::getGlobalMatrix() const
{
    if (getRoot())
    {
        return OMatrix(); // identity matrix at the root site
    }
    TYSiteNode const* parentsite = dynamic_cast<TYSiteNode*> (getParent());
    if (parentsite == nullptr)
    {
        return OMatrix();
        // should throw an exception
    }

    for(int i = 0; i < parentsite->_listSiteNode.size(); i++)
    {
        TYSiteNode const* subsite =  dynamic_cast<TYSiteNode*>(parentsite->_listSiteNode[i]->getElement());
        if(subsite->getID() == getID())
        {
            return parentsite->getGlobalMatrix() * parentsite->_listSiteNode[i]->getMatrix();
        }
    }
    return OMatrix(); // this should not happen
}

bool almost_equal(double a, double b, double precision)
{
    return abs(a -b) < abs(precision);
}

void TYSiteNode::groundBasedFaces(const TYTabAcousticVolumeGeoNode& volumes,
        const OMatrix& global_matrix, std::map<TYUUID, TYTabPoint3D>& contours) const
{
    // Go through all the faces of the all the input volumes
    for (unsigned int i = 0; i < volumes.size(); i ++)
    {
        OMatrix matrix = volumes[i]->getMatrix();
        matrix = global_matrix * matrix;
        TYAcousticVolume* volume = dynamic_cast<TYAcousticVolume*>(volumes[i]->getElement());
        assert (volume != nullptr &&
                "found an object which isn't a  TYAcousticVolume in a TYTabAcousticVolumeGeoNode");
        TYTabAcousticSurfaceGeoNode faces = volume->acousticFaces();
        for (unsigned int j = 0; j < faces.size(); j++)
        {
            // Compute global matrix for the current face
            OMatrix face_matrix = matrix * faces[j]->getMatrix();
            faces[j]->setMatrix(face_matrix);
            TYAcousticSurface *pFace = dynamic_cast<TYAcousticSurface*>(faces[j]->getElement());

            // Take the contour of the acoustic face, move the points to a global scale
            // and make a polygon with them
            TYTabPoint3D contour = pFace->getOContour();
            TYPolygon poly;
            for (unsigned int k = 0; k < contour.size(); k++)
            {
                contour[k] = face_matrix * contour[k];
                poly.getPoints().push_back( TYPoint(contour[k]) );
            }
            // Compute polygon' normal
            poly.updateNormal();
            OVector3D normal = poly.normal();
            // We are looking for the floor-based face of the volumes. We keep the
            // current face if it is parallel to the ground
            if ( !almost_equal( abs(normal.scalar(OVector3D(0., 0., 1.)) ), 1., 10e-6))
                continue;
            // only keep the face if it is on the ground, i.e. z == 0
            if (almost_equal(contour[0]._z, 0, 10e-6))
            {
                contours[volume->getID()] = contour;
            }
        }
    }
}

void TYSiteNode::getFacesOnGround(std::map<TYUUID, TYTabPoint3D>& contours) const
{
    assert(contours.empty() &&
            "Output argument 'contours' is supposed to be empty when calling 'TYSiteNode::getFacesOnGround'");
    // Buildings
    for (unsigned int i = 0; i < _pInfrastructure->getListBatiment().size(); i++)
    {
        // If this building is active in the current simulation
        LPTYBatiment pBuilding = dynamic_cast<TYBatiment*>(_pInfrastructure->getBatiment(i)->getElement());
        assert(pBuilding != nullptr && "found an object which is not a TYBatiment in _pInfrastructure->getListBatiment()");
        if (pBuilding->isInCurrentCalcul())
        {
            // Building transform matrix
            OMatrix matrix = _pInfrastructure->getBatiment(i)->getMatrix();
            TYTabAcousticVolumeGeoNode& building_volumes = pBuilding->getTabAcousticVol();
            // 1 TYTabPoint3D per volume face on the ground. Indeed there may be several,
            // since buildings and machines are volume nodes wghich means they
            // are made of one or more volumes.
            std::deque<TYTabPoint3D> base_faces;
            // Get the base of the building
            groundBasedFaces(building_volumes, matrix, contours);
        }
    }
    // Machines
    for (int i = 0; i < _pInfrastructure->getListMachine().size(); i++)
    {
        // Si cette machine est active pour le calcul
        LPTYMachine pMachine = dynamic_cast<TYMachine*>(_pInfrastructure->getMachine(i)->getElement());
        assert(pMachine != nullptr && "found an object which is not a TYMachine in _pInfrastructure->getListMachine()");
        if (pMachine->isInCurrentCalcul())
        {
            // Matrice de changement de repere pour cette machine
            OMatrix matrix = _pInfrastructure->getMachine(i)->getMatrix();
            TYTabAcousticVolumeGeoNode machine_volumes = pMachine->getTabAcousticVol();
            std::deque<TYTabPoint3D> base_faces;
            // Get the base of the machine
            groundBasedFaces(machine_volumes, matrix, contours);
        }
    }
}

void TYSiteNode::getListFaces(TYTabAcousticSurfaceGeoNode& tabFaces, unsigned int& nbFaceInfra, std::vector<bool>& EstUnIndexDeFaceEcran) const
{
    EstUnIndexDeFaceEcran.clear();

    unsigned int j, i;
    TYTabAcousticSurfaceGeoNode tabTmp;

    tabFaces.clear();

    // Batiments
    for (i = 0; i < _pInfrastructure->getListBatiment().size(); i++)
    {
        // Si ce batiment est actif pour le calcul
        LPTYBatiment pBatiment = TYBatiment::safeDownCast(_pInfrastructure->getBatiment(i)->getElement());
        if (pBatiment && pBatiment->isInCurrentCalcul())
        {
            tabTmp.clear();

            // Matrice de changement de repere pour ce batiment
            OMatrix matrix = _pInfrastructure->getListBatiment()[i]->getMatrix();

            //                LPTYBatiment pBatiment = TYBatiment::safeDownCast(_pInfrastructure->getBatiment(i)->getElement());
            LPTYEtage pEtage = TYEtage::safeDownCast(pBatiment->getAcousticVol(0));

            // Old Code_TYMPAN version could use a floor as a screen so, that case should be treated
            bool bEtageEcran = false;
            if (pEtage)
            {
                bEtageEcran = !pEtage->getClosed();
            }
            if (bEtageEcran)
            {
                pEtage->setacousticFacesPourCalcul(true);
            }

            // L'ensemble des faces de ce batiment
            tabTmp = pBatiment->acousticFaces();

            // Pour chacune de ces faces
            for (j = 0; j < tabTmp.size(); j++)
            {
                // On concatene les matrices
                tabTmp[j]->setMatrix(matrix * tabTmp[j]->getMatrix());

                // Ajout de la face
                tabFaces.push_back(tabTmp[j]);
                EstUnIndexDeFaceEcran.push_back(bEtageEcran);
            }
            if (bEtageEcran)
            {
                pEtage->setacousticFacesPourCalcul(false);
            }
        }
    }

    // Machines
    for (i = 0; i < _pInfrastructure->getListMachine().size(); i++)
    {
        // Si cette machine est active pour le calcul
        LPTYMachine pMachine = TYMachine::safeDownCast(_pInfrastructure->getMachine(i)->getElement());
        if (pMachine && pMachine->isInCurrentCalcul())
        {
            tabTmp.clear();

            // Matrice de changement de repere pour cette machine
            OMatrix matrix = _pInfrastructure->getListMachine()[i]->getMatrix();

            // L'ensemble des faces de cette machine
            tabTmp = TYMachine::safeDownCast(_pInfrastructure->getMachine(i)->getElement())->acousticFaces();

            // Pour chacune de ces faces
            for (j = 0; j < tabTmp.size(); j++)
            {
                // On concatene les matrices
                tabTmp[j]->setMatrix(matrix * tabTmp[j]->getMatrix());

                // Ajout de la face
                tabFaces.push_back(tabTmp[j]);
                EstUnIndexDeFaceEcran.push_back(false);
            }
        }
    }

    nbFaceInfra = static_cast<uint32>(tabFaces.size()); // Determination du nombre de faces de l'infrastructure;

    // Les faces de la topographie (altimetrie) sont transformee en faces acoustiques
    // avec des proprietes acoustiques nulles
    //EstUnIndexDeFaceEcran n'est pas a affecter, car les faces d'infrastructures sont separees de celles de l'alti,
    //donc sachant ou commence les faces d'alti, le test "est un ecran" n'a pas de sens pour ces dernieres

    TYTabLPPolygon& listFacesAlti = getAltimetry()->getListFaces();
    unsigned int nbFacesAlti = static_cast<uint32>(listFacesAlti.size());

    for (i = 0; i < nbFacesAlti; i++)
    {
        LPTYAcousticPolygon pAccPolygon = new TYAcousticPolygon();
        pAccPolygon->setParent(getAltimetry());

        // Geomtrie
        *pAccPolygon->getPolygon() = *listFacesAlti.at(i);

        // Ajout
        LPTYAcousticSurfaceGeoNode pNode = new TYAcousticSurfaceGeoNode((LPTYElement) pAccPolygon);
        tabFaces.push_back(pNode);
    }

}

void TYSiteNode::setChildsNotInCurrentCalcul()
{
    TYTabSiteNodeGeoNode& tabGeoNode = getListSiteNode();
    TYSiteNode* pSite = NULL;

    for (unsigned int i = 0; i < tabGeoNode.size(); i++)
    {
        pSite = TYSiteNode::safeDownCast(tabGeoNode[i]->getElement());
        pSite->setInCurrentCalcul(false, true, false);
    }
}


TYTabSiteNodeGeoNode TYSiteNode::collectSites(bool include /*=true*/) const
{
    TYTabSiteNodeGeoNode sites;

    if (include)
    {
        sites.push_back(new TYSiteNodeGeoNode((TYSiteNode*) this));
    }

    for (unsigned int i = 0; i < _listSiteNode.size(); i++)
    {
        // On inclut forcement les sites sinon sans interet
        TYSiteNode* pSite = TYSiteNode::safeDownCast(_listSiteNode[i]->getElement());
        if (pSite && pSite->isInCurrentCalcul())   // Uniquement si le sous-site est dans le calcul
        {
            TYTabSiteNodeGeoNode tabChild = pSite->collectSites(true);

            // Concatenation des matrices
            OMatrix matrix = _listSiteNode[i]->getMatrix();
            for (unsigned int j = 0; j < tabChild.size(); j++)
            {
                tabChild[j]->setMatrix(matrix * tabChild[j]->getMatrix());
            }

            //...et ajoute au tableau a retourner
            sites.insert(sites.end(), tabChild.begin(), tabChild.end());
        }
    }

    return sites;
}

bool TYSiteNode::update(TYElement* pElem)
{
    bool ret = false;

    TYSourcePonctuelle* pSource = dynamic_cast<TYSourcePonctuelle*>(pElem);
    if (pSource != nullptr)
    {
        return true; // Pas de mise à jour nécessaire
    }
    TYAcousticLine* pLine = dynamic_cast<TYAcousticLine*>(pElem);
    if (pLine != nullptr) // Cas 1 : un objet de type source linéique
    {
        pLine->updateAcoustic(true);
    }
    else // Autres cas, recherche de parent et traitement approprié
    {
        TYElement* pParent = pElem; // On commencera par tester l'objet lui-meme
        do
        {
            TYAcousticVolumeNode* pVolNode = dynamic_cast<TYAcousticVolumeNode*>(pParent);
            if (pVolNode != nullptr)
            {
                ret = pVolNode->updateAcoustic(true);
                break; // On a trouvé, on peut sortir
            }
            TYSiteNode* pSite = dynamic_cast<TYSiteNode*>(pParent);
            if (pSite != nullptr)
            {
                pSite->updateAcoustique();
                ret = true;
                break; // On a trouvé, on peut sortir
            }

            pParent = pParent->getParent();
        }
        while (pParent);
    }

    return ret;
}


void TYSiteNode::update(const bool& force) // Force = false
{
    // Altimetrisation des infrastructures du site
    updateAltiInfra();

    // Mise a jour de l'acoustique des elements du site
    updateAcoustique(force);

    // Et celle des sites inclus
    for (unsigned short i = 0; i < _listSiteNode.size(); i++)
    {
        TYSiteNode* pSite = TYSiteNode::safeDownCast(_listSiteNode[i]->getElement());

        if (pSite && pSite->isInCurrentCalcul()) { pSite->update(force); }
    }

    // Si le site est dans un projet, on altimétrise les points de controle
    if (_pProjet && getRoot())
    {
        _pProjet->updateAltiRecepteurs();
    }
}

bool TYSiteNode::addSiteNode(LPTYSiteNodeGeoNode pSiteNodeGeoNode)
{
    assert(pSiteNodeGeoNode);

    LPTYSiteNode pSite = TYSiteNode::safeDownCast(pSiteNodeGeoNode->getElement());

    assert(pSite);

    pSiteNodeGeoNode->setParent(this);
    pSite->setParent(this);
    pSite->setProjet(_pProjet); // Informe du projet cadre

    _listSiteNode.push_back(pSiteNodeGeoNode);

#if TY_USE_IHM
    pSite->updateGraphicTree();
#endif
    setIsGeometryModified(true);

    return true;
}



bool TYSiteNode::addSiteNode(LPTYSiteNode pSiteNode)
{
    return addSiteNode(new TYSiteNodeGeoNode((LPTYElement)pSiteNode));
}

bool TYSiteNode::remSiteNode(const LPTYSiteNodeGeoNode pSiteNodeGeoNode)
{
    assert(pSiteNodeGeoNode);
    bool ret = false;
    TYTabSiteNodeGeoNode::iterator ite;

    for (ite = _listSiteNode.begin(); ite != _listSiteNode.end(); ite++)
    {
        if ((*ite) == pSiteNodeGeoNode)
        {
            // Suppression des calcul
            if (_pProjet) { _pProjet->remElmtFromCalculs((*ite)->getElement()); }

            _listSiteNode.erase(ite);
            ret = true;
            break;
        }
    }

    setIsGeometryModified(true);

    return ret;
}

bool TYSiteNode::remSiteNode(const LPTYSiteNode pSiteNode)
{
    assert(pSiteNode);
    bool ret = false;
    TYTabSiteNodeGeoNode::iterator ite;

    for (ite = _listSiteNode.begin(); ite != _listSiteNode.end(); ite++)
    {
        if (TYSiteNode::safeDownCast((*ite)->getElement()) == pSiteNode)
        {
            // Suppression des calcul
            if (_pProjet) { _pProjet->remElmtFromCalculs((*ite)->getElement()); }

            _listSiteNode.erase(ite);
            ret = true;
            break;
        }
    }

    setIsGeometryModified(true);

    return ret;
}

bool TYSiteNode::remSiteNode(QString idSiteNode)
{
    bool ret = false;
    TYTabSiteNodeGeoNode::iterator ite;

    for (ite = _listSiteNode.begin(); ite != _listSiteNode.end(); ite++)
    {
        if (TYSiteNode::safeDownCast((*ite)->getElement())->getID().toString() == idSiteNode)
        {
            // Suppression des calcul
            if (_pProjet) { _pProjet->remElmtFromCalculs((*ite)->getElement()); }

            _listSiteNode.erase(ite);
            ret = true;
            break;
        }
    }

    setIsGeometryModified(true);

    return ret;
}

LPTYSiteNodeGeoNode TYSiteNode::findSiteNode(const LPTYSiteNode pSiteNode)
{
    assert(pSiteNode);
    TYTabSiteNodeGeoNode::iterator ite;

    for (ite = _listSiteNode.begin(); ite != _listSiteNode.end(); ite++)
    {
        if (TYSiteNode::safeDownCast((*ite)->getElement()) == pSiteNode)
        {
            return (*ite);
        }
    }

    return NULL;
}

LPTYSiteNode TYSiteNode::merge()
{
    LPTYSiteNode pSite = new TYSiteNode();
    pSite->getTopographie()->getListTerrain().clear(); // On vide car un terrain par defaut a ete cree dans le site cible

    unsigned int j;

    // On copie les elements de ce site
    appendSite(this, OMatrix(), pSite);
    pSite->getTopographie()->setDefTerrainIdx(getTopographie()->getDefTerrainIdx());
    pSite->getTopographie()->setEmprise(getTopographie()->getEmprise(), false);
    pSite->getTopographie()->setDefTerrain(getTopographie()->getDefTerrainIdx());

    // Merge des sites enfants
    for (j = 0; j < _listSiteNode.size(); j++)
    {
        // Site enfant courant
        LPTYSiteNodeGeoNode pSiteNodeGeoNode = _listSiteNode[j];

        // Appel recursif
        TYSiteNode* pSiteChild = TYSiteNode::safeDownCast(pSiteNodeGeoNode->getElement());
        assert(pSiteChild);
        if (pSiteChild && !(pSiteChild->isInCurrentCalcul())) { continue; }

        LPTYSiteNode pSiteTmp = pSiteChild->merge();

        // On copie les elements du site enfant en prenant compte le changement de repere
        appendSite(pSiteTmp, pSiteNodeGeoNode->getMatrix(), pSite);
    }

    pSite->getTopographie()->sortTerrainsBySurface();
    pSite->setProjet(_pProjet);

    return pSite;
}

void TYSiteNode::appendSite(LPTYSiteNode pSiteFrom, const OMatrix& matrix, LPTYSiteNode pSiteTo)
{
    assert(pSiteFrom);
    assert(pSiteTo);
    unsigned int i;
    OMatrix newMatrix;

    // Ajout des elements de topo
    LPTYTopographie pTopoFrom = pSiteFrom->getTopographie();
    LPTYTopographie pTopoTo = pSiteTo->getTopographie();

    // TODO BUG #0008309 : Courbe de niveau pas prise en compte dans l'altimétrie
    if (pSiteFrom->getUseEmpriseAsCrbNiv())
    {
        pTopoTo->addCrbNiv(new TYCourbeNiveau(pTopoFrom->getEmprise(), pSiteFrom->getAltiEmprise()));
    }
    // This scope is here to make p_ground local
    {
        // Add the 'emprise' as a Terrain with its defaultTErrain as Sol attribute.
        TYTerrain* p_ground = new TYTerrain();
        p_ground->setSol(pTopoFrom->getDefTerrain()->getSol());
        p_ground->setListPoints(pTopoFrom->getEmprise());
        pTopoTo->addTerrain(new TYTerrainGeoNode(p_ground, matrix));
    }
    for (i = 0; i < pTopoFrom->getListCrbNiv().size(); i++)
    {
        newMatrix = matrix * pTopoFrom->getListCrbNiv()[i]->getMatrix();
        pTopoTo->addCrbNiv(new TYCourbeNiveauGeoNode(pTopoFrom->getListCrbNiv()[i]->getElement(), newMatrix));
    }

    for (i = 0; i < pTopoFrom->getListTerrain().size(); i++)
    {
        newMatrix = matrix * pTopoFrom->getListTerrain()[i]->getMatrix();
        pTopoTo->addTerrain(new TYTerrainGeoNode(pTopoFrom->getListTerrain()[i]->getElement(), newMatrix));
    }

    for (i = 0; i < pTopoFrom->getListCrsEau().size(); i++)
    {
        newMatrix = matrix * pTopoFrom->getListCrsEau()[i]->getMatrix();
        pTopoTo->addCrsEau(new TYCoursEauGeoNode(pTopoFrom->getListCrsEau()[i]->getElement(), newMatrix));
    }

    for (i = 0; i < pTopoFrom->getListPlanEau().size(); i++)
    {
        newMatrix = matrix * pTopoFrom->getListPlanEau()[i]->getMatrix();
        pTopoTo->addPlanEau(new TYPlanEauGeoNode(pTopoFrom->getListPlanEau()[i]->getElement(), newMatrix));
    }


    // Ajout des elements d'infra
    LPTYInfrastructure pInfraFrom = pSiteFrom->getInfrastructure();
    LPTYInfrastructure pInfraTo = pSiteTo->getInfrastructure();

    for (i = 0; i < pInfraFrom->getListRoute().size(); i++)
    {
        newMatrix = matrix * pInfraFrom->getListRoute()[i]->getMatrix();
        pInfraTo->addRoute(new TYRouteGeoNode(pInfraFrom->getListRoute()[i]->getElement(), newMatrix));
    }

    for (i = 0; i < pInfraFrom->getListResTrans().size(); i++)
    {
        newMatrix = matrix * pInfraFrom->getListResTrans()[i]->getMatrix();
        pInfraTo->addResTrans(new TYReseauTransportGeoNode(pInfraFrom->getListResTrans()[i]->getElement(), newMatrix));
    }

    for (i = 0; i < pInfraFrom->getListBatiment().size(); i++)
    {
        newMatrix = matrix * pInfraFrom->getListBatiment()[i]->getMatrix();
        TYBatimentGeoNode* pBatNode = new TYBatimentGeoNode(pInfraFrom->getListBatiment()[i]->getElement(), newMatrix);
        pBatNode->setHauteur(pInfraFrom->getListBatiment()[i]->getHauteur());
        pInfraTo->addBatiment(pBatNode);
    }

    for (i = 0; i < pInfraFrom->getListMachine().size(); i++)
    {
        newMatrix = matrix * pInfraFrom->getListMachine()[i]->getMatrix();
        TYMachineGeoNode* pMachineNode = new TYMachineGeoNode(pInfraFrom->getListMachine()[i]->getElement(), newMatrix);
        pMachineNode->setHauteur(pInfraFrom->getListMachine()[i]->getHauteur());
        pInfraTo->addMachine(pMachineNode);
    }

    for (i = 0; i < pInfraFrom->getSrcs().size(); i++)
    {
        newMatrix = matrix * pInfraFrom->getSrcs()[i]->getMatrix();
        pInfraTo->addSrc(new TYSourcePonctuelleGeoNode(pInfraFrom->getSrcs()[i]->getElement(), newMatrix));
    }

}

void TYSiteNode::exportCSV(std::ofstream& ofs)
{
    // Export du nom de l'objet
    ofs << getName().toAscii().data() << '\n';

    // Export du type de l'objet
    ofs << toString() << '\n';
    // Export des donnees acoustiques
    LPTYElementArray childs;
    getChilds(childs);

    for (int i = 0; i < childs.size() ; i++)
    {
        TYElement* pElement = childs[i];
        TYSiteNode* pSite = dynamic_cast<TYSiteNode*>(pElement);
        if (pSite != nullptr)
        {
            // Export du nom de l'objet
            ofs << pElement->getName().toAscii().data() << '\n';
            continue;
        }
        LPTYAcousticVolumeNode pVolNode = dynamic_cast<TYAcousticVolumeNode*>(pElement);
        if (pVolNode != nullptr)
        {
            pVolNode->exportCSV(ofs);
            continue;
        }
        LPTYAcousticLine pAcLine = dynamic_cast<TYAcousticLine*>(pElement);
        if (pAcLine != nullptr)
        {
            pAcLine->exportCSV(ofs);
            continue;
        }
        LPTYUserSourcePonctuelle pSource = dynamic_cast<TYUserSourcePonctuelle*>(pElement);
        if (pSource != nullptr)
        {
            pSource->exportCSV(ofs);
        }
    }

    ofs << '\n';
}
