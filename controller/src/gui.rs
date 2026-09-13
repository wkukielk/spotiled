use std::time::Duration;

use eframe::egui;

use crate::status::{self, ControllerStatus};

const GREEN: egui::Color32 = egui::Color32::from_rgb(46, 160, 67);
const RED: egui::Color32 = egui::Color32::from_rgb(220, 53, 69);

pub fn run() -> eframe::Result {
    let options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default()
            .with_inner_size([360.0, 220.0])
            .with_resizable(false),
        ..Default::default()
    };
    eframe::run_native(
        "SpotiLED Controller",
        options,
        Box::new(|_cc| Ok(Box::new(ControllerApp))),
    )
}

struct ControllerApp;

impl eframe::App for ControllerApp {
    fn ui(&mut self, ui: &mut egui::Ui, _frame: &mut eframe::Frame) {
        ui.ctx().request_repaint_after(Duration::from_millis(200));

        egui::CentralPanel::default().show(ui, |ui| {
            ui.add_space(20.0);
            ui.vertical_centered(|ui| match status::get() {
                ControllerStatus::Starting => {
                    ui.heading("Starting…");
                }
                ControllerStatus::Running => {
                    ui.heading(egui::RichText::new("● Running").color(GREEN));
                }
                ControllerStatus::Pairing { sas } => {
                    ui.heading("Pairing in progress");
                    ui.add_space(16.0);
                    ui.label("Confirm this code matches the one on your phone:");
                    ui.add_space(10.0);
                    ui.label(egui::RichText::new(sas).size(40.0).strong().monospace());
                }
                ControllerStatus::Error(message) => {
                    ui.heading(egui::RichText::new("Error").color(RED));
                    ui.add_space(10.0);
                    ui.label(message);
                }
            });
        });
    }
}
