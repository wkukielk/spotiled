fn main() {
    let dom_color = cmake::Config::new("dom_color")
        .build_target("dom_color")
        .build();

    println!(
        "cargo:rustc-link-search=native={}/build",
        dom_color.display()
    );
    println!("cargo:rustc-link-lib=static=dom_color");

    cxx_build::bridge("src/led_manager.rs")
        .file("dom_color/led_manager/LEDManager.cpp")
        .include("dom_color")
        .include("dom_color/straux")
        .include("dom_color/colors")
        .include("dom_color/histograms2D")
        .include("dom_color/histograms")
        .include("dom_color/histograms/histogram")
        .include("dom_color/histograms/histogram_common")
        .include("dom_color/hsv_histograms")
        .std("c++20")
        .compile("led_manager_bridge");

    println!("cargo:rerun-if-changed=src/led_manager.rs");
    println!("cargo:rerun-if-changed=dom_color/led_manager/LEDManager.hpp");
    println!("cargo:rerun-if-changed=dom_color/led_manager/LEDManager.cpp");
    println!("cargo:rerun-if-changed=dom_color/CMakeLists.txt");
}
