/**
 * \file test_OBox.cpp
 * \test Testing of OBox libs used by lot of TYMPAN features
 *
 *  Created on: november 6, 2013
 *      Author: Denis THOMASSON <denis.thomasson@edf.fr>
 */

#include <cstdlib>

#include "gtest/gtest.h"
#include "Tympan/MetierSolver/ToolsMetier/OGeometrie.h"
#include "Tympan/MetierSolver/ToolsMetier/OCoord3D.h"
#include "Tympan/MetierSolver/ToolsMetier/OPoint3D.h"
#include "Tympan/MetierSolver/ToolsMetier/OVector3D.h"
#include "Tympan/MetierSolver/ToolsMetier/ORepere3D.h"
#include "Tympan/MetierSolver/ToolsMetier/OBox.h"
#include "Tympan/MetierSolver/ToolsMetier/OBox2.h"

using std::cout;
using std::cerr;
using std::endl;

TEST(IsInsideTest, dumpenv) {
	// Cr�ation des points min et max
	OCoord3D pt1(-2.0, -4.0, -3.0);
	OCoord3D pt2(5.0, 2.0, 1.0);
	
	// Cr�ation de la box
	ORepere3D rep;
	OBox box(pt1, pt2);
	OBox2 box2(box);
	
	// Cr�ation du point � tester
	OPoint3D pt(-1.0, -3.0, -2.0);
	
	// Test positif
	bool resu = box2.isInside(pt);
	ASSERT_TRUE(resu == true);
	
	// Test n�gatif
	pt._z = 3.0;
	resu = box2.isInside(pt);
	ASSERT_FALSE(resu == true);
}

TEST(IsInside2DTest, dumpenv) {
	// Cr�ation des points min et max
	OCoord3D pt1(-2.0, -4.0, -3.0);
	OCoord3D pt2(5.0, 2.0, 1.0);
	
	// Cr�ation de la box
	ORepere3D rep;
	OBox box(pt1, pt2);
	OBox2 box2(box);
	
	// Cr�ation du point � tester
	OPoint3D pt(-1.0, -3.0, -2.0);
	
	// Test positif (1) : Point is inside 2D and 3D box
	bool resu = box2.isInside2D(pt);
	ASSERT_TRUE(resu == true);
	
	// Test positif (2) : Point is inside 2D but not 3D box
	pt._z = 3.0;
	resu = box2.isInside2D(pt);
	ASSERT_TRUE(resu == true);

	// Test n�gatif : Point is outside 2D box
	pt._x = -5;
	resu = box2.isInside2D(pt);
	ASSERT_FALSE(resu == true);
}

TEST(translateTest, dumpenv) {
	// Cr�ation des points min et max
	OCoord3D pt1(-2.0, -4.0, -3.0);
	OCoord3D pt2(5.0, 2.0, 1.0);
	
	// Cr�ation de la box
	ORepere3D rep;
	OBox box(pt1, pt2);
	OBox2 box2(box);
	
	// Cr�ation du point servant � translater
	OVector3D vt(-1.0, -3.0, -2.0);
	
	// Test positif (1) : Point is inside 2D and 3D box
	box2.Translate(vt);

	ASSERT_TRUE( box2._A == OPoint3D(-3.0, -5.0, -4.0) );
	ASSERT_TRUE( box2._center == OPoint3D( 0.5, -2.0, -2.0 ) );
	ASSERT_TRUE( box2._H == OPoint3D( 4.0, 1.0, 0.0 ) );
}

TEST(rotationTest, dumpenv) {
	// Cr�ation des points min et max
	OCoord3D pt1(-2.0, -4.0, -3.0);
	OCoord3D pt2(5.0, 2.0, 1.0);
	
	// Cr�ation de la box
	ORepere3D rep;
	OBox box(pt1, pt2);
	OBox2 box2(box);
	
	// Cr�ation du point servant � translater
	OVector3D vt(-1.0, -3.0, -2.0);
	
	// Test positif (1) : Point is inside 2D and 3D box
	box2.Translate(vt);

	ASSERT_TRUE( box2._A == OPoint3D(-3.0, -5.0, -4.0) );
	ASSERT_TRUE( box2._center == OPoint3D( 0.5, -2.0, -2.0 ) );
	ASSERT_TRUE( box2._H == OPoint3D( 4.0, 1.0, 0.0 ) );
}