Import("env")

# Werte aus platformio.ini lesen
username = env.GetProjectOption("custom_username")
password = env.GetProjectOption("custom_password")
ssid = env.GetProjectOption("custom_wifi_ssid")

# Als Build-Flags für C++ hinzufügen
env.Append(
    CPPDEFINES=[
        ("DOTA_USERNAME", '"%s"' % username),
        ("DOTA_PASSWORD", '"%s"' % password),
        ("WIFI_SSID", '"%s"' % ssid)
    ]
)
