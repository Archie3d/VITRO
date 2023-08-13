namespace vitro {

JSClassID MenuBar::jsClassID = 0;

const Identifier MenuBar::tag("MenuBar");

//==============================================================================

MenuBar::Model::Model(vitro::MenuBar& owner)
    : menuBar{ owner }
{
}

juce::StringArray MenuBar::Model::getMenuBarNames()
{
    StringArray items{};

    for (auto& child : menuBar.children) {
        if (auto item{ std::dynamic_pointer_cast<MenuItem>(child) }) {
            items.add(item->getText());
        }
    }

    return items;
}

juce::PopupMenu MenuBar::Model::getMenuForIndex(int topLevelMenuIndex, const String& menuName)
{
    int index = 0;

    for (auto& child : menuBar.children) {
        if (index == topLevelMenuIndex) {
            if (auto item{ std::dynamic_pointer_cast<MenuItem>(child) }) {
                return item->getSubMenu();
            }
        }

        ++index;
    }

    juce::PopupMenu menu{};
    return menu;
}

void MenuBar::Model::menuItemSelected([[maybe_unused]] int menuItemID, [[maybe_unused]] int topLevelMenuIndex)
{
}

//==============================================================================

MenuBar::MenuBar(Context& ctx)
    : ComponentElement(MenuBar::tag, ctx),
      model(*this)
{
    setModel(&model);
}

MenuBar::~MenuBar()
{
    setModel(nullptr);
}

void MenuBar::repopulateItems()
{
    model.menuItemsChanged();
}

void MenuBar::update()
{
    LayoutElement::update();
    model.menuItemsChanged();
}

//==============================================================================

JSClassID MenuItem::jsClassID = 0;

const Identifier MenuItem::tag("MenuItem");


MenuItem::MenuItem(Context& ctx)
    : Element(MenuItem::tag, ctx)
{
}

String MenuItem::getText() const
{
    return getAttribute(attr::text).toString();
}

juce::PopupMenu MenuItem::getSubMenu()
{
    juce::PopupMenu menu{};

    for (auto& child : children) {
        if (auto item{ std::dynamic_pointer_cast<MenuItem>(child) }) {
            auto subMenu{ item->getSubMenu() };

            if (subMenu.getNumItems() > 0) {
                menu.addSubMenu(item->getText(), item->getSubMenu(), item->isEnabled());
            } else {
                PopupMenu::Item popUpItem(item->getText());
                popUpItem.setEnabled(item->isEnabled());
                popUpItem.setAction([elem = std::weak_ptr<MenuItem>(item)]() {
                    if (auto menuItem{ elem.lock() }) {
                        menuItem->invokeOnClickHandler();
                    }
                });
                menu.addItem(popUpItem);
            }

        } else if (auto sep{ std::dynamic_pointer_cast<MenuSeparator>(child) }) {
            menu.addSeparator();
        }
    }

    return menu;
}

void MenuItem::invokeOnClickHandler()
{
    evaluateAttributeScript(attr::onclick);
}

std::shared_ptr<MenuBar> MenuItem::getMenuBar()
{
    if (auto parent{ getParentElement() }) {
        if (auto parentMenuBar{ std::dynamic_pointer_cast<MenuBar>(parent) })
            return parentMenuBar;
    
        if (auto parentMenuItem{ std::dynamic_pointer_cast<MenuItem>(parent) })
            return parentMenuItem->getMenuBar();

    }

    return nullptr;
}

void MenuItem::update()
{
    Element::update();

    if (auto&& [changed, val]{ getAttributeChanged(attr::enabled) }; changed)
        enabled = val.isVoid() ? true : (bool)val;

    if (auto menuBar{ getMenuBar() })
        menuBar->repopulateItems();
}

//==============================================================================

JSClassID MenuSeparator::jsClassID = 0;

const Identifier MenuSeparator::tag("MenuSeparator");


MenuSeparator::MenuSeparator(Context& ctx)
    : Element(MenuSeparator::tag, ctx)
{
}

} // namespace vitro
