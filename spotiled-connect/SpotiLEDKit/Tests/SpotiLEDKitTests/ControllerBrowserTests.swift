//
//  DeviceBrowserTests.swift
//  SpotiLEDKitTests
//
//  Created by wkuki on 19/07/2026.
//

import Testing
import Mockable
@testable import SpotiLEDKit

struct DeviceBrowserTests {

    @Test func startDelegatesToDiscovery() {
        let discovery = MockDeviceDiscovering(policy: .relaxed)
        let browser = DeviceBrowser(discovery: discovery)

        browser.start()

        verify(discovery).start().called(.once)
    }

    @Test func stopDelegatesToDiscovery() {
        let discovery = MockDeviceDiscovering(policy: .relaxed)
        let browser = DeviceBrowser(discovery: discovery)

        browser.stop()

        verify(discovery).cancel().called(.once)
    }

    @Test func publishesNamesReportedByDiscovery() {
        let discovery = MockDeviceDiscovering(policy: .relaxed)
        var capturedHandler: (([DiscoveredDevice]) -> Void)?
        given(discovery).onDevicesChanged(.any).willProduce { handler in
            capturedHandler = handler
        }

        let browser = DeviceBrowser(discovery: discovery)
        capturedHandler?([
            DiscoveredDevice(name: "Alpha", endpoint: .hostPort(host: "192.168.1.10", port: 80)),
            DiscoveredDevice(name: "Zeta", endpoint: .hostPort(host: "192.168.1.11", port: 80))
        ])

        #expect(browser.discoveredNames == ["Alpha", "Zeta"])
    }

    @Test func startsWithNoDiscoveredNames() {
        let discovery = MockDeviceDiscovering(policy: .relaxed)
        let browser = DeviceBrowser(discovery: discovery)

        #expect(browser.discoveredNames.isEmpty)
    }
}
