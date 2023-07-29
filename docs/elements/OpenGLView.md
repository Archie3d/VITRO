# `<OpenGLView>`

OpenGL 2D render surface painted by a fragment shader.

## Attributes

| Attribute | Description                           |
|:----------|:--------------------------------------|
| `fps`     | Repaint frequency (frames per second) |

Child elements `<texture>` and `<shader>` have special mining when inside the `<OpenGLView>` - they define textures and shaders to be used to render the view.

## `<texture>`

This element defines a texture loaded from the resource.

### `<texture>` attributes

| Attribute | Description                         |
|:----------|:------------------------------------|
| `name`    | Texture name (used for referencing) |
| `src`     | Texture surce image                 |

For example:

```xml
<View>
    <OpenGLView>
        <texture name="wall" src="brickwall.jpg" />
        <texture name="sky" src="bluesky.jpg" />
        <shader src="shader.glsl">
            <uniform name="background" type="texture" value="wall" />
            <uniform name="foreground" type="texture" value="sky" />
        </shader>
    </OpenGLView>
</View>
```
Texture name is used in shaders uniforms (see below).

## `<shader>`

This element defines a fragment shader. Shader's source code is loaded from resource.

### `<shader>` attributes

| Attribute | Description                        |
|:----------|:-----------------------------------|
|`name`     | Shader name (used for referencing) |
|`src`      | Shader source file                 |
|`width`    | Output framebuffer width           |
|`height`   | Output framebuffer height          |

When `width` and `height` attributes are defined the shader will render into the internal frame buffer of the given size, instead of rendering on screen. The framebuffer can then be referenced by another shader as a texture (via a uniform by shader's name).

### `<uniform>`
A shader may have multiple uniform variables. A uniform variable can have a value assigned from outside the shader and will be firther read by the fragment shader. Uniforms are defined via the `<uniform>` elements inside the `<shader>`

#### `<uniform>` attributes

| Attribute | Description                         |
|:----------|:------------------------------------|
|`name`     | Variable name (as in shader's code) |
|`type`     | Data type (see below)               |
|`value`    | Value                               |

The following types of uniforms is currently suported:

| Type      | Description                 |
|:----------|:----------------------------|
|`int`      | 32-bit integer              |
|`float`    | 32-bit floating point value |
|`vec2`     | Vector of 2 floats          |
|`vec3`     | Vector of 3 floats          |
|`vec4`     | Vector of 4 floats          |
|`float_vec`| Array of floats             |
|`texture`  | 2D texture                  |

For example:

```xml
<View>
    <OpenGLView>
        <shader src="lava.glsl">
            <uniform name="color1" type="vec3" value="1.0, 0.0, 0.0" />
            <uniform name="color2" type="vec3" value="0.0, 1.0, 0.2" />
        </shader>
    </OpenGLView>
</View>
```

When there are multiple shaders defined inside a `<OpenGLView>` then will be executed in the order of their appearence in the XML. Multiple shaders can be used to make several rendering passed to build the output image:

```xml
<View>
    <OpenGLView>
        <shader name="pass1" src="pass1.glsl" width="512" height="512" />
        <shader name="pass2" src="pass2.glsl" width="512" height="512" />
        <shader src="final_pass.glsl">
            <uniform name="tex1" type="texture" value="pass1" />
            <uniform name="tex2" type="texture" value="pass2" />
        </shader>
    </OpenGLView>
</View>
```

### Dynamically assign uniform values

Uniform elements sit on the document tree alogn all other elements and thus can be accessed by their id from JavaScript or C++ code:

```html
<View>
    <OpenGLView>
        <shader src="paint.glsl">
            <uniform id="paint_color" name="color" type="vec3" value="0.0, 0.0, 0.0" />
        </shader>
    </OpenGLView>

    <script>
        const color=[1.0, 0.0, 0.0];
        view.getElementById("paint_color").setAttribute("value", color);
    </script>
</View>
```
> :warning: Note that assiging a uniform variable does not cause OpenGL view repaint, unless it's resized or set to constanmt repaintinf via the `fps` attribute.

### Shader input and output

A fragment shader must take a `vec2 pixelPos` on input and produce `vec4 fragColor` on output:

```glsl
in vec2 pixelPos
out vec4 fragColor;

uniform vec2 iResolution;

void main()
{
    vec2 uv = pixelPos / iResolution;
    fragColor = vec4(uv, 0.2f, 1.0f);
}
```

### Shader reserved uniforms

There are several predefined uniforms accessible from the shader:

| Uniform         | Type  |Description                                      |
|:----------------|:------|:------------------------------------------------|
|`iResolution`    |`vec2` | Target render surface size (x, y)               |
|`iViewResolution`|`vec2` | OpenGLView component size (x, y)                |
|`iFrame`         |`int`  | Frame number (incremented at each frame)        |
|`iTimeDelta`     |`float`| Time (in seconds) between consecutive frames    |
|`iTime`          |`float`| Time (in seconds) elapsed from the render start |
|`iMouse`         |`vec2` | Mouse cursor position relative to the component |

> :point_right: Shader's coordinate system origin is at the bottom left corner of the rendering surface.

## Child UI elements

Child UI elements inside the `<OpenGLView>` will be rendered *on top of the shader rendered surface* via JUCE OpenGL rasterizer:

```xml
<View>
    <OpenGLView fps="30" >
        <shader src="background.glsl" />

        <Label text="This label will be show inside the OpenGL view" />
    </OpenGLView>
</View>
```