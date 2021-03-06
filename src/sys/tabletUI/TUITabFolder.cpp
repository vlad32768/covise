/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

#include <assert.h>
#include <stdio.h>

#include <QTabWidget>

#include "TUITabFolder.h"
#include "TUITab.h"
#include "TUIApplication.h"

/// Constructor
TUITabFolder::TUITabFolder(int id, int type, QWidget *w, int parent, QString name)
    : TUIContainer(id, type, w, parent, name)
{
    tabWidget = new QTabWidget(w);
    widget = tabWidget;
    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(valueChanged(int)));
}

/// Destructor
TUITabFolder::~TUITabFolder()
{
    removeAllChildren();
    delete widget;
}

void TUITabFolder::valueChanged(int index)
{
    QWidget *tab = tabWidget->widget(index);
    for (ElementList::iterator it = elements.begin(); it != elements.end(); ++it)
    {
        TUIElement *el = *it;
        if (el->getWidget() == tab)
        {
            if (strcmp(el->getClassName(), "TUITab") == 0)
            {
                assert(dynamic_cast<TUITab *>(el));
                ((TUITab *)el)->activated();
            }
            // else cant activate other elements, just tabs
        }
    }
}

/** Appends a child widget to this elements layout.
  @param el element to add
*/
void TUITabFolder::addElementToLayout(TUIElement *el)
{
    if (tabWidget->indexOf(el->getWidget()) < 0)
    {
        if (!el->isHidden())
            tabWidget->addTab(el->getWidget(), el->getLabel());
    }
    else
    {
        tabWidget->setTabText(tabWidget->indexOf(el->getWidget()), el->getLabel());
    }
}

char *TUITabFolder::getClassName()
{
    return (char *)"TUITabFolder";
}

bool TUITabFolder::isOfClassName(char *classname)
{
    // paranoia makes us mistrust the string library and check for NULL.
    if (classname && getClassName())
    {
        // check for identity
        if (!strcmp(classname, getClassName()))
        { // we are the one
            return true;
        }
        else
        { // we are not the wanted one. Branch up to parent class
            return TUIElement::isOfClassName(classname);
        }
    }

    // nobody is NULL
    return false;
}
