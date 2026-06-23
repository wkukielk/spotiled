#[repr(C)]
pub struct Rgb {
    pub r: u8,
    pub g: u8,
    pub b: u8,
}

unsafe impl cxx::ExternType for Rgb {
    type Id = cxx::type_id!("RGB");
    type Kind = cxx::kind::Trivial;
}

#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        include!("led_manager/LEDManager.hpp");

        type RGB = super::Rgb;

        fn led_manager_get_color(pixels: &[u16]) -> RGB;
    }
}

pub use ffi::led_manager_get_color as get_color;
