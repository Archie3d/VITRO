namespace vitro {

JSClassID Script::jsClassID = 0;

const Identifier Script::tag("script");

Script::Script(Context& ctx)
    : Element(Script::tag, ctx)
{
}

void Script::update()
{
    Element::update();

    if (const auto&& [changed, val]{ getAttributeChanged(attr::src) }; changed) {
        const auto script = context.getLoader().loadText(val.toString());
        context.eval(script);
    }
}

void Script::forwardXmlElement(const juce::XmlElement& xml)
{
    const auto script{ xml.getAllSubText().trim() };
    context.eval(script);
}

} // namespace vitro
