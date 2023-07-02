namespace vitro {

StyledElement::StyledElement(const Identifier& tag, Context& ctx)
    : Element(tag, ctx)
{
}

void StyledElement::setStyleAttribute(const var& value)
{
    localStylesheet.clear();
    localStylesheet.populateFromVar(value);
}

void StyledElement::updateStyleProperties()
{
    for (int i = 0; i < styleProperties.size(); ++i) {
        const auto name{ styleProperties.getName(i) };

        if (auto& local{ localStylesheet.getProperty(name, valueTree) }; !local.isVoid())
            styleProperties.set(name, local);
        else
            styleProperties.set(name, context.getStylesheet().getProperty(name, valueTree));
    }
}

const var& StyledElement::getStyleProperty(const Identifier& name) const
{
    return styleProperties[name];
}

void StyledElement::registerStyleProperty(const juce::Identifier& name, const var& value)
{
    styleProperties.set(name, value);
}



} // namespace vitro
