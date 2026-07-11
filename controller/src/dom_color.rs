#[repr(C)]
#[derive(Debug, PartialEq)]
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

pub use ffi::get_dominating_color;

#[cfg_attr(test, mockall::automock)]
pub trait JPEGDecoder: Send + Sync {
    fn decode_jpg_to_rgb565(jpg: &[u8]) -> Result<Vec<u16>, image::ImageError>;
}

pub struct StdJPEGDecoder {}

impl JPEGDecoder for StdJPEGDecoder {
    fn decode_jpg_to_rgb565(jpg: &[u8]) -> Result<Vec<u16>, image::ImageError> {
        use image::GenericImageView;
        let img = image::load_from_memory_with_format(jpg, image::ImageFormat::Jpeg)?;
        Ok(img
            .pixels()
            .map(|(_, _, px)| {
                ((px[0] as u16 & 0xf8) << 8) | ((px[1] as u16 & 0xfc) << 3) | (px[2] as u16 >> 3)
            })
            .collect())
    }
}

pub fn get_jpg_dominating_color<T: JPEGDecoder>(
    jpg: bytes::Bytes,
) -> Result<Rgb, image::ImageError> {
    let pixels = T::decode_jpg_to_rgb565(&jpg)?;
    Ok(get_dominating_color(&pixels))
}
