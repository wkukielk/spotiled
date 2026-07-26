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
    @State private var controllerConnection = WebSocketControllerConnection()

    var body: some Scene {
        WindowGroup {
            ContentView()
                .environment(\.controllerConnection, controllerConnection)
        }
    }
}
