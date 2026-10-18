//
//  ManualTab.swift
//  spotiled-connect
//
//  Created by wkuki on 23/07/2026.
//

import SwiftUI
import SpotiLEDKit
import Network

private enum ManualAnimation: String, CaseIterable, Identifiable {
    case pulsate = "Pulsate"

    var id: Self { self }
}

struct ManualTab: View {
    @State private var color: Color = .mint;
    @State private var controller = ManualColorController()
    @State private var esp32Connection = DirectESP32Connection()
    @State private var autoBrowser = DeviceBrowser(discovery: BonjourServiceDiscovery(serviceType: "_s-led-auto._tcp"))
    @State private var selectedAnimation: ManualAnimation = .pulsate
    @State private var showDeviceSettings = false
    @Environment(SelectedESP32Store.self) private var esp32Store
    @Environment(\.esp32Session) private var session

    private var matchedAutoDevice: DiscoveredDevice? {
        guard let device = esp32Store.device else { return nil }
        return autoBrowser.discoveredDevices.first { $0.name == device.name }
    }

    var body: some View {
        NavigationStack {
            VStack {
                SelectedESP32Label(device: esp32Store.device)
                ColorPicker("Color", selection: $color, supportsOpacity: false)

                Picker("Animation", selection: $selectedAnimation) {
                    ForEach(ManualAnimation.allCases) { animation in
                        Text(animation.rawValue).tag(animation)
                    }
                }
                .pickerStyle(.menu)

                animationPanel
            }
            .padding(.horizontal)
            .navigationTitle("Manual mode")
            .toolbar {
                ToolbarItem(placement: .topBarTrailing) {
                    Button {
                        showDeviceSettings = true
                    } label: {
                        Image(systemName: "gearshape")
                    }
                }
            }
            .sheet(isPresented: $showDeviceSettings) {
                DeviceSettingsSheet(connection: esp32Connection, esp32Store: esp32Store)
            }
            .hideKeyboardOnTap()
        }
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
        .onChange(of: color) { _, newColor in
            controller.send(color: newColor)
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
            PulsateAnimationPanel(connection: esp32Connection, color: color)
        }
    }
}

private struct PulsateAnimationPanel: View {
    let connection: any DirectESP32Connecting
    let color: Color

    @State private var durationMs: Int32 = 5000
    
    var body: some View {
        VStack(spacing: 8) {
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

@Observable
public final class ManualColorController {
    private var sender: RGBSending? = nil;
    
    public init() {}
    
    public func send(color: Color) {
        sender?.send(RGBColor(color))
    }

    public func updateTarget(_ endpoint: NWEndpoint) {
        sender = UDPRGBSender(endpoint: endpoint)
    }
}


#Preview {
    ManualTab()
        .environment(SelectedESP32Store())
}

