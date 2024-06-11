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
from pathlib import Path
import os

@pytest.mark.parametrize('config', [
    'build_esp32_release-v5.0',  # 添加更多配置路径以支持多个测试用例
], indirect=True)
@pytest.mark.supported_targets("esp32")  # Specify the target, esp32 in this case
def test_esp32_ip_to_geolocation(dut: Dut, config):
    build_path = config
    flasher_args_path = Path(build_path, 'flasher_args.json')
    sdkconfig_path = Path(build_path, 'config/sdkconfig.json')

    # Check if the paths exist
    assert flasher_args_path.exists(), f"{flasher_args_path} does not exist"
    assert sdkconfig_path.exists(), f"{sdkconfig_path} does not exist"

    # Existing test logic
    print(f"flasher_args.json found at: {flasher_args_path}")

    # Start the test
    dut.expect_exact("wifi_init_sta finished.")
    dut.expect("connected to ap SSID:Wokwi-GUEST")

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
        dut.expect(key)

    # Optionally, check for completion of the HTTP task or any other specific logs
    dut.expect("HTTP request completed successfully")  # Modify based on actual log message on success
