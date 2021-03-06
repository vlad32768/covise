#include "Group.h"

#include <algorithm>
#include <cassert>

namespace opencover {
namespace ui {

Group::Group(const std::string &name, Owner *owner)
: Element(name, owner)
{
}

Group::Group(Group *parent, const std::string &name)
: Element(parent, name)
{
}

bool Group::add(Element *elem)
{
    if (Container::add(elem))
    {
        if (elem->parent() != this)
        {
            if (elem->parent())
                elem->parent()->remove(elem);
            assert(!elem->m_parent);
        }
        elem->m_parent = this;
        return true;
    }
    return false;
}

bool Group::remove(Element *elem)
{
    if (Container::remove(elem))
    {
        elem->m_parent = nullptr;
        return true;
    }
    return false;
}

}
}

