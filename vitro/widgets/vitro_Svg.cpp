namespace vitro {

JSClassID Svg::jsClassID = 0;

const Identifier Svg::tag("Svg");

Svg::Svg(Context& ctx)
    : ComponentElementWithBackground(Svg::tag, ctx)
{
}

void Svg::forwardXmlElement(const juce::XmlElement& xml)
{
    // xml here may represent this <Svg> element, we have to find the first <svg> child
    if (xml.getTagName() == "svg") {
        populateFromXml(xml);
    } else if (auto* svgXml{ xml.getChildByName("svg") }) {
        populateFromXml(*svgXml);
    }
}

void Svg::paint(juce::Graphics& g)
{
    ComponentElementWithBackground::paintBackground(g);

    if (drawable != nullptr)
        drawable->draw(g, 1.0f, scaleTransform);
}

void Svg::resized()
{
    ComponentElementWithBackground::updateGradientToComponentSize();

    if (drawable != nullptr)
        drawable->setBounds(getLocalBounds());

    updateScaleTransform();
}

void Svg::update()
{
    ComponentElementWithBackground::update();
    
    if (const auto&& [changed, val]{ getAttributeChanged(attr::src) }; changed)
        populateFromXmlResource(val.toString());

    if (const auto&& [changed, val]{ getAttributeChanged(attr::scale) }; changed)
        scale = (float)val;
}

void Svg::populateFromXml(const juce::XmlElement& svg)
{
    drawable = Drawable::createFromSVG(svg);
}

void Svg::populateFromXmlResource(const String& location)
{
    if (auto xml{ context.getLoader().loadXML(location) })
        populateFromXml(*xml);
    else
        drawable.reset();
}

void Svg::updateScaleTransform()
{
    // Fit the SVG into the component's bounds
    const auto bounds{ drawable->getDrawableBounds() };
    const float scaleX{ bounds.getWidth() > 0 ? getWidth() / bounds.getWidth() : 1.0f };
    const float scaleY{ bounds.getHeight() > 0 ? getHeight() / bounds.getHeight() : 1.0f };
    scaleTransform = AffineTransform::scale(scaleX *  scale, scaleY * scale);
}

} // namespace vitro
