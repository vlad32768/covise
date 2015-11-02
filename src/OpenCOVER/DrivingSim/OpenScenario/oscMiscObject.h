/* This file is part of COVISE.

You can use it under the terms of the GNU Lesser General Public License
version 2.1 or later, see lgpl-2.1.txt.

* License: LGPL 2+ */
#ifndef OSC_MISC_OBJECT_H
#define OSC_MISC_OBJECT_H
#include <oscExport.h>
#include <oscObjectBase.h>
#include <oscObjectVariable.h>
#include <oscVariables.h>
#include <oscHeader.h>
#include <oscFile.h>
//#include <oscNamedObject.h>
#include <oscDimension.h>

namespace OpenScenario {

class OpenScenarioBase;

/// \class This class represents a generic OpenScenario Object
class OPENSCENARIOEXPORT oscMiscObject: public oscObjectBase
{
public:
    oscMiscObject()
    {
		OSC_OBJECT_ADD_MEMBER(header,"oscHeader");
		OSC_ADD_MEMBER(name);
		OSC_ADD_MEMBER(type);
		OSC_ADD_MEMBER(mass);
		OSC_OBJECT_ADD_MEMBER(dimension,"oscDimension");
		OSC_OBJECT_ADD_MEMBER(Geometry,"oscFile");
    };
    oscHeaderMember header;
	oscString name;
	oscString type;
	oscDouble mass;
	oscDimensionMember dimension;
	oscFileMember Geometry;
};

typedef oscObjectVariable<oscMiscObject *> oscMiscObjectMember;

}

#endif //OSC_MISC_OBJECT_H