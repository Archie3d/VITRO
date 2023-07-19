namespace vitro {

/** Combo box element.

    This element is equivalent to juce::ComboBox.

    The items of the ComboBox are defined via the child <item> elements.
    Additional child elements are <heading> and <separator>.

    Attributes:
        emptytext
        noselectiontext
        selectedid
        onchange

    Style properties:
        color
        text-color
        button-color
        arrow-color
        popup-color
        highlight-color
        highlight-text-color
        text-align
*/
class ComboBox : public vitro::ComponentElement,
                 public juce::ComboBox,
                 private juce::ComboBox::Listener
{
public:
    const static juce::Identifier tag;          // <ComboBox>
    const static juce::Identifier tagHeading;   // <heading>
    const static juce::Identifier tagSeparator; // <separator>
    const static juce::Identifier tagItem;      // <item>

    static JSClassID jsClassID;

    ComboBox(Context& ctx);

    JSClassID getJSClassID() const override { return vitro::ComboBox::jsClassID; }

    juce::Component* getComponent() override { return this; }

protected:

    // vitro::Element
    void update() override;

private:

    void populateItems();

    // juce::ComboBox::Listener
    void comboBoxChanged(juce::ComboBox*) override;

    // juce::ValueTree::Listener
    void valueTreeChildAdded(juce::ValueTree&, juce::ValueTree&) override;
    void valueTreeChildRemoved(juce::ValueTree&, juce::ValueTree&, int) override;

    bool itemsUpdatePending{};
};

} // namespace vitro
