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
    // We must register a generic element since it
    // will be created on an unknown tag
    registerElement<vitro::Element>();

    registerElement<vitro::Script>();
    registerElement<vitro::Style>();
    registerElement<vitro::View>();
    registerElement<vitro::Panel>();
    registerElement<vitro::Label>();
    registerElement<vitro::TextButton>();
    registerElement<vitro::ToggleButton>();
    registerElement<vitro::DrawableButton>();
    registerElement<vitro::Slider>();
    registerElement<vitro::Svg>();
    registerElement<vitro::TextEditor>();
    registerElement<vitro::ScrollArea>();
    registerElement<vitro::ComboBox>();
    registerElement<vitro::MenuBar>();
    registerElement<vitro::MenuItem>();
    registerElement<vitro::MenuSeparator>();
    registerElement<vitro::CodeEditor>();
    registerElement<vitro::OpenGLView>();
    registerElement<vitro::MidiKeyboard>();
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

    if (element != nullptr) {

        // Inject look and feel into the component element.
        if (element->isComponentElement()) {
            if (auto compElem{ std::dynamic_pointer_cast<ComponentElement>(element) }) {
                if (auto* comp{ compElem->getComponent() })
                    comp->setLookAndFeel(&context.getLookAndFeel());
            }
        }

        element->initialize();
    }

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
