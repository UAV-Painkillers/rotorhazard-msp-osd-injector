import { ApiEventName, Api } from "../api";

let section: HTMLDivElement;

let switchToWifiButton: HTMLButtonElement;

let saveCredentialsForm: HTMLFormElement;
let saveCredentialsFormSSIDInput: HTMLInputElement;
let saveCredentialsSubmitButton: HTMLButtonElement;

function loadElements() {
  section = document.querySelector("section#wifi") as HTMLDivElement;

  switchToWifiButton = section.querySelector(
    "#wifi-switch-to-wifi"
  ) as HTMLButtonElement;

  saveCredentialsForm = section.querySelector('#wifi-form') as HTMLFormElement;
  saveCredentialsFormSSIDInput = saveCredentialsForm.querySelector('input[name="ssid"]') as HTMLInputElement;
  saveCredentialsSubmitButton = saveCredentialsForm.querySelector('button[type="submit"]') as HTMLButtonElement;
}

function updateswitchToWifiButton(
  hotspotIsEnabled: boolean,
  storedSSID: string,
) {
    if (hotspotIsEnabled && !!storedSSID) {
      switchToWifiButton.hidden = false;
    } else {
      switchToWifiButton.hidden = true;
    }

    switchToWifiButton.addEventListener('click', async () => {
      switchToWifiButton.setAttribute('aria-busy', 'true');
      await Api.connectToWiFi();
      switchToWifiButton.setAttribute('aria-busy', 'false');
    });
}

function setupSaveCredentialsForm() {
  saveCredentialsForm.addEventListener('submit', async (e) => {
    e.preventDefault();

    const formData = new FormData(saveCredentialsForm);
    const ssid = formData.get('ssid') as string;
    const password = formData.get('password') as string;

    saveCredentialsSubmitButton.setAttribute('aria-busy', 'true');
    await Api.setWifiCredentials(ssid, password);
    saveCredentialsSubmitButton.setAttribute('aria-busy', 'false');
  });
}

export function setupWiFiSection() {
  loadElements();

  let storedSSID = '';
  let hotspotIsEnabled = true;

  Api.on(ApiEventName.hotspot, (data) => {
    hotspotIsEnabled = data.hotspot.isEnabled;
    updateswitchToWifiButton(hotspotIsEnabled, storedSSID);
  });

  Api.on(ApiEventName.wifi, (data) => {
    storedSSID = data.wifi.storedSSID;

    updateswitchToWifiButton(hotspotIsEnabled, storedSSID);
    saveCredentialsFormSSIDInput.value = storedSSID;
  });

  setupSaveCredentialsForm();
}
