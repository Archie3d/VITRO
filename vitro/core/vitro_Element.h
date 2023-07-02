namespace vitro {

/** UI element base class

    This is a base class for all the elements that compose the UI.
    An element is identified by its tag, and it has a set of properties
    (attributes) stored as a ValueTree.

    Element is a part of the UI tree, but it does not necessarily correspond
    to a layout or a visual component. For this the Element class is further
    specialised into LayoutElement and ComponentElement.

    @see StyledElement
    @see LayoutElement
    @see ComponentElement
*/
class Element : private juce::ValueTree::Listener
{
public:

    Element();
    Element(const juce::Identifier& tag);
    virtual ~Element();

    virtual bool isStyledElement() const { return false; }
    virtual bool isLayoutElement() const { return false; }
    virtual bool isComponentElement() const { return false; }

    juce::Identifier getTag() const;

    Element* getParentElement() const;
    Element* getTopLevelElement();
    void addChildElement(Element* element);
    void removeChildElement(Element* element, bool deleteObject);
    void removeAllChildElements();

private:

    juce::ValueTree valueTree;

    Element* parent{};

    juce::OwnedArray<Element> children{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Element)
};

} // namespace vitro
