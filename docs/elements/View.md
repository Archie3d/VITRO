# `<View>`
View must be the root element of the UI container.

View element is exposed as a global `view` object into the script environment.

## Styling

View element has the same styling properties as [Panel](Panel.md).

## Scripting

| Method name               | Description                          |
|:--------------------------|:-------------------------------------|
| `createElement(tag_name)` | Create a new element for a given tag |

Since `view` is a root element it can be used to look for an element by id globally:

```js
var elem = view.getElementById("id_to_find");
```