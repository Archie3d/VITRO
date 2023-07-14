#ifdef VITRO_H_INCLUDED
#   error "Incorrect use of cpp file"
#endif

#include "vitro.h"

#include "yoga/log.cpp"
#include "yoga/Utils.cpp"
#include "yoga/YGConfig.cpp"
#include "yoga/YGEnums.cpp"
#include "yoga/YGLayout.cpp"
#include "yoga/YGNode.cpp"
#include "yoga/YGNodePrint.cpp"
#include "yoga/YGStyle.cpp"
#include "yoga/YGValue.cpp"
#include "yoga/Yoga.cpp"
#include "yoga/event/event.cpp"
#include "yoga/internal/experiments.cpp"

using namespace juce;

#include "css/vitro_Stylesheet.cpp"
#include "css/vitro_CSSParser.cpp"

#include "core/vitro_Utils.cpp"
#include "core/vitro_Loader.cpp"
#include "core/vitro_Context.cpp"
#include "core/vitro_Attributes.cpp"
#include "core/vitro_Script.cpp"
#include "core/vitro_Element.cpp"
#include "core/vitro_ElementsFactory.cpp"
#include "core/vitro_StyledElement.cpp"
#include "core/vitro_LayoutElement.cpp"
#include "core/vitro_ComponentElement.cpp"
#include "core/vitro_View.cpp"

#include "widgets/vitro_Panel.cpp"
#include "widgets/vitro_Label.cpp"
#include "widgets/vitro_Button.cpp"
#include "widgets/vitro_Slider.cpp"
