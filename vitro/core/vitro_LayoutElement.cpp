#include "yoga/YGNode.h"

namespace vitro {

namespace yoga {

// Symbolic properties
const static Identifier align_items     ("align-items");
const static Identifier align_content   ("align-content");
const static Identifier align_self      ("align-self");
const static Identifier direction       ("direction");
const static Identifier flex_direction  ("flex-direction");
const static Identifier justify_content ("justify-content");
const static Identifier display         ("display");
const static Identifier overflow        ("overflow");
const static Identifier position        ("position");
const static Identifier flex_wrap       ("flex-wrap");

// Numeric properties
const static Identifier flex         ("flex");
const static Identifier flex_grow    ("flex-grow");
const static Identifier flex_shrink  ("flex-shrink");
const static Identifier flex_basis   ("flex-basis");
const static Identifier width        ("width");
const static Identifier height       ("height");
const static Identifier min_width    ("min-width");
const static Identifier min_height   ("min-height");
const static Identifier max_width    ("max-width");
const static Identifier max_height   ("max-height");
const static Identifier aspect_ratio ("aspect-ratio");

static const String margin_  ("margin-");
static const String padding_ ("padding-");
static const String border_  ("border-");

/* Yoga layout enums to string translation table */

const static std::map<String, YGAlign> alignValues {
    { YGAlignToString(YGAlignAuto),         YGAlignAuto         },
    { YGAlignToString(YGAlignFlexStart),    YGAlignFlexStart    },
    { YGAlignToString(YGAlignCenter),       YGAlignCenter       },
    { YGAlignToString(YGAlignFlexEnd),      YGAlignFlexEnd      },
    { YGAlignToString(YGAlignStretch),      YGAlignStretch      },
    { YGAlignToString(YGAlignBaseline),     YGAlignBaseline     },
    { YGAlignToString(YGAlignSpaceBetween), YGAlignSpaceBetween },
    { YGAlignToString(YGAlignSpaceAround),  YGAlignSpaceAround  }
};

const static std::map<String, YGDimension> dimensionValues {
    { YGDimensionToString(YGDimensionWidth),  YGDimensionWidth  },
    { YGDimensionToString(YGDimensionHeight), YGDimensionHeight }
};

const static std::map<String, YGDirection> directionValues {
    { YGDirectionToString(YGDirectionInherit), YGDirectionInherit },
    { YGDirectionToString(YGDirectionLTR),     YGDirectionLTR     },
    { YGDirectionToString(YGDirectionRTL),     YGDirectionRTL     }
};

const static std::map<String, YGDisplay> displayValues {
    { YGDisplayToString(YGDisplayFlex), YGDisplayFlex },
    { YGDisplayToString(YGDisplayNone), YGDisplayNone }
};

const static std::map<String, YGEdge> edgeValues {
    { YGEdgeToString(YGEdgeLeft),       YGEdgeLeft       },
    { YGEdgeToString(YGEdgeTop),        YGEdgeTop        },
    { YGEdgeToString(YGEdgeRight),      YGEdgeRight      },
    { YGEdgeToString(YGEdgeBottom),     YGEdgeBottom     },
    { YGEdgeToString(YGEdgeStart),      YGEdgeStart      },
    { YGEdgeToString(YGEdgeEnd),        YGEdgeEnd        },
    { YGEdgeToString(YGEdgeHorizontal), YGEdgeHorizontal },
    { YGEdgeToString(YGEdgeVertical),   YGEdgeVertical   },
    { YGEdgeToString(YGEdgeAll),        YGEdgeAll        }
};

const static std::map<String, YGFlexDirection> flexDirectionValues {
    { YGFlexDirectionToString(YGFlexDirectionColumn),        YGFlexDirectionColumn        },
    { YGFlexDirectionToString(YGFlexDirectionColumnReverse), YGFlexDirectionColumnReverse },
    { YGFlexDirectionToString(YGFlexDirectionRow),           YGFlexDirectionRow           },
    { YGFlexDirectionToString(YGFlexDirectionRowReverse),    YGFlexDirectionRowReverse    }
};

const static std::map<String, YGJustify> justifyValues {
    { YGJustifyToString(YGJustifyFlexStart),    YGJustifyFlexStart    },
    { YGJustifyToString(YGJustifyCenter),       YGJustifyCenter       },
    { YGJustifyToString(YGJustifyFlexEnd),      YGJustifyFlexEnd      },
    { YGJustifyToString(YGJustifySpaceBetween), YGJustifySpaceBetween },
    { YGJustifyToString(YGJustifySpaceAround),  YGJustifySpaceAround  },
    { YGJustifyToString(YGJustifySpaceEvenly),  YGJustifySpaceEvenly  }
};

const static std::map<String, YGOverflow> overflowValues {
    { YGOverflowToString(YGOverflowVisible), YGOverflowVisible },
    { YGOverflowToString(YGOverflowHidden),  YGOverflowHidden  },
    { YGOverflowToString(YGOverflowScroll),  YGOverflowScroll  }
};

const static std::map<String, YGPositionType> positionTypeValues {
    { YGPositionTypeToString(YGPositionTypeStatic),   YGPositionTypeStatic   },
    { YGPositionTypeToString(YGPositionTypeRelative), YGPositionTypeRelative },
    { YGPositionTypeToString(YGPositionTypeAbsolute), YGPositionTypeAbsolute }
};

const static std::map<String, YGWrap> wrapValues {
    { YGWrapToString(YGWrapNoWrap),      YGWrapNoWrap },
    { YGWrapToString(YGWrapWrap),        YGWrapWrap },
    { YGWrapToString(YGWrapWrapReverse), YGWrapWrapReverse }
};

} // namespace yoga

//==============================================================================

/** Helper to make sure that Yoga's default configuration
    gets properly deleted. Otherwise a memmory leak gets reported
    on applicatiom termination.
*/
struct DefaultConfigDeleter
{
    ~DefaultConfigDeleter()
    {
        if (auto* ref{ YGConfigGetDefault() })
            YGConfigFree(ref);
    }
};

const static DefaultConfigDeleter defaultConfigDeleter;

//==============================================================================

struct LayoutElement::Layout final
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

