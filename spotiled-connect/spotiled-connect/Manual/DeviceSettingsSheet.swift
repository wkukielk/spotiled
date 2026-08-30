//
//  DeviceSettingsSheet.swift
//  spotiled-connect
//
//  Created by wkuki on 30/08/2026.
//

import SwiftUI
import SpotiLEDKit

struct DeviceSettingsSheet: View {
    let connection: any DirectESP32Connecting
    let esp32Store: SelectedESP32Store
    @Environment(\.dismiss) private var dismiss

    var body: some View {
        NavigationStack {
            VStack(spacing: 16) {
                RenameDevicePanel(connection: connection, esp32Store: esp32Store)
                ReprovisionPanel(connection: connection, esp32Store: esp32Store)
                Spacer()
            }
            .padding()
            .navigationTitle("Device settings")
            .toolbar {
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") { dismiss() }
                }
            }
        }
    }
}

private struct RenameDevicePanel: View {
    let connection: any DirectESP32Connecting
    let esp32Store: SelectedESP32Store
    @State private var name = ""

    private var isConnected: Bool {
        if case .connected = connection.state { return true }
        return false
    }

    var body: some View {
        VStack(spacing: 8) {
            TextField("Device name", text: $name)
                .textFieldStyle(.roundedBorder)
            Button("Rename device") {
                Task {
                    try? await connection.rename(to: name)
                    esp32Store.device = nil
                }
            }
            .disabled(!isConnected || name.trimmingCharacters(in: .whitespaces).isEmpty)
            .buttonStyle(.appPrimary)
        }
        .padding(8)
        .background(Color.cardBackground)
        .border(Color.cardBorder, width: 1)
        .onAppear {
            name = esp32Store.device?.name ?? ""
        }
    }
}

private struct ReprovisionPanel: View {
    let connection: any DirectESP32Connecting
    let esp32Store: SelectedESP32Store
    @State private var showConfirmation = false

    private var isConnected: Bool {
        if case .connected = connection.state { return true }
        return false
    }

    var body: some View {
        VStack(spacing: 8) {
            Button("Reprovision device", role: .destructive) {
                showConfirmation = true
            }
            .disabled(!isConnected)
            .buttonStyle(.appPrimary)
        }
        .padding(8)
        .background(Color.cardBackground)
        .border(Color.cardBorder, width: 1)
        .confirmationDialog(
            "Reprovision this device?",
            isPresented: $showConfirmation,
            titleVisibility: .visible
        ) {
            Button("Reprovision", role: .destructive) {
                Task {
                    try? await connection.reprovision()
                    esp32Store.device = nil
                }
            }
            Button("Cancel", role: .cancel) {}
        } message: {
            Text("The device will disconnect from WiFi and restart in pairing mode. You'll need to provision it again to reconnect.")
        }
    }
}
