//
//  MiddleTab.swift
//  spotiled-connect
//
//  Created by wkuki on 23/07/2026.
//

import SwiftUI
import SpotiLEDKit
import SpotiLEDProvisioning
import Network


struct MiddleTab: View {
    @State private var isProvisioningDevice = false
    @State private var isSelectingController = false
    @State private var isSelectingESP32 = false
    @Environment(\.esp32Session) private var session
    @Environment(SelectedESP32Store.self) private var esp32Store

    var body: some View {
        NavigationStack {
            homePage
                .navigationDestination(isPresented: $isProvisioningDevice) {
                    ProvisionDeviceView(provisioning: ESPBLEProvisioning())
                }
                .navigationDestination(isPresented: $isSelectingESP32) {
                    SelectESP32View { esp32Store.device = $0 }
                }
                .navigationDestination(isPresented: $isSelectingController) {
                    SelectControllerView()
                }
        }
    }

    private var homePage: some View {
        VStack {
            controllerConnectionSection
            esp32Section.padding(.bottom, 32)
            Button("Provision device") { isProvisioningDevice = true }
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
        switch session.controllerState {
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
        switch session.controllerState {
        case .disconnected, .failed:
            Button("Select controller") { isSelectingController = true }
        case .connecting:
            Button("Cancel", role: .cancel) { session.disconnect() }
        case .connected:
            HStack {
                Button("Disconnect", role: .destructive) { session.disconnect() }
            }
        }
    }

    private var esp32Section: some View {
        HStack {
            Image(systemName: "cpu")
                .font(.system(size: 32))

            VStack(spacing: 8) {
                SelectedESP32Label(device: esp32Store.device)
                Button(esp32Store.device == nil ? "Select ESP32" : "Change ESP32") {
                    isSelectingESP32 = true
                }
            }
        }
        .padding(8)
        .background(Color.cardBackground)
        .border(Color.cardBorder, width: 1)
    }
}

#Preview("Default") {
    MiddleTab()
        .environment(SelectedESP32Store())
}

#Preview("Connection failed") {
    MiddleTab()
        .environment(SelectedESP32Store())
        .environment(\.esp32Session, PreviewControllerConnection(state: .failed(name: "Living Room")))
}