    ~Layout()
    {
        if (auto* owner{ node->getOwner() })
            YGNodeRemoveChild(owner, node);
    }

    template <typename T>
    bool assignProperty(const Identifier& name,
                        const std::map<String, T>& values,
                        void(*assign)(YGNodeRef, T),
                        T(*getter)(YGNodeConstRef))
    {
        const auto& val{ self.getStyleProperty(name) };

        if (!val.isVoid()) {
            const auto it{ values.find(val.toString()) };

            if (it != values.cend()) {
                const auto currentValue{ getter(node) };

                if (currentValue != it->second) {
                    assign(node, it->second);
                    return true;
                }
            }
        }

        return false;
    }

    bool assignFloatProperty(const Identifier& name,
                             void(*assign)(YGNodeRef, float),
                             float(*getter)(YGNodeConstRef))
    {
        const auto& val{ self.getStyleProperty(name) };

        if (!val.isVoid()) {
            const float floatValue{ val };

            if (floatValue != getter(node)) {
                assign(node, floatValue);
                return true;
            }
        }

        return false;
    }

    bool assignFloatProperty(const Identifier& name,
                             void(*assign)(YGNodeRef, float),
                             void(*assignPercent)(YGNodeRef, float),
                             YGValue(*getter)(YGNodeConstRef))
    {
        const auto& val{ self.getStyleProperty(name) };

        if (!val.isVoid()) {
            const float floatValue{ val };
            const auto currentValue{ getter(node) };

            if (val.toString().endsWithChar('%')) {
                if (currentValue.unit != YGUnitPercent || currentValue.value != floatValue) {
                    assignPercent(node, floatValue);
                    return true;
                }
            } else {
                if (currentValue.unit == YGUnitPercent || currentValue.value != floatValue) {
                    assign(node, floatValue);
                    return true;
                }
            }
        }

        return false;
    }

