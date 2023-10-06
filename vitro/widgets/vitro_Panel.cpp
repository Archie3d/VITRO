namespace vitro {

JSClassID Panel::jsClassID = 0;

const Identifier Panel::tag("Panel");

Panel::Panel(Context& ctx)
    : Panel(Panel::tag, ctx)
{
}

Panel::Panel(const Identifier& elementTag, Context& ctx)
    : ComponentElementWithBackground(elementTag, ctx),
      juce::Component()
{
}

void Panel::resized()
{
    updateGradientToComponentSize();
}

void Panel::paint(Graphics& g)
{
    paintBackground(g);
}

void Panel::update()
{
    ComponentElementWithBackground::update();
}

} // namespace vitro
