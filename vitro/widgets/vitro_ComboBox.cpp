namespace vitro {

JSClassID ComboBox::jsClassID = 0;

const Identifier ComboBox::tag("ComboBox");
const Identifier ComboBox::tagHeading("heading");
const Identifier ComboBox::tagSeparator("separator");
const Identifier ComboBox::tagItem("item");

ComboBox::ComboBox(Context& ctx)
    : ComponentElement(ComboBox::tag, ctx)
{
    registerStyleProperty(attr::css::color);
    registerStyleProperty(attr::css::text_color);
    registerStyleProperty(attr::css::button_color);
    registerStyleProperty(attr::css::arrow_color);
    registerStyleProperty(attr::css::popup_color);
    registerStyleProperty(attr::css::highlight_color);
    registerStyleProperty(attr::css::highlight_text_color);
    registerStyleProperty(attr::css::text_align);
}

void ComboBox::update()
{
    ComponentElement::update();

    if (itemsUpdatePending) {
        populateItems();
        itemsUpdatePending = false;
    }

    setColourFromStyleProperty(juce::ComboBox::textColourId,       attr::css::color);
    setColourFromStyleProperty(juce::ComboBox::backgroundColourId, attr::css::background_color);
    setColourFromStyleProperty(juce::ComboBox::outlineColourId,    attr::css::border_color);
    setColourFromStyleProperty(juce::ComboBox::buttonColourId,     attr::css::button_color);
    setColourFromStyleProperty(juce::ComboBox::arrowColourId,      attr::css::arrow_color);

    if (auto&& [changed, prop]{ getStylePropertyChanged(attr::css::text_align) }; changed) {
        juce::ComboBox::setJustificationType(prop.isVoid() ? Justification::left
                                                           : parseJustificationFromString(prop.toString()));
    }

    // @todo This will probably affect the global popup window colour,
    //       so we'll need to find another way.
    if (auto&& [changed, val]{ getStylePropertyChanged(attr::css::text_color) }; changed && !val.isVoid())
        getLookAndFeel().setColour(juce::PopupMenu::textColourId, parseColourFromString(val.toString()));

    if (auto&& [changed, val]{ getStylePropertyChanged(attr::css::popup_color) }; changed && !val.isVoid())
        getLookAndFeel().setColour(juce::PopupMenu::backgroundColourId, parseColourFromString(val.toString()));

    if (auto&& [changed, val]{ getStylePropertyChanged(attr::css::highlight_text_color) }; changed && !val.isVoid())
        getLookAndFeel().setColour(juce::PopupMenu::highlightedTextColourId, parseColourFromString(val.toString()));

    if (auto&& [changed, val]{ getStylePropertyChanged (attr::css::highlight_color) }; changed && !val.isVoid())
        getLookAndFeel().setColour(juce::PopupMenu::highlightedBackgroundColourId, parseColourFromString(val.toString()));


    if (auto&& [changed, val]{ getAttributeChanged(attr::emptytext) }; changed)
        setTextWhenNoChoicesAvailable(val.toString());

    if (auto&& [changed, val]{ getAttributeChanged(attr::noselectiontext) }; changed)
        setTextWhenNothingSelected(val.toString());

    if (auto&& [changed, val]{ getAttributeChanged(attr::selectedid) }; changed && !val.isVoid()) {
        const int newSelectedId{ val };

        if (getSelectedId() != newSelectedId)
            setSelectedId(newSelectedId);
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
    setAttribute(attr::selectedid, getSelectedId(), false);
    setAttribute(attr::text, getText(), false);

    evaluateAttributeScript(attr::onchange);
}

void ComboBox::valueTreeChildAdded(juce::ValueTree& tree, juce::ValueTree& child)
{
    ComponentElement::valueTreeChildAdded(tree, child);
    itemsUpdatePending = true;
}

void ComboBox::valueTreeChildRemoved(juce::ValueTree& tree, juce::ValueTree& child, int idx)
{
    ComponentElement::valueTreeChildRemoved(tree, child, idx);
    itemsUpdatePending = true;
}

} // namespace vitro