    bool assignEdgeFloatProperty(void(*assign)(YGNodeRef, YGEdge, float),
                                 void(*assignPercent)(YGNodeRef, YGEdge, float),
                                 YGValue(*getter)(YGNodeConstRef, YGEdge))
    {
        bool changed{ false };

        for (const auto& [edgeName, edgeEnum] : yoga::edgeValues) {
            const auto& val{ self.getStyleProperty(edgeName) };

            if (!val.isVoid()) {
                const float floatValue{ val };
                const auto currentValue{ getter(node, edgeEnum) };

                if (val.toString().endsWithChar('%')) {
                    if (currentValue.unit != YGUnitPercent || currentValue.value != floatValue) {
                        assignPercent(node, edgeEnum, floatValue);
                        changed = true;
                    }
                } else {
                    if (currentValue.unit == YGUnitPercent || currentValue.value != floatValue) {
                        assign(node, edgeEnum, floatValue);
                        changed = true;
                    }
                }
            }
        }

        return changed;
    }

    bool assignEdgeFloatPropertyWithPrefix(const String& prefix,
                                           void(*assign)(YGNodeRef, YGEdge, float),
                                           float(*getter)(YGNodeConstRef, YGEdge))
    {
        bool changed{ false };

        for (const auto& [edgeName, edgeEnum] : yoga::edgeValues) {
            const auto propertyName{ prefix + edgeName };
            const auto& val{ self.getStyleProperty (propertyName) };

            if (!val.isVoid()) {
                const float floatValue{ val };
                const float currentValue{ getter(node, edgeEnum) };

                if (floatValue != currentValue) {
                    assign(node, edgeEnum, floatValue);
                    changed = true;
                }
            }
        }

        return changed;
    }

    bool assignEdgeFloatPropertyWithPrefix(const String& prefix,
                                           void(*assign)(YGNodeRef, YGEdge, float),
                                           void(*assignPercent)(YGNodeRef, YGEdge, float),
                                           YGValue(*getter)(YGNodeConstRef, YGEdge))
    {
        bool changed{ false };

        for (const auto& [edgeName, edgeEnum] : yoga::edgeValues) {
            const auto propertyName{ prefix + edgeName };
            const auto& val{ self.getStyleProperty(propertyName) };

            if (!val.isVoid()) {
                const float floatValue{ val };
                const auto currentValue{ getter(node, edgeEnum) };

                if (val.toString().endsWithChar('%')) {
                    if (currentValue.unit != YGUnitPercent || currentValue.value != floatValue) {
                        assignPercent(node, edgeEnum, floatValue);
                        changed = true;
                    }
                } else {
                    if (currentValue.unit == YGUnitPercent || currentValue.value != floatValue) {
                        assign(node, edgeEnum, floatValue);
                        changed = true;
                    }
                }
            }
        }

        return changed;
    }

