# `<style>`

Style element defines a CSS style sheet that will be applied globally to the view.

The inner content of the `<style>` XML element is interpreted as CSS.

> Use `CDATA` section when using special symbols that may break XML syntax.

## Attributes

`<style>` may have `src` attribute to point to external css file or binary resources:

```html
<View>
    <style src="style.css" />
</View>
```