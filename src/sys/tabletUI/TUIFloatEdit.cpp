/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

#include <assert.h>
#include <stdio.h>

#include <QLabel>
#include <QString>
#include <QLineEdit>

#if !defined _WIN32_WCE && !defined ANDROID_TUI
#include <net/tokenbuffer.h>
#else
#include <wce_msg.h>
#endif

#include "TUIFloatEdit.h"
#include "TUIApplication.h"
#include "TUIContainer.h"
#include "TUILineCheck.h"

/// Constructor
TUIFloatEdit::TUIFloatEdit(int id, int type, QWidget *w, int parent, QString name)
    : TUIElement(id, type, w, parent, name)
{
    string = new TUILineCheck(w);
    connect(string, SIGNAL(contentChanged()), this, SLOT(valueChanged()));
}

/// Destructor
TUIFloatEdit::~TUIFloatEdit()
{
    delete widget;
}

void TUIFloatEdit::setPos(int x, int y)
{
    xPos = x;
    yPos = y;
    TUIContainer *parent;
    widget = string;
    if ((parent = getParent()))
    {
        parent->addElementToLayout(this);
    }
    else
    {
        TUIMainWindow::getInstance()->addElementToLayout(this);
    }
    string->setVisible(!hidden);
}

void TUIFloatEdit::valueChanged()
{
    value = string->text().toFloat();
    covise::TokenBuffer tb;
    tb << ID;
    tb << value;
    TUIMainWindow::getInstance()->send(tb);
}

/** Set activation state of this container and all its children.
  @param en true = elements enabled
*/
void TUIFloatEdit::setEnabled(bool en)
{
    TUIElement::setEnabled(en);
}

/** Set highlight state of this container and all its children.
  @param hl true = element highlighted
*/
void TUIFloatEdit::setHighlighted(bool hl)
{
    TUIElement::setHighlighted(hl);
}

char *TUIFloatEdit::getClassName()
{
    return (char *)"TUIFloatEdit";
}

bool TUIFloatEdit::isOfClassName(char *classname)
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

void TUIFloatEdit::setValue(int type, covise::TokenBuffer &tb)
{
    //cerr << "setValue " << type << endl;
    if (type == TABLET_MIN)
    {
    }
    else if (type == TABLET_MAX)
    {
    }
    else if (type == TABLET_FLOAT)
    {
        value = 123.4f;
        tb >> value;
        //cerr << "TUIFloatEdit::setValue " << value << endl;
        QString tmp;
        tmp = QString("%1").arg(value);
        string->setText(tmp);
    }
    TUIElement::setValue(type, tb);
}
