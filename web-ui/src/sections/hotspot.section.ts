import { Api, ApiEventName } from "../api";

let section: HTMLDivElement;

let saveCredentialsForm: HTMLFormElement;
let saveCredentialsSubmitButton: HTMLButtonElement;
let ssidInput: HTMLInputElement;
let passwordInput: HTMLInputElement;
let enableHotspotButton: HTMLButtonElement;

function loadElements() {
  section = document.querySelector("section#hotspot") as HTMLDivElement;

  saveCredentialsForm = section.querySelector('#hotspot-form') as HTMLFormElement;
  saveCredentialsSubmitButton = saveCredentialsForm.querySelector('button[type="submit"]') as HTMLButtonElement;

  ssidInput = saveCredentialsForm.querySelector('input[name="ssid"]') as HTMLInputElement;
  passwordInput = saveCredentialsForm.querySelector('input[name="password"]') as HTMLInputElement;

  enableHotspotButton = section.querySelector('#hotspot-enable-btn') as HTMLButtonElement;
}

function setupCredentialsForm() {
  saveCredentialsForm.addEventListener('submit', async (e) => {
    e.preventDefault();

    const formData = new FormData(saveCredentialsForm);
    const ssid = formData.get('ssid') as string;
    const password = formData.get('password') as string;

    saveCredentialsSubmitButton.setAttribute('aria-busy', 'true');
    await Api.setHotspotCredentials(ssid, password);
    saveCredentialsSubmitButton.setAttribute('aria-busy', 'false');
  });
}

function updateCurrentCredentials(ssid: string, password: string) {
  ssidInput.value = ssid;
  passwordInput.value = password;
}

export function setupHotspotSection() {
  loadElements();

  Api.on(ApiEventName.hotspot, (data) => {
    updateCurrentCredentials(
      data.hotspot.credentials.ssid,
      data.hotspot.credentials.password
    );

    if (data.hotspot.isEnabled) {
      enableHotspotButton.hidden = true;
    } else {
      enableHotspotButton.hidden = false;
    }
  });

  setupCredentialsForm();

  enableHotspotButton.addEventListener('click', async () => {
    enableHotspotButton.setAttribute('aria-busy', 'true');
    await Api.enableHotspot();
    enableHotspotButton.setAttribute('aria-busy', 'false');
  });
}
