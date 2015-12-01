/* This file is part of COVISE.

You can use it under the terms of the GNU Lesser General Public License
version 2.1 or later, see lgpl-2.1.txt.

* License: LGPL 2+ */
#ifndef OSC_DRIVER_REF_H
#define OSC_DRIVER_REF_H
#include <oscExport.h>
#include <oscObjectBase.h>
#include <oscObjectVariable.h>
#include <oscVariables.h>
#include <oscNameId.h>

namespace OpenScenario {

class OpenScenarioBase;

/// \class This class represents a generic OpenScenario Object
class OPENSCENARIOEXPORT oscDriverRef: public oscObjectBase
{
public:
    oscDriverRef()
    {
        OSC_OBJECT_ADD_MEMBER(name,"oscNameId");
    };
    oscNameIdMember name;
};

typedef oscObjectVariable<oscDriverRef *> oscDriverRefMember;

}

#endif //OSC_DRIVER_REF_H