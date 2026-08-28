//
//  BLEProvisioningTests.swift
//  SpotiLEDProvisioningTests
//

import Testing
import Mockable
@testable import SpotiLEDProvisioning

@MainActor
struct BLEProvisioningTests {

    @Test func provisionDrivesStateThroughConnectingProvisioningSuccess() async throws {
        let controller = MockESPBLEDeviceControlling(policy: .relaxed)
        let sut = ESPBLEProvisioning()
        let device = BLEDevice(id: "PROV_ABC123", controller: controller)

        try await sut.provision(device: device, ssid: "MyWifi", password: "secret")

        #expect(sut.state == .success)
    }

    @Test func provisionSetsConnectingStateBeforeConnectCompletes() async throws {
        let controller = MockESPBLEDeviceControlling(policy: .relaxed)
        let sut = ESPBLEProvisioning()
        let device = BLEDevice(id: "PROV_ABC123", controller: controller)
        var stateWhileConnecting: ProvisioningState?
        given(controller).connect(delegate: .any).willProduce { _ in
            stateWhileConnecting = sut.state
        }

        try await sut.provision(device: device, ssid: "MyWifi", password: "secret")

        #expect(stateWhileConnecting == .connecting("PROV_ABC123"))
    }

    @Test func provisionSetsProvisioningStateAfterConnectCompletes() async throws {
        let controller = MockESPBLEDeviceControlling(policy: .relaxed)
        let sut = ESPBLEProvisioning()
        let device = BLEDevice(id: "PROV_ABC123", controller: controller)
        var stateWhileProvisioning: ProvisioningState?
        given(controller).provision(ssid: .any, passPhrase: .any).willProduce { _, _ in
            stateWhileProvisioning = sut.state
        }

        try await sut.provision(device: device, ssid: "MyWifi", password: "secret")

        #expect(stateWhileProvisioning == .provisioning("PROV_ABC123"))
    }

    @Test func provisionThrowsAndSetsFailedStateWhenConnectFails() async {
        let controller = MockESPBLEDeviceControlling(policy: .relaxed)
        given(controller).connect(delegate: .any).willThrow(ProvisioningError.disconnected)
        let sut = ESPBLEProvisioning()
        let device = BLEDevice(id: "PROV_ABC123", controller: controller)

        await #expect(throws: ProvisioningError.disconnected) {
            try await sut.provision(device: device, ssid: "MyWifi", password: "secret")
        }
        guard case .failed = sut.state else {
            Issue.record("expected .failed state, got \(sut.state)")
            return
        }
    }

    @Test func provisionThrowsAndSetsFailedStateWhenProvisionFails() async {
        let controller = MockESPBLEDeviceControlling(policy: .relaxed)
        given(controller).provision(ssid: .any, passPhrase: .any).willThrow(ProvisioningError.disconnected)
        let sut = ESPBLEProvisioning()
        let device = BLEDevice(id: "PROV_ABC123", controller: controller)

        await #expect(throws: ProvisioningError.disconnected) {
            try await sut.provision(device: device, ssid: "MyWifi", password: "secret")
        }
        guard case .failed = sut.state else {
            Issue.record("expected .failed state, got \(sut.state)")
            return
        }
    }

    @Test func provisionDoesNotCallProvisionBeforeConnectSucceeds() async {
        let controller = MockESPBLEDeviceControlling(policy: .relaxed)
        given(controller).connect(delegate: .any).willThrow(ProvisioningError.disconnected)
        let sut = ESPBLEProvisioning()
        let device = BLEDevice(id: "PROV_ABC123", controller: controller)

        _ = try? await sut.provision(device: device, ssid: "MyWifi", password: "secret")

        verify(controller).provision(ssid: .any, passPhrase: .any).called(.never)
    }
}
