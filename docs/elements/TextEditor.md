# `<TextEditor>`

TextEdtidor element corresponds to Juce TextEditor.

## Attributes

| Attribute   | Description                                 |
|:------------|:--------------------------------------------|
| `text`      | Text being edited                           |
| `emptytext` | Text to be shown then the edit box is empty |
| `readonly`  | `true` for read-only text field             |
| `onchange`  | Text changed event handler                  |
| `onenter`   | Enter (return) key event handler            |


## Style properties

| Property               | Description                                |
|:-----------------------|:-------------------------------------------|
| `multiline`            | `true` for multiline editor                |
| `wrap`                 | `true` to wrap text (instead of scrolling) |
| `password-character`   | Character used for hidden text             |
| `text-color`           | Text color                                 |
| `empty-text-color`     | Color of the empty text string             |
| `background-color`     | Background color                           |
| `highlight-color`      | Selection background color                 |
| `highlight-text-color` | Selection text color                       |
| `border-color`         | Border color                               |
| `border-radius`        | Border corners radius                      |
| `border-width`         | Border width in pixels                     |
| `focused-border-color` | Border color when active                   |
