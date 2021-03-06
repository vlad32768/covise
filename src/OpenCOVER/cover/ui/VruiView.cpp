#include "VruiView.h"

#include <cassert>

#include <OpenVRUI/coMenuItem.h>
#include <OpenVRUI/coRowMenu.h>
#include <OpenVRUI/coLabelMenuItem.h>
#include <OpenVRUI/coButtonMenuItem.h>
#include <OpenVRUI/coCheckboxMenuItem.h>
#include <OpenVRUI/coSubMenuItem.h>
#include <OpenVRUI/coPotiMenuItem.h>
#include <OpenVRUI/coSliderMenuItem.h>
#include <OpenVRUI/coCheckboxGroup.h>

#include "Element.h"
#include "Menu.h"
#include "ButtonGroup.h"
#include "Label.h"
#include "Action.h"
#include "Button.h"
#include "Slider.h"
#include "SelectionList.h"

#include <cover/coVRPluginSupport.h>
#include <config/CoviseConfig.h>

using namespace vrui;

namespace opencover {
namespace ui {

VruiView::VruiView()
: View("vrui")
{
#if 0
    m_rootMenu = new coRowMenu("COVER2", nullptr);
    m_rootMenu->setVisible(true);
#else
    m_rootMenu = cover->getMenu();
#endif
}

coMenu *VruiView::getMenu(const Element *elem) const
{
    manager()->update();

    auto ve = vruiElement(elem);
    if (ve)
        return ve->m_menu;
    return nullptr;
}

coMenuItem *VruiView::getItem(const Element *elem) const
{
    manager()->update();

    auto ve = vruiElement(elem);
    if (ve)
        return ve->m_menuItem;
    return nullptr;
}

VruiViewElement *VruiView::vruiElement(const std::string &path) const
{
    auto e = viewElement(path);
    auto ve = dynamic_cast<VruiViewElement *>(e);
    assert(!e || ve);
    return ve;
}

VruiViewElement *VruiView::vruiElement(const Element *elem) const
{
    if (!elem)
        return nullptr;
    auto e = viewElement(elem);
    auto ve = dynamic_cast<VruiViewElement *>(e);
    assert(!e || ve);
    return ve;
}

VruiViewElement *VruiView::vruiParent(const Element *elem) const
{
    return vruiElement(elem->parent());
}


VruiViewElement *VruiView::vruiContainer(const Element *elem) const
{
    auto p = vruiParent(elem);
    if (p)
    {
        if (p->m_menu)
            return p;
        return vruiParent(p->element);
    }
    return nullptr;
}

void VruiView::add(VruiViewElement *ve, const Element *elem)
{
    if (!ve)
        std::cerr << "Warning: no VruiViewElement" << std::endl;
    if (!elem)
        std::cerr << "Warning: Element" << std::endl;

    auto parent = vruiContainer(elem);

    std::string configPath = "COVER.UI." + elem->path();
    bool exists = false;
    std::string parentPath = covise::coCoviseConfig::getEntry("parent", configPath, &exists);
    //std::cerr << "config: " << configPath << " parent: " << parentPath << std::endl;
    if (exists)
    {
        auto p = vruiElement(parentPath);
        if (!p)
        {
            //std::cerr << "ui::Vrui: did not find configured parent '" << parentPath << "' for '" << elem->path() << "'" << std::endl;
        }
        parent = p;
    }

    if (ve->m_menuItem)
    {
        if (parent && parent->m_menu)
        {
            parent->m_menu->add(ve->m_menuItem);
        }
        else
        {
            if (parent)
            {
                std::cerr << "ui::Vrui: parent " << parent->element->path() << " for " << elem->path() << " is not a menu" << std::endl;
            }
            if (m_rootMenu)
                m_rootMenu->add(ve->m_menuItem);
        }
        if (ve->m_menuItem)
            ve->m_menuItem->setMenuListener(ve);
    }
}

void VruiView::updateEnabled(const Element *elem)
{

}

void VruiView::updateVisible(const Element *elem)
{
    auto ve = vruiElement(elem);
    if (ve)
    {
        if (auto m = dynamic_cast<const Menu *>(elem))
        {
            if (auto smi = dynamic_cast<coSubMenuItem *>(ve->m_menuItem))
            {
                if (!m->visible())
                {
                    smi->closeSubmenu();
                    delete smi;
                    ve->m_menuItem = nullptr;
                }
            } else if (!ve->m_menuItem) {
                if (m->visible())
                {
                    auto smi = new coSubMenuItem(m->text()+"...");
                    smi->setMenu(ve->m_menu);
                    ve->m_menuItem = smi;
                    add(ve, m);
                }
            }
            if (ve->m_menu)
            {
                if (!elem->visible())
                    ve->m_menu->setVisible(elem->visible());
            }
        }
    }
}

void VruiView::updateText(const Element *elem)
{
    auto ve = vruiElement(elem);
    if (ve)
    {
        ve->m_text = elem->text();
        std::string itemText = elem->text();
        if (auto m = dynamic_cast<const Menu *>(elem))
        {
            itemText += "...";
        }
        if (ve->m_menuItem)
            ve->m_menuItem->setName(itemText);
        if (auto re = dynamic_cast<coRowMenu *>(ve->m_menu))
            re->updateTitle(ve->m_text.c_str());
    }
}

void VruiView::updateState(const Button *button)
{
    auto ve = vruiElement(button);
    if (ve)
    {
        if (auto cb = dynamic_cast<coCheckboxMenuItem *>(ve->m_menuItem))
            cb->setState(button->state(), false);
    }
}

void VruiView::updateChildren(const Menu *menu)
{
}

void VruiView::updateChildren(const SelectionList *sl)
{
    auto ve = vruiElement(sl);
    if (!ve)
        return;

    auto m = dynamic_cast<coRowMenu *>(ve->m_menu);
    if (!m)
        return;

    const auto items = sl->items();
    while (m->getItemCount() < items.size())
    {
        int i = m->getItemCount();
        coCheckboxMenuItem *item = new coCheckboxMenuItem(items[i], false);
        item->setMenuListener(ve);
        m->add(item);
    }
    auto all = m->getAllItems();
    while (m->getItemCount() > items.size())
    {
        int i = m->getItemCount()-1;
        m->remove(all[i]);
    }
    for (size_t i=0; i<items.size(); ++i)
    {
        auto cb = dynamic_cast<coCheckboxMenuItem *>(all[i]);
        if (!cb)
        {
            std::cerr << "VruiView::updateChildren(SelectionList): menu entry is not a checkbox" << std::endl;
            continue;
        }
        cb->setName(items[i]);
        cb->setState(sl->selection()[i]);
    }

    std::string t = sl->text();
    int s = sl->selectedIndex();
    if (s >= 0)
    {
        t += ": ";
        t += sl->items()[s];
    }
    if (ve->m_menuItem)
        ve->m_menuItem->setName(t);
}

void VruiView::updateInteger(const Slider *slider)
{
    auto ve = vruiElement(slider);
    if (!ve)
        return;
    if (auto vp = dynamic_cast<coPotiMenuItem *>(ve->m_menuItem))
    {
        vp->setInteger(slider->integer());
    }
    else if (auto vs = dynamic_cast<coSliderMenuItem *>(ve->m_menuItem))
    {
        vs->setInteger(slider->integer());
    }
}

void VruiView::updateValue(const Slider *slider)
{
    auto ve = vruiElement(slider);
    if (!ve)
        return;
    if (auto vp = dynamic_cast<coPotiMenuItem *>(ve->m_menuItem))
    {
        vp->setValue(slider->value());
    }
    else if (auto vs = dynamic_cast<coSliderMenuItem *>(ve->m_menuItem))
    {
        vs->setValue(slider->value());
    }
}

void VruiView::updateBounds(const Slider *slider)
{
    auto ve = vruiElement(slider);
    if (!ve)
        return;
    if (auto vp = dynamic_cast<coPotiMenuItem *>(ve->m_menuItem))
    {
        vp->setMin(slider->min());
        vp->setMax(slider->max());
    }
    else if (auto vs = dynamic_cast<coSliderMenuItem *>(ve->m_menuItem))
    {
        vs->setMin(slider->min());
        vs->setMax(slider->max());
    }
}

VruiViewElement *VruiView::elementFactoryImplementation(Label *label)
{
    auto ve = new VruiViewElement(label);
    ve->m_menuItem = new coLabelMenuItem(label->text());
    add(ve, label);
    return ve;
}

VruiViewElement *VruiView::elementFactoryImplementation(Button *button)
{
    auto ve = new VruiViewElement(button);
    auto parent = vruiParent(button);
    vrui::coCheckboxGroup *vrg = nullptr;
    if (parent)
        vrg = parent->m_group;
    ve->m_menuItem = new coCheckboxMenuItem(button->text(), button->state(), vrg);
    add(ve, button);
    return ve;
}

VruiViewElement *VruiView::elementFactoryImplementation(Group *group)
{
    auto ve = new VruiViewElement(group);
    //ve->m_group = new vrui::coCheckboxGroup;
    //ve->m_menuItem = new coCheckboxMenuItem(rg->text(), rg->state());
    //add(ve, rg);
#if 0
    auto parent = vruiParent(rg);
    if (parent)
    {
        ve->m_menu = parent->m_menu;
    }
    else
    {
        ve->m_menu = m_rootMenu;
    }
#endif
    return ve;
}


VruiViewElement *VruiView::elementFactoryImplementation(Slider *slider)
{
    auto ve = new VruiViewElement(slider);
    switch (slider->presentation())
    {
    case Slider::AsSlider:
        ve->m_menuItem = new coSliderMenuItem(slider->text(), slider->min(), slider->max(), slider->value());
        break;
    case Slider::AsDial:
        ve->m_menuItem = new coPotiMenuItem(slider->text(), slider->min(), slider->max(), slider->value());
        break;
    }
    add(ve, slider);
    return ve;

}

VruiViewElement *VruiView::elementFactoryImplementation(SelectionList *sl)
{
    auto parent = vruiContainer(sl);
    auto ve = new VruiViewElement(sl);
    auto smi  = new coSubMenuItem(sl->name());
    ve->m_menuItem = smi;
    ve->m_menu = new coRowMenu(ve->m_text.c_str(), parent ? parent->m_menu : m_rootMenu);
    smi->setMenu(ve->m_menu);
    smi->closeSubmenu();
    add(ve, sl);
    return ve;
}

VruiViewElement *VruiView::elementFactoryImplementation(Menu *menu)
{
    auto parent = vruiContainer(menu);
#if 0
    std::cerr <<"Vrui: creating menu: text=" << menu->text() << std::endl;
    if (parent)
    {
        std::cerr <<"   parent: text=" << parent->element->text() << std::endl;
        std::cerr <<"   parent: menu=" << parent->m_menu << std::endl;
    }
#endif
    auto ve = new VruiViewElement(menu);
    auto smi  = new coSubMenuItem(menu->name()+"...");
    ve->m_menuItem = smi;
    add(ve, menu);
    ve->m_menu = new coRowMenu(ve->m_text.c_str(), parent ? parent->m_menu : m_rootMenu);
    smi->setMenu(ve->m_menu);
    smi->closeSubmenu();
    //ve->m_menu->closeMenu();
    //ve->m_menu->show();
    //ve->m_menu->setVisible(false);
    return ve;
}

VruiViewElement *VruiView::elementFactoryImplementation(Action *action)
{
    auto ve = new VruiViewElement(action);
    ve->m_menuItem = new coButtonMenuItem(action->text());
    add(ve, action);
    return ve;
}

VruiViewElement::VruiViewElement(Element *elem)
: View::ViewElement(elem)
, m_text(elem->text())
{
}

namespace {

void updateSlider(Slider *s, coMenuItem *item, bool moving)
{
    auto vd = dynamic_cast<coPotiMenuItem *>(item);
    auto vs = dynamic_cast<coSliderMenuItem *>(item);
    if (vd)
        s->setValue(vd->getValue());
    if (vs)
        s->setValue(vs->getValue());
    s->setMoving(moving);
    s->trigger();
}

}

void VruiViewElement::menuEvent(coMenuItem *menuItem)
{
    if (auto b = dynamic_cast<Button *>(element))
    {
        auto cb = dynamic_cast<coCheckboxMenuItem *>(menuItem);
        if (cb)
        {
            b->setState(cb->getState());
            b->trigger();
        }
    }
    else if (auto a = dynamic_cast<Action *>(element))
    {
        auto b = dynamic_cast<coButtonMenuItem *>(menuItem);
        if (b)
            a->trigger();
    }
    else if (auto s = dynamic_cast<Slider *>(element))
    {
        updateSlider(s, menuItem, true);
    }
    else if (auto sl = dynamic_cast<SelectionList *>(element))
    {
        auto m = dynamic_cast<coRowMenu *>(m_menu);
        if (dynamic_cast<coCheckboxMenuItem *>(menuItem))
        {
            if (auto smi = dynamic_cast<coSubMenuItem *>(m_menuItem))
                smi->closeSubmenu();
            auto idx = m_menu->index(menuItem);
            sl->select(idx);
            sl->trigger();
        }
    }
}

void VruiViewElement::menuReleaseEvent(coMenuItem *menuItem)
{
    if (auto s = dynamic_cast<Slider *>(element))
    {
        updateSlider(s, menuItem, false);
    }
}

}
}
