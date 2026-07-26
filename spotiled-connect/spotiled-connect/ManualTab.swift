//
//  ManualTab.swift
//  spotiled-connect
//
//  Created by wkuki on 23/07/2026.
//

import SwiftUI
import SpotiLEDKit

struct ManualTab: View {
    @State private var controller = ManualColorController()
    @Environment(SelectedESP32Store.self) private var esp32Store

    var body: some View {
        VStack() {
            Text("Manual mode")
                .navigationTitle("Manual mode")
            if let device = esp32Store.device {
                Text("Target: \(device.name)")
                    .foregroundStyle(.secondary)
            } else {
                Text("No ESP32 selected")
                    .foregroundStyle(.secondary)
            }
            ColorPicker("Color", selection: $controller.color, supportsOpacity: false)
        }
        .padding(.horizontal)
        .task(id: esp32Store.device) {
            if let device = esp32Store.device {
                controller.updateTarget(device.endpoint)
            }
        }
    }
}

#Preview {
    ManualTab()
        .environment(SelectedESP32Store())
}

