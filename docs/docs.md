# Details about UI elements, scripting and styling

## Elements
Most UI elements tags directly correspond to JUCE components.
Here is the list of all reserved elements:

- [View](elements/View.md)
- [Panel](elements/Panel.md)
- [Label](elements/Label.md)
- [TextButton](elements/TextButton.md)
- [Slider](elements/Slider.md)
- [TextEditor](elements/TextEditor.md)
- [CodeEditor](elements/CodeEditor.md)
- [ComboBox](elements/ComboBox.md)
- [ScrollArea](elements/ScrollArea.md)
- [OpenGLView](elements/OpenGLView.md)
- [MidiKeyboard](elements/MidiKeyboard.md)

### Special elements
- [script](elements/script.md)
- [style](elements/style.md)

If a UI XML contains an unknown element the following rule is used to instantiate it:
1. If element's tag start with a capital letter, the element will be instantiated as [Panel](elements/Panel.md).
2. All other unknown tags will instantiate base elements that do not have visual representation. Such elements can be used to store internal data.

## Styling

All visual elements respond to the layout styling properties. These properties are passed directly to the Yoga layout engine when calculating the elements bounding boxes.

On top of the layout styles, each element type may respond to additional styling properties.

### Common style properties

These are the CSS properties applicable to all visual elements:

#### color
The color values are either defined by RGBA hex value as `#rrggbbaa` or by color name. Color names are passed directly to `juce::Colour`.

#### cursor
`cursor` property defines mouse cursor when over the element. Possible values are: `none`, `auto`, `wait`, `pointer`, `hand`, `crosshair`, `copy`, `drag`.

#### text-align
Some elements that show text (like [Label](elements/Label.md)) respond to `text-align` style property. The property can take the following values: `left`, `right`, `top`, `bottom`, `center`, `justify`.

## Attributes

An element may have arbitrary attributes assigned either via XML definition or from a script. Some attributes are reserved depending on element type, modifying such attributes will affect element's view or behavior.

Visual elements have predefined attributes that get assigned as user interacts with an element:

| Attribute | Description                                                       |
|:----------|:------------------------------------------------------------------|
| `hover`   | Set to `true` when mouse is over the element                      |
| `active`  | Set to `true` when mouse buttons is clicked when over the element |

These attributes are used in style selectors to decorate elements accordingly, e.g.
```css
Label {
    height: 20;
    color: white;
}

Label:hover {
    font-style: underline;
}
```

## Scripting

UI can be scripted using JavaScript. There are several ways to get JavaScript into the application.

### Loading and evaluating from script source

JavaScript can be injected into the application via `vitro::Context`:
```cpp
const juce::String script = R"(
    console.log('Hello from JS');
)";
viewContainer.getContext().eval(script);
```
### Using `<script>` element
[See this section about using the \<script\> element.](elements/script.md)

### Elements' common properties
All UI elements expose the following properties to the JavaScript interface.

| Property name   | Access     | Description                        |
|:----------------|:-----------|:-----------------------------------|
| `tagName`       | read only  | Element's tag                      |
| `id `           | read/write | Element's ID                       |
| `class`         | read/write | Element'c class string             |
| `style`         | read/write | Element's local style attribute    |
| `parentElement` | read only  | Parent element                     |
| `children`      | read only  | Array of child elements            |
| `attributes`    | read only  | Dictionary of element's attributes |

> :warning:
> `children` property cannot be used to modify (append or remove) element's children. Instead `addChild()` and `removeChild()` should be used.

> :warning:
> `attributes` property cannot be used to modify element's attributes. Instead `setAttribute()` anf `getAttribute()` methods should be used.

#### Layout elements' properties

| Property | Access    | Description                                                 |
|:---------|:----------|:------------------------------------------------------------|
| `bounds` | read only | Element's bounds within its parent: `{x, y, width, height}` |

#### Component element's properties

These properties are exposed by the component elements (widgets).

| Property       | Access    | Description                                                   |
|:---------------|:----------|:--------------------------------------------------------------|
| `viewBounds`   | read only | Element's bounds within the top view: `{x, y, width, height}` |
| `screenBounds` | read only | Element's bounds within the screen: `{x, y, width, height}`   |


### Elements' common methods

| Method name              | Description                      |
|:-------------------------|:---------------------------------|
| `getAttribute(name)`     | Read element's attribute value   |
| `setAttribute(name)`     | Assign element's attribute value |
| `hasAttribute(name)`     | Tell whether an attribute exists |
| `appendChild(element)`   | Append a child element           |
| `removeChild(element)`   | Remove a child element           |
| `replaceChildren(array)` | Replace children with a new set  |

> :information_source:
> New elements are create via the [`view`](elements/View.md) global object (which is the instance of the global [View](elements/View.md) element):
> `var elem = view.createElement("Button");`

### Scripting attributes

Visual elements may trigger events that can have script attached to them (like `onclick` or `onchange`). Any elements can have `onload` attribute, which will be interpreted as JavaScript and evaluated when element gets instantiated:

```html
<View onload="entry();">
    <script>
        function entry() {
            console.log("UI has been loaded.");
        }
    </script>
</View>
```

### Drag and drop

An element will be draggable when its `draggable` attribute is set to `true`. Any element can accept drops if its `acceptdrop` attribute is set to `true`. When drop gets accepted, the `ondrop` script attribute will be evaluated on the drop target element.

```html
<View>
    <Label text="Draggable" draggable="true" />
    <Panel acceptdrop="true" id="drop_target"/>

    <script>
        // Bind ondrop event to the drop target panel
        var panel = view.getElementById('drop_target');
        panel.setAttribute('ondrop', function(dropped_element) {
            if (dropped_element.tagName == 'Label') {
                console.log('Dropped label ' + dropped_element.getAttribute('text'));
            }
        });
    </script>
</View>
```
