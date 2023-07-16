namespace vitro {

ElementsFactory::ElementsFactory(Context& ctx)
    : context{ ctx }
{
}

void ElementsFactory::reset()
{
    creators.clear();
}

void ElementsFactory::registerDefaultElements()
{
    registerElement<vitro::Script>();
    registerElement<vitro::View>();
    registerElement<vitro::Panel>();
    registerElement<vitro::Label>();
    registerElement<vitro::TextButton>();
    registerElement<vitro::ToggleButton>();
    registerElement<vitro::DrawableButton>();
    registerElement<vitro::Slider>();
    registerElement<vitro::TextEditor>();
    registerElement<vitro::ScrollArea>();
}

Element::Ptr ElementsFactory::createElement(const Identifier& tag)
{
    auto it{ creators.find(tag) };

    Element::Ptr element{};

    if (it != creators.end())
        element = it->second();

    if (element == nullptr) {
        const auto& tagStr{ tag.toString() };

        if (tagStr.isNotEmpty()) {
            const auto firstChar{ tagStr.substring(0, 1) };
            if (firstChar == firstChar.toUpperCase()) {
                // Create a panel
                element = std::make_shared<vitro::Panel>(tag, context);
            } else {
                // Creating a default element with unknown tag
                element = std::make_shared<vitro::Element>(tag, context);
            }
        }
    }

    if (element != nullptr)
        element->initialize();

    return element;
}

void ElementsFactory::stashElement(const Element::Ptr& element)
{
    if (std::find(stashedElements.begin(), stashedElements.end(), element) == stashedElements.end()) {
        stashedElements.push_back(element);
    }
}

void ElementsFactory::removeStashedElement(const Element::Ptr& element)
{
    stashedElements.erase(std::remove(stashedElements.begin(), stashedElements.end(), element), stashedElements.end());
}

void ElementsFactory::clearStashedElements()
{
    stashedElements.clear();
}

void ElementsFactory::clearUnreferencedStashedElements()
{
    auto it{ stashedElements.begin() };

    while (it != stashedElements.end()) {
        auto& ptr{ *it };

        if (ptr.use_count() == 1 && ptr->getJSValueRefCount() == 1) {
            it = stashedElements.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace vitro
