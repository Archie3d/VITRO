# `<script>`

JavaScript code can be injected into the UI via `<script>` element in UI definition XML. The text content of the `<script> ... </script>` block is interpreted as JavaScript code.

> :warning: Since a script may contain characters that conflict with XML syntax, it should better be wrapped into `CDATA` section.

```html
<View>
    <script>
    <![CDATA[
        console.log('Hello from JS');
    ]]>
    </script>
</View>
```

## Using `<script src="">` attribute

Providing the `src` attribute to the `<script>` element will load and execute the script from the resource located by the `src` value.

```xml
<View>
    <script src="script.js" />
</View>
```
It is possible to have multiple `<script>` elements in the UI XML definition. The scripts get evaluated in the order of the elements creation from the XML.
> :warning: This means that a script cannot reference elemets that will be created after its `<script>`

```html
<View>
    <Label id="labelA" />

    <script>
    <![CDATA[
        // labelA is available since it was created before this script
        var labelA = view.getElementById("labelA");

        // labelB is not available since it has not been yet created
        var labelB = view.getElementById("labelB");
    ]]>
    </script>

    <Label id="LabelB" />
</View>
```