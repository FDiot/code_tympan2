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

/*
 *
 */

#ifndef __TY_COURSEAU__
#define __TY_COURSEAU__


#include "Tympan/models/business/geoacoustic/TYAcousticLine.h"
#include "Tympan/gui/dllexport.h"

/**
 * Classe de definition du chemin d'un cours d'eau et de ses proprietes acoustiques.
 */
class TYMPAN_EXPORT TYCoursEau: public TYAcousticLine
{
    OPROTODECL(TYCoursEau)
    TY_EXTENSION_DECL_ONLY(TYCoursEau)
    TY_EXT_GRAPHIC_DECL_ONLY(TYCoursEau)

    // Methodes
public:
    /**
     * Constructeur.
     */
    TYCoursEau();
    /**
     * Constructeur par copie.
     */
    TYCoursEau(const TYCoursEau& other);
    /**
     * Destructeur.
     */
    virtual ~TYCoursEau();

    ///Operateur =.
    TYCoursEau& operator=(const TYCoursEau& other);
    ///Operateur ==.
    bool operator==(const TYCoursEau& other) const;
    ///Operateur !=.
    bool operator!=(const TYCoursEau& other) const;

    virtual std::string toString() const;

    virtual DOM_Element toXML(DOM_Element& domElement);
    virtual int fromXML(DOM_Element domElement);

    // Membres
protected:

};


///Noeud geometrique de type TYCoursEau.
typedef TYGeometryNode TYCoursEauGeoNode;
///Smart Pointer sur TYCoursEauGeoNode.
typedef SmartPtr<TYCoursEauGeoNode> LPTYCoursEauGeoNode;
///Collection de noeuds geometriques de type TYCoursEau.
typedef std::vector<LPTYCoursEauGeoNode> TYTabCoursEauGeoNode;


#endif // __TY_COURSEAU__
