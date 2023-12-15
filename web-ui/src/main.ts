import "@picocss/pico/css/pico.min.css";
import "./style.css";
import { setupAuthenticationOverlay } from "./sections/authentication-overlay";
import { setupWiFiSection } from "./sections/wifi.section";
import { Api } from "./api";
import { setupRotorHazardSection } from "./sections/rh.section";
import { setupHotspotSection } from "./sections/hotspot.section";
import { setupOTASection } from "./sections/ota.section";
import { setupLoggingSection } from "./sections/logging.section";

document.addEventListener("DOMContentLoaded", async () => {
  setupAuthenticationOverlay();
  setupRotorHazardSection();
  setupHotspotSection();
  setupWiFiSection();
  setupOTASection();
  setupLoggingSection();

  Api.getAllData();
  Api.authenticate();

  const rebootEspButton = document.getElementById(
    "system-reboot"
  ) as HTMLButtonElement;
  rebootEspButton.addEventListener("click", async () => {
    if (confirm("Are you sure you want to reboot the ESP?")) {
      await Api.reboot();
    }
  });
});
