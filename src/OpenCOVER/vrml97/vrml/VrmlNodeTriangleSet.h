/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeTriangleSet.h

#ifndef _VRMLNODETRIANGLESET_
#define _VRMLNODETRIANGLESET_

#include "VrmlNodePolygonsCommon.h"

namespace vrml
{

class VRMLEXPORT VrmlNodeTriangleSet : public VrmlNodePolygonsCommon

{

public:
    // Define the fields of indexed face set nodes
    static VrmlNodeType *defineType(VrmlNodeType *t = 0);
    virtual VrmlNodeType *nodeType() const;

    VrmlNodeTriangleSet(VrmlScene *);
    virtual ~VrmlNodeTriangleSet();

    virtual VrmlNode *cloneMe() const;

    virtual Viewer::Object insertGeometry(Viewer *v);

    virtual VrmlNodeTriangleSet *toTriangleSet() const;
};
}
#endif // _VRMLNODETRIANGLESET_
