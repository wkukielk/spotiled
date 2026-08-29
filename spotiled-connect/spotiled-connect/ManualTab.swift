//
//  ManualTab.swift
//  spotiled-connect
//
//  Created by wkuki on 23/07/2026.
//

import SwiftUI
import SpotiLEDKit

private enum ManualAnimation: String, CaseIterable, Identifiable {
    case pulsate = "Pulsate"

    var id: Self { self }
}

struct ManualTab: View {
    @State private var controller = ManualColorController()
    @State private var esp32Connection = DirectESP32Connection()
    @State private var autoBrowser = DeviceBrowser(discovery: BonjourServiceDiscovery(serviceType: "_s-led-auto._tcp"))
    @State private var selectedAnimation: ManualAnimation = .pulsate
    @Environment(SelectedESP32Store.self) private var esp32Store
    @Environment(\.esp32Session) private var session

    private var matchedAutoDevice: DiscoveredDevice? {
        guard let device = esp32Store.device else { return nil }
        return autoBrowser.discoveredDevices.first { $0.name == device.name }
    }

    var body: some View {
        VStack {
            Text("Manual mode")
                .navigationTitle("Manual mode")
            SelectedESP32Label(device: esp32Store.device)
            ColorPicker("Color", selection: $controller.color, supportsOpacity: false)

            Picker("Animation", selection: $selectedAnimation) {
                ForEach(ManualAnimation.allCases) { animation in
                    Text(animation.rawValue).tag(animation)
                }
            }
            .pickerStyle(.menu)

            animationPanel
        }
        .padding(.horizontal)
        .onAppear {
            autoBrowser.start()
        }
        .task(id: esp32Store.device) {
            guard let device = esp32Store.device else { return }
            controller.updateTarget(device.endpoint)
            try? await session.disconnectESP32()
        }
        .task(id: matchedAutoDevice) {
            guard let matchedAutoDevice else { return }
            esp32Connection.connect(to: matchedAutoDevice)
        }
        .onDisappear {
            esp32Connection.disconnect()
            autoBrowser.stop()
        }
    }

    @ViewBuilder
    private var animationPanel: some View {
        switch selectedAnimation {
        case .pulsate:
            PulsateAnimationPanel(connection: esp32Connection)
        }
    }
}

private struct PulsateAnimationPanel: View {
    let connection: any DirectESP32Connecting

    @State private var color = Color(.mint)
    @State private var durationMs: Int32 = 500

    var body: some View {
        VStack(spacing: 8) {
            ColorPicker("Color", selection: $color, supportsOpacity: false)
            LabeledNumberField(title: "Duration (ms)", value: $durationMs)
            Button("Play") {
                Task {
                    try? await connection.pulsate(color: RGBColor(color), durationMs: durationMs)
                }
            }
            .buttonStyle(.appPrimary)
        }
        .padding(8)
        .background(Color.cardBackground)
        .border(Color.cardBorder, width: 1)
    }
}

#Preview {
    ManualTab()
        .environment(SelectedESP32Store())
}

