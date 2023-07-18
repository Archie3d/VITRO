namespace vitro {

JSClassID ComboBox::jsClassID = 0;

const Identifier ComboBox::tag("ComboBox");
const Identifier ComboBox::tagHeading("heading");
const Identifier ComboBox::tagSeparator("separator");
const Identifier ComboBox::tagItem("item");

ComboBox::ComboBox(Context& ctx)
    : ComponentElement(ComboBox::tag, ctx)
{
}

void ComboBox::update()
{
    ComponentElement::update();

    if (itemsUpdatePending) {
        populateItems();
        itemsUpdatePending = false;
    }
}

void ComboBox::populateItems()
{
    juce::ComboBox::clear(juce::dontSendNotification);

    forEachChild([this](const Element::Ptr& elem) {
        const auto tag{ elem->getTag() };

        if (tag == tagHeading) {
            if (const auto& val{ elem->getAttribute(attr::text) }; !val.isVoid())
                addSectionHeading(val.toString());
        } else if (tag == tagSeparator) {
            addSeparator();
        } else if (tag == tagItem) {
            const auto text{ elem->getAttribute(attr::text).toString() };
            const int itemId{ elem->getAttribute(attr::itemid) };

            if (itemId > 0 && text.isNotEmpty())
                addItem(text, itemId);
        }
        // Ignore other elements
    }, false);
}

void ComboBox::comboBoxChanged(juce::ComboBox*)
{

}

void ComboBox::valueTreeChildAdded(juce::ValueTree&, juce::ValueTree&)
{
    itemsUpdatePending = true;
}

void ComboBox::valueTreeChildRemoved(juce::ValueTree&, juce::ValueTree&, int)
{
    itemsUpdatePending = true;
}

} // namespace vitro
