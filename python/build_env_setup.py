import esp_compress
Import("env", "projenv")

target_name = env['PIOENV'].upper()

if "_OTA" in target_name:
    env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", esp_compress.compressFirmware)