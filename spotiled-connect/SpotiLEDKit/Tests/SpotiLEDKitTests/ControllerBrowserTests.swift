//
//  ControllerBrowserTests.swift
//  SpotiLEDKitTests
//
//  Created by wkuki on 19/07/2026.
//

import Testing
import Mockable
@testable import SpotiLEDKit

struct ControllerBrowserTests {

    @Test func startDelegatesToDiscovery() {
        let discovery = MockControllerDiscovering(policy: .relaxed)
        let browser = ControllerBrowser(discovery: discovery)

        browser.start()

        verify(discovery).start().called(.once)
    }

    @Test func stopDelegatesToDiscovery() {
        let discovery = MockControllerDiscovering(policy: .relaxed)
        let browser = ControllerBrowser(discovery: discovery)

        browser.stop()

        verify(discovery).cancel().called(.once)
    }

    @Test func publishesNamesReportedByDiscovery() {
        let discovery = MockControllerDiscovering(policy: .relaxed)
        var capturedHandler: (([String]) -> Void)?
        given(discovery).onNamesChanged(.any).willProduce { handler in
            capturedHandler = handler
        }

        let browser = ControllerBrowser(discovery: discovery)
        capturedHandler?(["Alpha", "Zeta"])

        #expect(browser.discoveredNames == ["Alpha", "Zeta"])
    }

    @Test func startsWithNoDiscoveredNames() {
        let discovery = MockControllerDiscovering(policy: .relaxed)
        let browser = ControllerBrowser(discovery: discovery)

        #expect(browser.discoveredNames.isEmpty)
    }
}
