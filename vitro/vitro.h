/*
 ==============================================================================

    BEGIN_JUCE_MODULE_DECLARATION

    ID:            vitro
    vendor:        Arthur Benilov
    version:       1.0.0
    name:          vitro
    description:   Declarative UI
    website:       https://github.com/Archie3d/juce_melon
    license:       GPL/Commercial

    dependencies:  juce_gui_basics, juce_gui_extra, juce_opengl

    END_JUCE_MODULE_DECLARATION

 ==============================================================================
*/

#pragma once

#define VITRO_H_INCLUDED

#include <optional>

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_opengl/juce_opengl.h>

#ifdef VITRO_USE_INTERNAL_QUICK_JS

extern "C" {
#include "quickjs.h"
}

#else

#include <juce_core/javascript/choc/javascript/choc_javascript_QuickJS.h>
using namespace choc::javascript::quickjs;

#endif // VITRO_USE_INTERNAL_QUICK_JS

#include "css/vitro_Stylesheet.h"
#include "css/vitro_CSSParser.h"

#include "core/vitro_Utils.h"
#include "core/vitro_Loader.h"
#include "core/vitro_Attributes.h"
#include "core/vitro_LookAndFeel.h"
#include "core/vitro_Element.h"
#include "core/vitro_Context.h"
#include "core/vitro_Script.h"
#include "core/vitro_Style.h"
#include "core/vitro_ElementsFactory.h"
#include "core/vitro_StyledElement.h"
#include "core/vitro_LayoutElement.h"
#include "core/vitro_ComponentElement.h"
#include "core/vitro_ComponentElementWithBackground.h"
#include "core/vitro_View.h"
#include "core/vitro_ViewContainer.h"

#include "widgets/vitro_Panel.h"
#include "widgets/vitro_Label.h"
#include "widgets/vitro_Button.h"
#include "widgets/vitro_Slider.h"
#include "widgets/vitro_Svg.h"
#include "widgets/vitro_TextEditor.h"
#include "widgets/vitro_ScrollArea.h"
#include "widgets/vitro_ComboBox.h"
#include "widgets/vitro_MenuBar.h"
#include "widgets/vitro_CodeEditor.h"
#include "widgets/vitro_OpenGLView.h"
#include "widgets/vitro_MidiKeyboard.h"
