namespace vitro {

// List of the frequently used attributes. These include the attributes
// that can be assigned to the elements' value tree, and also
// the stylesheet property names.

namespace attr {

const Identifier id     ("id");
const Identifier clazz  ("class");
const Identifier style  ("style");
const Identifier src    ("src");

const Identifier text   ("text");

const Identifier hover  ("hover");
const Identifier active ("active");

//==============================================================================
namespace css {

const Identifier color            ("color");
const Identifier background_color ("background-color");
const Identifier background_image ("background-image");

const Identifier border_color     ("border-color");
const Identifier border_radius    ("border-radius");
const Identifier border_width     ("border-width");

const Identifier shadow_color     ("shadow-color");
const Identifier shadow_radius    ("shadow-radius");
const Identifier shadow_offset_x  ("shadow-offset-x");
const Identifier shadow_offset_y  ("shadow-offset-y");

const Identifier text_align       ("text-align");

const Identifier font_family      ("font-family");
const Identifier font_style       ("font-style");
const Identifier font_size        ("font-size");
const Identifier font_kerning     ("font-kerning");

const Identifier cursor           ("cursor");

} // namespace css

} // namespace attr

} // namespace vitro
