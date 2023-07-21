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
        const auto script{ context.getLoader().loadText(val.toString()) };
        evaluate(script);
    }
}

void Script::forwardXmlElement(const juce::XmlElement& xml)
{
    const auto script{ xml.getAllSubText().trim() };

    if (script.isNotEmpty())
        evaluate(script);
}

void Script::evaluate(const juce::String& script)
{
    const size_t hash{ script.hash() };
    const bool shouldEvaluate{ script.isNotEmpty() && (!evaluatedOnce || hash != scriptHash) };

    if (shouldEvaluate) {
        context.eval(script);
        evaluatedOnce = true;
        scriptHash = hash;
    }
}

} // namespace vitro
