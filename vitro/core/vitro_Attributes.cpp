namespace vitro {

// List of the frequently used attributes. These include the attributes
// that can be assigned to the elements' value tree, and also
// the stylesheet property names.

namespace attr {

const Identifier id         ("id");
const Identifier clazz      ("class");
const Identifier style      ("style");
const Identifier src        ("src");

const Identifier enabled    ("enabled");
const Identifier visible    ("visible");
const Identifier readonly   ("readonly");

const Identifier text       ("text");
const Identifier value      ("value");
const Identifier emptytext  ("emptytext");
const Identifier decimals   ("decimals");
const Identifier min        ("min");
const Identifier max        ("max");
const Identifier step       ("step");

const Identifier hover      ("hover");
const Identifier active     ("active");
const Identifier toggle     ("toggle");
const Identifier radiogroup ("radiogroup");

//==============================================================================
namespace css {

const Identifier color                     ("color");
const Identifier alpha                     ("alpha");
const Identifier background_color          ("background-color");
const Identifier background_color_on       ("background-color-on");
const Identifier background_image          ("background-image");
const Identifier highlight_color           ("highlight-color");
const Identifier highlighted_text_color    ("highlighted-text-color");

const Identifier border_color              ("border-color");
const Identifier border_radius             ("border-radius");
const Identifier border_width              ("border-width");
const Identifier focused_border_color      ("focused-border-color");

const Identifier shadow_color              ("shadow-color");
const Identifier shadow_radius             ("shadow-radius");
const Identifier shadow_offset_x           ("shadow-offset-x");
const Identifier shadow_offset_y           ("shadow-offset-y");

const Identifier text_align                ("text-align");
const Identifier text_color                ("text-color");
const Identifier text_color_on             ("text-color-on");
const Identifier text_color_off            ("text-color-off");
const Identifier empty_text_color          ("empty-text-color");

const Identifier multiline                 ("multiline");
const Identifier wrap                      ("wrap");
const Identifier password_character        ("password-character");

const Identifier tick_color                ("tick-color");
const Identifier tick_disabled_color       ("tick-disabled-color");

const Identifier font_family               ("font-family");
const Identifier font_style                ("font-style");
const Identifier font_size                 ("font-size");
const Identifier font_kerning              ("font-kerning");

const Identifier cursor                    ("cursor");

const Identifier toggle                    ("toggle");
const Identifier trigger_down              ("trigger-down");
const Identifier click_through             ("click-through");

const Identifier button_style              ("button-style");

const Identifier image_normal              ("image-normal");
const Identifier image_over                ("image-over");
const Identifier image_down                ("image-down");
const Identifier image_disabled            ("image-disabled");
const Identifier image_normal_on           ("image-normal-on");
const Identifier image_over_on             ("image-over-on");
const Identifier image_down_on             ("image-down-on");
const Identifier image_disabled_on         ("image-disabled-on");

const Identifier slider_style              ("slider-style");

const Identifier thumb_color               ("thumb-color");
const Identifier track_color               ("track-color");
const Identifier fill_color                ("fill-color");
const Identifier text_box_color            ("text-box-color");
const Identifier text_box_background_color ("text-box-background-color");
const Identifier text_box_highlight_color  ("text-box-highlight-color");
const Identifier text_box_border_color     ("text-box-border-color");
const Identifier popup_color               ("popup-color");
const Identifier text_box_position         ("text-box-position");
const Identifier text_box_read_only        ("text-box-read-only");
const Identifier text_box_width            ("text-box-width");
const Identifier text_box_height           ("text-box-height");

} // namespace css

} // namespace attr

} // namespace vitro
