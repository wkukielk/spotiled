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
        include!("bridge.hpp");

        type RGB = super::Rgb;

        fn get_dominating_color(pixels: &[u16]) -> RGB;
    }
}

pub use ffi::get_dominating_color as get_dominating_color;