    bool rebuild()
    {
        bool changed{ false };

        changed = assignProperty(yoga::direction,       yoga::directionValues,     YGNodeStyleSetDirection,      YGNodeStyleGetDirection)      || changed;
        changed = assignProperty(yoga::flex_direction,  yoga::flexDirectionValues, YGNodeStyleSetFlexDirection,  YGNodeStyleGetFlexDirection)  || changed;
        changed = assignProperty(yoga::justify_content, yoga::justifyValues,       YGNodeStyleSetJustifyContent, YGNodeStyleGetJustifyContent) || changed;
        changed = assignProperty(yoga::align_items,     yoga::alignValues,         YGNodeStyleSetAlignItems,     YGNodeStyleGetAlignItems)     || changed;
        changed = assignProperty(yoga::align_content,   yoga::alignValues,         YGNodeStyleSetAlignContent,   YGNodeStyleGetAlignContent)   || changed;
        changed = assignProperty(yoga::align_self,      yoga::alignValues,         YGNodeStyleSetAlignSelf,      YGNodeStyleGetAlignSelf)      || changed;
        changed = assignProperty(yoga::position,        yoga::positionTypeValues,  YGNodeStyleSetPositionType,   YGNodeStyleGetPositionType)   || changed;
        changed = assignProperty(yoga::flex_wrap,       yoga::wrapValues,          YGNodeStyleSetFlexWrap,       YGNodeStyleGetFlexWrap)       || changed;
        changed = assignProperty(yoga::overflow,        yoga::overflowValues,      YGNodeStyleSetOverflow,       YGNodeStyleGetOverflow)       || changed;
        changed = assignProperty(yoga::display,         yoga::displayValues,       YGNodeStyleSetDisplay,        YGNodeStyleGetDisplay)        || changed;

        changed = assignFloatProperty(yoga::flex,         YGNodeStyleSetFlex,        YGNodeStyleGetFlex)        || changed;
        changed = assignFloatProperty(yoga::flex_grow,    YGNodeStyleSetFlexGrow,    YGNodeStyleGetFlexGrow)    || changed;
        changed = assignFloatProperty(yoga::flex_shrink,  YGNodeStyleSetFlexShrink,  YGNodeStyleGetFlexShrink)  || changed;
        changed = assignFloatProperty(yoga::aspect_ratio, YGNodeStyleSetAspectRatio, YGNodeStyleGetAspectRatio) || changed;

        changed = assignFloatProperty(yoga::flex_basis,   YGNodeStyleSetFlexBasis, YGNodeStyleSetFlexBasisPercent, YGNodeStyleGetFlexBasis) || changed;
        changed = assignFloatProperty(yoga::width,        YGNodeStyleSetWidth,     YGNodeStyleSetWidthPercent,     YGNodeStyleGetWidth)     || changed;
        changed = assignFloatProperty(yoga::height,       YGNodeStyleSetHeight,    YGNodeStyleSetHeightPercent,    YGNodeStyleGetHeight)    || changed;
        changed = assignFloatProperty(yoga::min_width,    YGNodeStyleSetMinWidth,  YGNodeStyleSetMinWidthPercent,  YGNodeStyleGetMinWidth)  || changed;
        changed = assignFloatProperty(yoga::min_height,   YGNodeStyleSetMinHeight, YGNodeStyleSetMinHeightPercent, YGNodeStyleGetMinHeight) || changed;
        changed = assignFloatProperty(yoga::max_width,    YGNodeStyleSetMaxWidth,  YGNodeStyleSetMaxWidthPercent,  YGNodeStyleGetMaxWidth)  || changed;
        changed = assignFloatProperty(yoga::max_height,   YGNodeStyleSetMaxHeight, YGNodeStyleSetMaxHeightPercent, YGNodeStyleGetMaxHeight) || changed;

        changed = assignEdgeFloatProperty(YGNodeStyleSetPosition, YGNodeStyleSetPositionPercent, YGNodeStyleGetPosition) || changed;

        changed = assignEdgeFloatPropertyWithPrefix(yoga::margin_,  YGNodeStyleSetMargin,  YGNodeStyleSetMarginPercent,  YGNodeStyleGetMargin)  || changed;
        changed = assignEdgeFloatPropertyWithPrefix(yoga::padding_, YGNodeStyleSetPadding, YGNodeStyleSetPaddingPercent, YGNodeStyleGetPadding) || changed;

        changed = assignEdgeFloatPropertyWithPrefix(yoga::border_,  YGNodeStyleSetBorder, YGNodeStyleGetBorder) || changed;

        return changed;
    }
};

//==============================================================================

