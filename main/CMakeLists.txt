#c语言开发
idf_component_register(SRCS "esp32-ip-to-geolocation.c"
"0014-espressif_esp32_send_data.cpp"
                    INCLUDE_DIRS "." "../src" "../src/managed_components/bblanchon__arduinojson" "../src/managed_components/bblanchon__arduinojson/src")

#c++语言开发
#idf_component_register(SRCS "main.cpp"
#                    INCLUDE_DIRS ".")
