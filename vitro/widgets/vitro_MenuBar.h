namespace vitro {

class MenuItem;

/** Menu bar.

    This widget corresponds to juce::MenuBar.

*/
class MenuBar : public vitro::ComponentElement,
                public juce::MenuBarComponent
{
public:

    const static juce::Identifier tag;  // <MenuBar>

    static JSClassID jsClassID;

    MenuBar(Context& ctx);
    ~MenuBar();

    JSClassID getJSClassID() const override { return vitro::MenuBar::jsClassID; }

    juce::Component* getComponent() override { return this; }

    void repopulateItems();

protected:

    // vitro::Element
    void update() override;

private:

    class Model : public juce::MenuBarModel
    {
    public:
        Model(vitro::MenuBar& owner);

        juce::StringArray getMenuBarNames() override;
        juce::PopupMenu getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName) override;
        void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

    private:
        vitro::MenuBar& menuBar;
    };

    friend class Model;

    Model model;
};

/** A signel menu item. */
class MenuItem : public vitro::Element
{
public:
    const static juce::Identifier tag;  // <MenuItem>

    static JSClassID jsClassID;

    MenuItem(Context& ctx);

    bool isEnabled() const { return enabled; }
    juce::String getText() const;
    juce::PopupMenu getSubMenu();

    void invokeOnClickHandler();

protected:

    std::shared_ptr<MenuBar> getMenuBar();

    // vitro::Element
    void update() override;

private:
    bool enabled{ true };
};

/** Menu items separator. */
class MenuSeparator : public vitro::Element
{
public:
    const static juce::Identifier tag;  // <MenuSeparator>

    static JSClassID jsClassID;

    MenuSeparator(Context& ctx);

};

} // namespace vitro
