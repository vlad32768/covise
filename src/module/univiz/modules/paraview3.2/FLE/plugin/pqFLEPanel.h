/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

#ifndef _pqFLEPanel_h
#define _pqFLEPanel_h

#include "pqAutoGeneratedObjectPanel.h"
#include "pqObjectPanelInterface.h"

class pqFLEPanel : public pqAutoGeneratedObjectPanel
{
    Q_OBJECT
public:
    pqFLEPanel(pqProxy *proxy, QWidget *p);
    ~pqFLEPanel();

private slots:
    void onUnsteadyChanged(int state);
    void onModeChanged(int index);
};

#endif
