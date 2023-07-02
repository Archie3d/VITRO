#include "yoga/YGNode.h"

namespace vitro {

struct LayoutElement::Layout
{
    LayoutElement& self;
    std::unique_ptr<YGNode, void(*)(const YGNodeRef)> nodePtr;
    YGNodeRef node{};

    Layout(LayoutElement& element)
        : self{ element },
          nodePtr(YGNodeNew(), YGNodeFree),
          node{ nodePtr.get() }
    {
        YGNodeSetContext(node, &element);
    }

};

//==============================================================================

LayoutElement::LayoutElement(const Identifier& tag, Context& ctx)
    : StyledElement(tag, ctx),
      layout{ std::make_unique<Layout>(*this) }
{
    registerStyleProperty("width");
    registerStyleProperty("height");
}

juce::Rectangle<float> LayoutElement::getLayoutElementBounds() const
{
    return {
        YGNodeLayoutGetLeft   (layout->node),
        YGNodeLayoutGetTop    (layout->node),
        YGNodeLayoutGetWidth  (layout->node),
        YGNodeLayoutGetHeight (layout->node)
    };
}

void LayoutElement::numberOfChildrenChanged()
{
    StyledElement::numberOfChildrenChanged();

    childrenChanged = true;
}

} // namespace vitro
