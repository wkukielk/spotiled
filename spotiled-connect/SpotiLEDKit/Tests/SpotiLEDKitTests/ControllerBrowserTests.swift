//
//  DeviceBrowserTests.swift
//  SpotiLEDKitTests
//
//  Created by wkuki on 19/07/2026.
//

import Testing
import Mockable
import Network
import dnssd
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

    @Test func startsInSearchingState() {
        let discovery = MockDeviceDiscovering(policy: .relaxed)
        let browser = DeviceBrowser(discovery: discovery)

        #expect(browser.state == .searching)
    }

    @Test func publishesStateReportedByDiscovery() {
        let discovery = MockDeviceDiscovering(policy: .relaxed)
        var capturedHandler: ((DeviceDiscoveryState) -> Void)?
        given(discovery).onStateChanged(.any).willProduce { handler in
            capturedHandler = handler
        }

        let browser = DeviceBrowser(discovery: discovery)
        capturedHandler?(.localNetworkPermissionDenied)

        #expect(browser.state == .localNetworkPermissionDenied)
    }

    @Test func startResetsStateToSearching() {
        let discovery = MockDeviceDiscovering(policy: .relaxed)
        var capturedHandler: ((DeviceDiscoveryState) -> Void)?
        given(discovery).onStateChanged(.any).willProduce { handler in
            capturedHandler = handler
        }

        let browser = DeviceBrowser(discovery: discovery)
        capturedHandler?(.failed("boom"))
        browser.start()

        #expect(browser.state == .searching)
    }

    @Test func mapsNoAuthDNSFailureToLocalNetworkPermissionDenied() {
        let error = NWError.dns(DNSServiceErrorType(kDNSServiceErr_NoAuth))

        #expect(BonjourServiceDiscovery.discoveryState(from: .failed(error)) == .localNetworkPermissionDenied)
        #expect(BonjourServiceDiscovery.discoveryState(from: .waiting(error)) == .localNetworkPermissionDenied)
    }

    @Test func mapsPolicyDeniedDNSFailureToLocalNetworkPermissionDenied() {
        let error = NWError.dns(DNSServiceErrorType(kDNSServiceErr_PolicyDenied))

        #expect(BonjourServiceDiscovery.discoveryState(from: .failed(error)) == .localNetworkPermissionDenied)
        #expect(BonjourServiceDiscovery.discoveryState(from: .waiting(error)) == .localNetworkPermissionDenied)
    }

    @Test func mapsOtherFailuresToFailedWithMessage() {
        let error = NWError.posix(.ECONNREFUSED)

        #expect(BonjourServiceDiscovery.discoveryState(from: .failed(error)) == .failed(error.debugDescription))
    }

    @Test func mapsSetupReadyAndCancelledToSearching() {
        #expect(BonjourServiceDiscovery.discoveryState(from: .setup) == .searching)
        #expect(BonjourServiceDiscovery.discoveryState(from: .ready) == .searching)
        #expect(BonjourServiceDiscovery.discoveryState(from: .cancelled) == .searching)
    }
}
