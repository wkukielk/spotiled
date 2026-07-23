//
//  MiddleTab.swift
//  spotiled-connect
//
//  Created by wkuki on 23/07/2026.
//

import SwiftUI


struct MiddleTab: View {
    @State private var isProvisioningDevice = false
    @State private var isSelectingController = false

    var body: some View {
        NavigationStack {
            homePage
                .navigationDestination(isPresented: $isProvisioningDevice) {
                    ProvisionDeviceView()
                }
                .navigationDestination(isPresented: $isSelectingController) {
                    SelectControllerView()
                }
        }
    }

    private var homePage: some View {
        VStack {
            Image(systemName: "globe")
                .imageScale(.large)
                .foregroundStyle(.tint)
            Text("Hello, world!")
            Button("Provision device") { isProvisioningDevice = true }
            Button("Select ESP32") {}
            Button("Select controller") { isSelectingController = true }
        }
        .padding(.horizontal)
        .buttonStyle(.appPrimary)
    }
}
