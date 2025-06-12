Import("env")

# Werte aus platformio.ini lesen
username = env.GetProjectOption("custom_username")
password = env.GetProjectOption("custom_password")
ssid = env.GetProjectOption("custom_wifi_ssid")

print("Username:", username)
print("Password:", password)
print("SSID:", ssid)


# Als Build-Flags für C++ hinzufügen
env.Append(
    CPPDEFINES=[
        ("OTA_USERNAME", '\\"%s\\"' % username),
        ("OTA_PASSWORD", '\\"%s\\"' % password),
        ("WIFI_HOSTNAME", '\\"%s\\"' % ssid)
    ]
)
