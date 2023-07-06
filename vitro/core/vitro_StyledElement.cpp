namespace vitro {

JSClassID StyledElement::jsClassID = 0;

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
    changedStyleProperties.clear();

    for (int i = 0; i < styleProperties.size(); ++i) {
        const auto name{ styleProperties.getName(i) };

        var value{};

        if (auto& local{ localStylesheet.getProperty(name, valueTree) }; !local.isVoid())
            value = local;
        else
            value = context.getStylesheet().getProperty(name, valueTree);

        if (styleProperties.set(name, value)) {
            // Style property has changed - register it
            changedStyleProperties.insert(name);
        }
    }
}

const var& StyledElement::getStyleProperty(const Identifier& name) const
{
    return styleProperties[name];
}

std::pair<bool, const juce::var&> StyledElement::getStylePropertyChanged(const juce::Identifier& name) const
{
    return { isStylePropertyChanged(name), styleProperties[name] };
}

void StyledElement::registerJSPrototype(JSContext* ctx, JSValue prototype)
{
    Element::registerJSPrototype(ctx, prototype);
}

void StyledElement::registerStyleProperty(const juce::Identifier& name, const var& value)
{
    styleProperties.set(name, value);
}

bool StyledElement::isStylePropertyChanged(const juce::Identifier& name) const
{
    const auto it{ changedStyleProperties.find(name) };
    return it != changedStyleProperties.cend();
}

} // namespace vitro
