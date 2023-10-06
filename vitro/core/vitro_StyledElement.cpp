namespace vitro {

JSClassID StyledElement::jsClassID = 0;

StyledElement::StyledElement(const Identifier& elementTag, Context& ctx)
    : Element(elementTag, ctx)
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

    //DBG("Style for <" << getTag() << ">");

    for (int i = 0; i < styleProperties.size(); ++i) {
        const auto name{ styleProperties.getName(i) };

        var value{};

        if (auto& local{ localStylesheet.getProperty(name, valueTree) }; !local.isVoid())
            value = local;
        else if (auto& global{ context.getStylesheet().getProperty(name, valueTree) }; !global.isVoid())
            value = global;
        else
            value = defaultStyleProperties[name];

        //if (!value.isVoid()) {
        //    DBG("    " << name << ": " << value.toString());
        //}

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

void StyledElement::reconcileElement()
{
    Element::reconcileElement();
}

void StyledElement::registerStyleProperty(const juce::Identifier& name, const var& value)
{
    styleProperties.set(name, value);

    if (!value.isVoid())
        defaultStyleProperties.set(name, value);
}

bool StyledElement::isStylePropertyChanged(const juce::Identifier& name) const
{
    const auto it{ changedStyleProperties.find(name) };
    return it != changedStyleProperties.cend();
}

void StyledElement::setDefaultStyleProperty(const juce::Identifier& name, const juce::var value)
{
    defaultStyleProperties.set(name, value);
}

} // namespace vitro
