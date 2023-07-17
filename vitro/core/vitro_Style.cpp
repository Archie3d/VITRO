namespace vitro {

JSClassID Style::jsClassID = 0;

const Identifier Style::tag("style");

Style::Style(Context& ctx)
    : Element(Style::tag, ctx)
{
}

void Style::update()
{
    Element::update();

    if (const auto&& [changed, val]{ getAttributeChanged(attr::src) }; changed) {
        const auto style = context.getLoader().loadText(val.toString());
        context.getStylesheet().populateFromString(style);
    }
}

void Style::forwardXmlElement(const juce::XmlElement& xml)
{
    const auto style{ xml.getAllSubText().trim() };
    context.getStylesheet().populateFromString(style);
}

} // namespace vitro
