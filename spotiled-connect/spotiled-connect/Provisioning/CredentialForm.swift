//
//  CredentialForm.swift
//  spotiled-connect
//

import SwiftUI
import SpotiLEDProvisioning

struct CredentialForm: View {
    let device: BLEDevice
    let errorMessage: String?
    @Binding var ssid: String
    @Binding var password: String
    let onSubmit: () -> Void
    let onChooseDifferentDevice: () -> Void

    var body: some View {
        VStack(spacing: 16) {
            Text(device.id)
                .foregroundStyle(.secondary)
            if let errorMessage {
                Text(errorMessage)
                    .foregroundStyle(.red)
            }
            TextField("WiFi network name", text: $ssid)
                .textFieldStyle(.roundedBorder)
                .textInputAutocapitalization(.never)
                .autocorrectionDisabled()
            SecureField("WiFi password", text: $password)
                .textFieldStyle(.roundedBorder)
            Button("Provision", action: onSubmit)
                .disabled(ssid.isEmpty)
            Button("Choose a different device", role: .cancel, action: onChooseDifferentDevice)
        }
        .padding()
        .buttonStyle(.appPrimary)
    }
}

#Preview {
    CredentialForm(
        device: .preview(id: "PROV_AABBCC"),
        errorMessage: nil,
        ssid: .constant(""),
        password: .constant(""),
        onSubmit: {},
        onChooseDifferentDevice: {}
    )
}

#Preview("With error") {
    CredentialForm(
        device: .preview(id: "PROV_AABBCC"),
        errorMessage: "Failed to connect",
        ssid: .constant("MyWiFi"),
        password: .constant(""),
        onSubmit: {},
        onChooseDifferentDevice: {}
    )
}
