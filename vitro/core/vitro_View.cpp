namespace vitro {

const Identifier View::tag("View");

View::View(Context& ctx)
    : ComponentElement(View::tag, ctx)
{
}

View::~View()
{
    // We must remove all the children in order for them to be notified
    // that they are about to be deleted. This is important for some
    // element to release resources and perform cleaning up.
    removeAllChildElements();
}

void View::resized()
{
    recalculateLayoutToCurrentBounds();
}

void View::update()
{
    triggerAsyncUpdate();
}

void View::handleAsyncUpdate()
{
    updateEverything();
}

void View::updateEverything()
{
    if (updateLayout())
        recalculateLayoutToCurrentBounds();

    updateChildren();

    repaint();
}

void View::recalculateLayoutToCurrentBounds()
{
    recalculateLayout(static_cast<float>(getWidth()), static_cast<float>(getHeight()));
}

} // namespace vitro
