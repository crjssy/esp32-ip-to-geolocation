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

def test_esp32_ip_to_geolocation(dut: Dut):
    try:
        # 增加超时时间到 60 秒
        dut.expect("HTTP GET Status = 200, content_length = ", timeout=60)
    except Exception as e:
        # 捕获超时异常并打印日志
        print("Test timed out or failed with exception: ", str(e))
        print("Full log:")
        print(dut.pexpect_proc.before.decode())
        print("Test incomplete, proceeding with the next steps.")
        # 继续进行后续步骤
        pytest.fail("Test incomplete due to timeout or other issue")


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
