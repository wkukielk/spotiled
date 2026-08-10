//
//  MiddleTab.swift
//  spotiled-connect
//
//  Created by wkuki on 23/07/2026.
//

import SwiftUI
import SpotiLEDKit


struct MiddleTab: View {
    @State private var isProvisioningDevice = false
    @State private var isSelectingController = false
    @State private var isSelectingESP32 = false
    @Environment(\.controllerConnection) private var connection
    @Environment(SelectedESP32Store.self) private var esp32Store

    var body: some View {
        NavigationStack {
            homePage
                .navigationDestination(isPresented: $isProvisioningDevice) {
                    ProvisionDeviceView()
                }
                .navigationDestination(isPresented: $isSelectingController) {
                    SelectControllerView()
                }
                .navigationDestination(isPresented: $isSelectingESP32) {
                    SelectESP32View { esp32Store.device = $0 }
                }
        }
    }

    private var homePage: some View {
        VStack {
            controllerConnectionSection
            Button("Provision device") { isProvisioningDevice = true }
            Button("Select ESP32") { isSelectingESP32 = true }
            if let device = esp32Store.device {
                Text("Selected: \(device.name)")
                    .foregroundStyle(.secondary)
            }
        }
        .padding(.horizontal)
        .buttonStyle(.appPrimary)
    }

    private var controllerConnectionSection: some View {
        HStack {
            Image(systemName: "wave.3.up.circle.fill")
                .font(.system(size: 32))
            
            VStack(spacing: 8) {
                connectionStatus
                connectionActions
            }
        }
        .padding(8)
        .background(Color.cardBackground)
        .border(Color.cardBorder, width: 1)
    }

    @ViewBuilder
    private var connectionStatus: some View {
        switch connection.state {
        case .disconnected:
            Label("Not connected", systemImage: "wifi.slash")
                .foregroundStyle(.secondary)
        case .connecting(let name):
            Label("Connecting to \(name)…", systemImage: "ellipsis.circle")
                .foregroundStyle(.secondary)
        case .connected(let name):
            Label("Connected to \(name)", systemImage: "checkmark.circle.fill")
                .foregroundStyle(.green)
        case .failed(let name):
            Label("Failed to connect to \(name)", systemImage: "xmark.circle.fill")
                .foregroundStyle(.red)
        }
    }

    @ViewBuilder
    private var connectionActions: some View {
        switch connection.state {
        case .disconnected, .failed:
            Button("Select controller") { isSelectingController = true }
        case .connecting:
            Button("Cancel", role: .cancel) { connection.disconnect() }
        case .connected:
            HStack {
                Button("Disconnect", role: .destructive) { connection.disconnect() }
            }
        }
    }
}

#Preview {
    MiddleTab()
        .environment(SelectedESP32Store())
}

#Preview("Connection failed") {
    MiddleTab()
        .environment(SelectedESP32Store())
        .environment(\.controllerConnection, PreviewControllerConnection(state: .failed(name: "Living Room")))
}

