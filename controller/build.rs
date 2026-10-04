fn main() {
    let dom_color = cmake::Config::new("../dom_color")
        .build_target("dom_color")
        .build();

    println!(
        "cargo:rustc-link-search=native={}/build",
        dom_color.display()
    );
    println!("cargo:rustc-link-lib=static=dom_color");

    cxx_build::bridge("src/dom_color.rs")
        .file("cxx/bridge.cpp")
        .include("cxx")
        .include("../dom_color/src")
        .include("../dom_color/src/straux")
        .include("../dom_color/src/colors")
        .include("../dom_color/src/histograms2D")
        .include("../dom_color/src/histograms")
        .include("../dom_color/src/histograms/histogram")
        .include("../dom_color/src/histograms/histogram_common")
        .include("../dom_color/src/hsv_histograms")
        .std("c++20")
        .compile("dom_color_bridge");

    println!("cargo:rerun-if-changed=src/dom_color.rs");
    println!("cargo:rerun-if-changed=cxx/bridge.hpp");
    println!("cargo:rerun-if-changed=cxx/bridge.cpp");
    println!("cargo:rerun-if-changed=../dom_color/CMakeLists.txt");
    println!("cargo:rerun-if-changed=../dom_color/src");
}
