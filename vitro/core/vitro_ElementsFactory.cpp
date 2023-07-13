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
}

Element::Ptr ElementsFactory::createElement(const Identifier& tag)
{
    auto it{ creators.find(tag) };

    Element::Ptr element{};

    if (it != creators.end())
        element = it->second();

    if (element == nullptr) {
        // Creating a default element with unknown tag
        element = std::make_shared<vitro::Element>(tag, context);
    }

    if (element != nullptr)
        element->initialize();

    return element;
}

void ElementsFactory::stashElement(const Element::Ptr& element)
{
    stashedElements.push_back(element);
}

void ElementsFactory::removeStashedElement(const Element::Ptr& element)
{
    stashedElements.erase(std::remove(stashedElements.begin(), stashedElements.end(), element), stashedElements.end());
}

} // namespace vitro
