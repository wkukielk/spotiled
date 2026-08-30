//
//  DeviceScanning.swift
//  spotiled-connect
//
//  Created by wkuki on 30/08/2026.
//

import Observation
import SpotiLEDKit
import SpotiLEDProvisioning
import Foundation

/// Reduces a scanner's own domain-specific state down to what `DiscoveredDeviceListView`
/// needs to render its empty state.
enum DiscoveryPhase: Equatable {
    case searching
    case permissionDenied(title: String, message: String, systemImage: String)
    case failed(String)
}

protocol DeviceScanning: AnyObject, Observable {
    associatedtype Device: Identifiable
    var discoveredDevices: [Device] { get }
    var discoveryPhase: DiscoveryPhase { get }
    func start()
    func stop()
}

extension DeviceBrowser: DeviceScanning {
    var discoveryPhase: DiscoveryPhase {
        switch state {
        case .searching:
            return .searching
        case .localNetworkPermissionDenied:
            return .permissionDenied(
                title: "Local Network Access Needed",
                message: "Allow local network access for SpotiLED Connect in Settings to discover devices.",
                systemImage: "wifi.exclamationmark"
            )
        case .failed(let message):
            return .failed(message)
        }
    }
}

extension ESPBLEProvisioning: DeviceScanning {
    var discoveredDevices: [BLEDevice] { devices }

    var discoveryPhase: DiscoveryPhase {
        switch state {
        case .idle, .scanning, .connecting, .provisioning, .success:
            return .searching
        case .failed(let message):
            return .failed(message)
        }
    }

    func start() { startScan() }
    func stop() { stopScan() }
}
