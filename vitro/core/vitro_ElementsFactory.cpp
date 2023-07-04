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

std::unique_ptr<Element> ElementsFactory::createElement (const Identifier& tag)
{
    auto it{ creators.find(tag) };

    if (it != creators.end()) {
        if (auto elem{ it->second() })
            return elem;

        return nullptr;
    }

    // @todo Creating a default comonent element with unknown tag

    //if (auto elem{ std::make_unique<vitro::widget::Component>(tag, context) })
    //    return elem;

    return nullptr;
}


} // namespace vitro
