//
//  spotiled_connectApp.swift
//  spotiled-connect
//
//  Created by wkuki on 19/07/2026.
//

import SwiftUI
import SpotiLEDKit

@main
struct spotiled_connectApp: App {
    @State private var esp32Session = ControllerESP32Session()
    @State private var selectedESP32Store = SelectedESP32Store()

    var body: some Scene {
        WindowGroup {
            ContentView()
                .environment(\.esp32Session, esp32Session)
                .environment(selectedESP32Store)
        }
    }
}
