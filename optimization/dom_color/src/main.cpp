#include <pybind11/pybind11.h>

#include "dominant_color.hpp"
#include "rgb.hpp"

namespace py = pybind11;

namespace
{
    RGB get_dominating_color_buffer(py::buffer pixels)
    {
        py::buffer_info info = pixels.request();

        if (info.format != py::format_descriptor<uint16_t>::format())
            throw std::runtime_error(
                "Incompatible buffer format: expected an array of uint16 (RGB565 pixels)");
        if (info.ndim != 1)
            throw std::runtime_error("Expected a 1-D buffer of pixels");

        auto *data = static_cast<const uint16_t *>(info.ptr);
        auto size = static_cast<size_t>(info.shape[0]);
        return get_dominating_color(std::span<const uint16_t>{data, size});
    }
} // namespace

PYBIND11_MODULE(_core, m, py::mod_gil_not_used(), py::multiple_interpreters::per_interpreter_gil())
{
    m.doc() = R"pbdoc(
        Python bindings for the dom_color C++ library.
    )pbdoc";

    py::class_<RGB>(m, "RGB")
        .def(py::init<uint8_t, uint8_t, uint8_t>(), py::arg("r"), py::arg("g"), py::arg("b"))
        .def_readwrite("r", &RGB::r)
        .def_readwrite("g", &RGB::g)
        .def_readwrite("b", &RGB::b)
        .def("to_pixel16", &RGB::to_pixel16, "Pack into an RGB565 uint16")
        .def("to_pixel32", &RGB::to_pixel32, "Pack into an RGB888 uint32")
        .def("__eq__", &RGB::operator==)
        .def("__repr__", [](const RGB &c)
             { return "RGB(r=" + std::to_string(c.r) + ", g=" + std::to_string(c.g) + ", b=" + std::to_string(c.b) + ")"; });

    m.def("get_dominating_color", &get_dominating_color_buffer, py::arg("pixels"), R"pbdoc(
        Compute the dominant color of an image.

        pixels must be a 1-D buffer of RGB565-packed uint16 values (e.g. a
        numpy array with dtype=numpy.uint16, or an array.array("H", ...)).
    )pbdoc");
}