LayoutElement::LayoutElement(const Identifier& tag, Context& ctx)
    : StyledElement(tag, ctx),
      layout{ std::make_unique<Layout>(*this) }
{
    registerStyleProperty(yoga::align_items);
    registerStyleProperty(yoga::align_content);
    registerStyleProperty(yoga::align_self);
    registerStyleProperty(yoga::direction);
    registerStyleProperty(yoga::flex_direction);
    registerStyleProperty(yoga::justify_content);
    registerStyleProperty(yoga::display);
    registerStyleProperty(yoga::overflow);
    registerStyleProperty(yoga::flex_wrap);

    registerStyleProperty(yoga::flex);
    registerStyleProperty(yoga::flex_grow);
    registerStyleProperty(yoga::flex_shrink);
    registerStyleProperty(yoga::flex_basis);
    registerStyleProperty(yoga::width);
    registerStyleProperty(yoga::height);
    registerStyleProperty(yoga::min_width);
    registerStyleProperty(yoga::min_height);
    registerStyleProperty(yoga::max_width);
    registerStyleProperty(yoga::max_height);
    registerStyleProperty(yoga::aspect_ratio);

    for (auto it = yoga::edgeValues.cbegin(); it != yoga::edgeValues.cend(); ++it) {
        registerStyleProperty(Identifier(yoga::margin_ + it->first));
        registerStyleProperty(Identifier(yoga::padding_ + it->first));
        registerStyleProperty(Identifier(yoga::border_ + it->first));
    }
}

juce::Rectangle<float> LayoutElement::getLayoutElementBounds() const
{
    return {
        YGNodeLayoutGetLeft  (layout->node),
        YGNodeLayoutGetTop   (layout->node),
        YGNodeLayoutGetWidth (layout->node),
        YGNodeLayoutGetHeight(layout->node)
    };
}

LayoutElement* LayoutElement::getParentLayoutElement()
{
    Element* p{ parent };

    while (p != nullptr) {
        if (p->isLayoutElement())
            return dynamic_cast<LayoutElement*>(p);

        p = p->getParentElement();
    }

    return nullptr;
}

bool LayoutElement::updateLayout()
{
    bool changed{ layout->rebuild() };

    jassert(layout != nullptr);
    jassert(layout->node != nullptr);
    
    const auto numChildren{ YGNodeGetChildCount(layout->node) };

    for (auto i = 0; i < numChildren; ++i) {
        YGNodeRef childNode{ YGNodeGetChild(layout->node, i) };
        jassert(childNode != nullptr);

        if (LayoutElement* childLayoutElement{ reinterpret_cast<LayoutElement*>(YGNodeGetContext(childNode)) })
            changed = childLayoutElement->updateLayout() || changed;
    }

    return changed;
}

void LayoutElement::recalculateLayout(float width, float height)
{
    YGNodeCalculateLayout(layout->node, width, height, YGDirectionInherit);

    if (isComponentElement()) {
        if (auto* componentElement{ dynamic_cast<ComponentElement*>(this) })
            componentElement->updateComponentBoundsToLayoutNode();
    }

    forEachChild([](Element* child) {
        if (child->isComponentElement()) {
            if (auto* componentElement{ dynamic_cast<ComponentElement*>(child) })
                componentElement->updateComponentBoundsToLayoutNode();
        }
    }, true);
}

void LayoutElement::numberOfChildrenChanged()
{
    StyledElement::numberOfChildrenChanged();

    childrenChanged = true;
}

void LayoutElement::reconcileElement()
{
    if (parent == nullptr) {
        if (auto* owner{ layout->node->getOwner() }) {
            YGNodeRemoveChild(owner, layout->node);
        }
    } else {
        if (layout->node->getOwner() == nullptr) {
            if (auto* parentLayoutElement{ getParentLayoutElement() }) {
                const auto count{ YGNodeGetChildCount(parentLayoutElement->layout->node) };
                YGNodeInsertChild(parentLayoutElement->layout->node, layout->node, count);
            }
        }
    }
}

} // namespace vitro
