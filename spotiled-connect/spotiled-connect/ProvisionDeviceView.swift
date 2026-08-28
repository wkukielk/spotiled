//
//  ProvisionDeviceView.swift
//  spotiled-connect
//
//  Created by wkuki on 19/07/2026.
//

import SwiftUI
import SpotiLEDProvisioning

struct ProvisionDeviceView: View {
    @State private var provisioning: any DeviceProvisioning
    @State private var selectedDevice: BLEDevice?
    @State private var ssid = ""
    @State private var password = ""
    @Environment(\.dismiss) private var dismiss

    @MainActor
    init(provisioning: (any DeviceProvisioning)? = nil) {
        _provisioning = State(initialValue: provisioning ?? ESPBLEProvisioning())
    }

    var body: some View {
        content
            .navigationTitle("Provision device")
            .task { provisioning.startScan() }
            .onDisappear { provisioning.stopScan() }
    }

    @ViewBuilder
    private var content: some View {
        if let selectedDevice {
            deviceFlow(for: selectedDevice)
        } else {
            deviceList
        }
    }

    @ViewBuilder
    private var deviceList: some View {
        if provisioning.devices.isEmpty {
            scanEmptyStateView
        } else {
            List(provisioning.devices) { device in
                Button {
                    selectedDevice = device
                } label: {
                    Text(device.id)
                }
            }
        }
    }

    @ViewBuilder
    private var scanEmptyStateView: some View {
        switch provisioning.state {
        case .failed(let message):
            ContentUnavailableView {
                Label("Search Failed", systemImage: "exclamationmark.triangle")
            } description: {
                Text(message)
            } actions: {
                Button("Retry") { provisioning.startScan() }
            }
        default:
            ContentUnavailableView("Searching for devices…", systemImage: "dot.radiowaves.left.and.right")
        }
    }

    @ViewBuilder
    private func deviceFlow(for device: BLEDevice) -> some View {
        switch provisioning.state {
        case .success:
            successView
        case .connecting, .provisioning:
            progressView
        default:
            credentialForm(for: device)
        }
    }

    private func credentialForm(for device: BLEDevice) -> some View {
        VStack(spacing: 16) {
            Text(device.id)
                .foregroundStyle(.secondary)
            if case .failed(let message) = provisioning.state {
                Text(message)
                    .foregroundStyle(.red)
            }
            TextField("WiFi network name", text: $ssid)
                .textFieldStyle(.roundedBorder)
                .textInputAutocapitalization(.never)
                .autocorrectionDisabled()
            SecureField("WiFi password", text: $password)
                .textFieldStyle(.roundedBorder)
            Button("Provision") {
                submit(device: device)
            }
            .disabled(ssid.isEmpty)
            Button("Choose a different device", role: .cancel) {
                selectedDevice = nil
            }
        }
        .padding()
        .buttonStyle(.appPrimary)
    }

    private var progressView: some View {
        VStack(spacing: 12) {
            ProgressView()
            Text(statusText)
                .foregroundStyle(.secondary)
        }
    }

    private var statusText: String {
        switch provisioning.state {
        case .connecting:
            return "Connecting…"
        case .provisioning:
            return "Sending WiFi credentials…"
        default:
            return ""
        }
    }

    private var successView: some View {
        VStack(spacing: 12) {
            Image(systemName: "checkmark.circle.fill")
                .font(.system(size: 48))
                .foregroundStyle(.green)
            Text("Device provisioned")
            Button("Done") { dismiss() }
        }
        .buttonStyle(.appPrimary)
    }

    private func submit(device: BLEDevice) {
        Task {
            try? await provisioning.provision(device: device, ssid: ssid, password: password)
        }
    }
}

#Preview("Device list") {
    NavigationStack {
        ProvisionDeviceView(provisioning: PreviewDeviceProvisioning())
    }
}

#Preview("Provisioning fails") {
    NavigationStack {
        ProvisionDeviceView(provisioning: PreviewDeviceProvisioning(shouldSucceed: false))
    }
}
