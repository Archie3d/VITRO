# VITRO
Declarative UI module for [JUCE](https://github.com/juce-framework/JUCE).

This module uses [Yoga](https://github.com/facebook/yoga/tree/main/yoga) flexbox layout engine for positioning the UI elements, and [QuickJS](https://github.com/bellard/quickjs) JavaScript engine for UI scripting.

## :triangular_ruler: Design idea

This library somewhat mimics the HTML/CSS/JavaScript way of constructing the interface. The UI elements are defined via a tree-like structure via XML or programmaticaly. Each XML element corresponds directly either to JUCE component/widget, to a layout element, or to a data element. Each element can carry some data, which is defined via XML attributes or can be set programmaticaly via script. Internally the elements and their attributes are stored in JUCE [ValueTree](https://docs.juce.com/master/classValueTree.html).

A subset of CSS syntax is used to define UI elements style - layout rules (which get forwarded to Yoga layout engine), or visual styling, like colour, border, font, cursor, etc. Different widgets respond to different style attributes. CSS selectors are used to calculate the styling rules for the UI widgets.

UI elements tree can be accessed from a JavaScript. A script can look for a particular element by its ID, read or modify its properties, create and add new elements to the tree, or remove elements. Interactive elements, like buttons, edit boxes or sliders have event attributes, which can reference JavaScript code that will be executed on user action, like `onclick` or `onchange`.

UI elements can be accessed from C++ as JUCE Components. This way they can be linked to C++ code (JavaScript usage is not required).
## :computer: Compilation

This module must be used via CMake (VITRO is not compatible with Projucer).

Add [JUCE](https://github.com/juce-framework/JUCE) and [VITRO](https://github.com/Archie3d/VITRO) submodules to your project, and then link `juce::vitro` with your target:

```CMake
add_subdirectory(JUCE)
add_subdirectory(VITRO)

target_link_libraries(${TARGET}
    PRIVATE
        juce::juce_core
        juce::juce_data_structures
        juce::juce_gui_basics

        juce::vitro

        juce::juce_recommended_config_flags
)
```

## :page_facing_up: Usage
The easiest way to instantiate the UI is via the `vitro::ViewContainer` component. It allows loading the UI description directly from embedded binary resources (or local filesystem if needed).
```cpp
class MainComponent : public juce::Component
{
public:
    MainComponent()
    {
        addAndMakeVisible(viewContainer);
        viewContainer.loadFromResource("view.xml", "style.css", "script.js");
    }

    void resized() override
    {
        viewContainer.setBounds(getLocalBounds());
    }

private:
    vitro::ViewContainer viewContainer{};
};
```

The `view.xml` document must start from the `<View>` root element, with all child elements placed inside:
```xml
<View>
    <Label text="Confirm:" />
    <TextButton text="OK" />
</View>
```

The style document is optional. It contains the CSS desacription of the UI elements:
```css
View {
    background-color: black;
    flex-direction: column;
    padding-all: 8;
}

Label {
    height: 20;
    color: grey;
}

Label:hover {
    color: blue;
}
```

The script is optional as well. When provided the script is executed _before_ the UI view gets populated with the elements.

## :ledger: Detailed information

:point_right: [See more detailed imformation here](docs/docs.md)

## :eyes: Examples
:point_right: [See the test project for examples.](https://github.com/Archie3d/VITRO-test/tree/master/examples)

## :briefcase: License
VITRO is licensed under MIT terms. However it depends on external librarlies:
- JUCE has a dual Commertcial/GPL licence
- QuickJS is licensed under MIT
