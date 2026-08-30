//
//  ProvisionDeviceView.swift
//  spotiled-connect
//
//  Created by wkuki on 19/07/2026.
//

import SwiftUI
import SpotiLEDProvisioning

struct ProvisionDeviceView<Provisioning: DeviceProvisioning & DeviceScanning>: View
where Provisioning.Device == BLEDevice {
    @State private var provisioning: Provisioning
    @State private var selectedDevice: BLEDevice?
    @State private var ssid = ""
    @State private var password = ""
    @Environment(\.dismiss) private var dismiss

    init(provisioning: Provisioning) {
        _provisioning = State(initialValue: provisioning)
    }

    var body: some View {
        content
    }

    @ViewBuilder
    private var content: some View {
        if let selectedDevice {
            deviceFlow(for: selectedDevice)
                .navigationTitle("Provision device")
        } else {
            DiscoveredDeviceListView(
                scanner: provisioning,
                title: "Provision device",
                emptyMessage: "Searching for devices…",
                emptySystemImage: "dot.radiowaves.left.and.right"
            ) { device in
                Button {
                    selectedDevice = device
                } label: {
                    Text(device.id)
                }
            }
        }
    }

    @ViewBuilder
    private func deviceFlow(for device: BLEDevice) -> some View {
        switch provisioning.state {
        case .success:
            successView
        case .connecting, .provisioning:
            progressView
        case .failed:
            failureView
        case .idle, .scanning:
            credentialForm(for: device)
        }
    }

    private func credentialForm(for device: BLEDevice) -> some View {
        CredentialForm(
            device: device,
            ssid: $ssid,
            password: $password,
            onSubmit: { submit(device: device) },
            onChooseDifferentDevice: { selectedDevice = nil }
        )
    }

    private var failureView: some View {
        VStack(spacing: 12) {
            Image(systemName: "wifi.exclamationmark")
                .font(.system(size: 48))
                .foregroundStyle(.red)
            Text("Couldn't connect to Wi-Fi")
            Text("Double-check the network name and password, then try again.")
                .font(.footnote)
                .foregroundStyle(.secondary)
                .multilineTextAlignment(.center)
            Button("Try Again") { provisioning.resetState() }
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
