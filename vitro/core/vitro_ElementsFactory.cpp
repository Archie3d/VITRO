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
    registerElement<vitro::View>();
}

std::unique_ptr<Element> ElementsFactory::createElement(const Identifier& tag)
{
    auto it{ creators.find(tag) };

    if (it != creators.end()) {
        if (auto elem{ it->second() })
            return elem;

        return nullptr;
    }

    // Creating a default component element with unknown tag
    if (auto elem{ std::make_unique<vitro::Component>(tag, context) })
        return elem;

    return nullptr;
}

} // namespace vitro
