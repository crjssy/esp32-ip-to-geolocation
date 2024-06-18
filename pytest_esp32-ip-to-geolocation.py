# SPDX-FileCopyrightText: 2022-2024 Hays Chan
# SPDX-License-Identifier: MIT

'''
Steps to run these cases:
- Build
  - . ${IDF_PATH}/export.sh
  - pip install idf_build_apps
  - python tools/build_apps.py components/button/test_apps -t esp32
- Test
  - pip install -r tools/requirements/requirement.pytest.txt
  - pytest components/button/test_apps --target esp32
'''

import pytest
from pytest_embedded import Dut

@pytest.mark.supported_targets("esp32")
def test_esp32_ip_to_geolocation(dut: Dut):
    # Start the test
    dut.expect_exact("wifi_init_sta finished.", timeout=60)
    dut.expect("connected to ap SSID:Wokwi-GUEST", timeout=60)

    dut.expect("HTTP_EVENT_ON_FINISH", timeout=60)

    # Check for the expected logs from the JSON response
    expected_keys = [
        "status:",
        "country:",
        "countryCode:",
        "region:",
        "regionName:",
        "city:",
        "zip:",
        "lat:",
        "lon:",
        "timezone",
        "isp",
        "org",
        "as",
        "query"
    ]

    # Check each expected log entry for presence only, not specific content
    for key in expected_keys:
        dut.expect(key, timeout=60)
