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
        const auto style{ context.getLoader().loadText(val.toString()) };

        if (style.isNotEmpty())
            populate(style);
    }
}

void Style::forwardXmlElement(const juce::XmlElement& xml)
{
    const auto style{ xml.getAllSubText().trim() };

    if (style.isNotEmpty())
        populate(style);
}

void Style::populate(const String& style)
{
    const size_t hash{ style.hash() };
    const bool shouldPopulate{ style.isNotEmpty() && (!populatedOnce || hash != styleHash) };

    if (shouldPopulate) {
        context.getStylesheet().populateFromString(style);
        populatedOnce = true;
        styleHash = hash;
    }
}


} // namespace vitro
