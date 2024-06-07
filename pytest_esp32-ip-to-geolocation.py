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
    print("Checking: wifi_init_sta finished.")
    dut.expect_exact("wifi_init_sta finished.")
    print("Found: wifi_init_sta finished.")
    
    print("Checking: connected to ap SSID:Wokwi-GUEST")
    dut.expect("connected to ap SSID:Wokwi-GUEST")
    print("Found: connected to ap SSID:Wokwi-GUEST")

    # Check for a successful HTTP request
    dut.expect("HTTP GET Status = 200, content_length = ")

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
        "timezone:",
        "isp:",
        "org:",
        "as:",
        "query:"
    ]

    # Check each expected log entry for presence only, not specific content
    for key in expected_keys:
        print(f"Checking: {key}")
        try:
            dut.expect(key)
            print(f"Found: {key}")
        except pexpect.exceptions.TIMEOUT:
            print(f"Failed to find: {key}")
            # Dump the buffer for debugging
            print(dut.pexpect_proc.before.decode('utf-8', 'ignore'))
            raise

    # Optionally, check for completion of the HTTP task or any other specific logs
    print("Checking: HTTP request completed successfully")
    dut.expect("HTTP request completed successfully")
    print("Found: HTTP request completed successfully")  # Modify based on actual log message on success

