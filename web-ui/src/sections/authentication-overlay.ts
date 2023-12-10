import { ApiPinCode, Api, ApiEventName } from "../api";

let dialog: HTMLDialogElement;
let form: HTMLFormElement;
let submitButton: HTMLButtonElement;
let requestPinButton: HTMLButtonElement;
let errorElement: HTMLQuoteElement;
let pinInput: HTMLInputElement;

function loadElements() {
  dialog = document.querySelector(
    "dialog#authentication-dialog"
  ) as HTMLDialogElement;

  form = dialog.querySelector("form") as HTMLFormElement;
  submitButton = form.querySelector(
    "button[type='submit']"
  ) as HTMLButtonElement;
  requestPinButton = form.querySelector(
    "button[type='button']"
  ) as HTMLButtonElement;
  errorElement = form.querySelector("blockquote") as HTMLQuoteElement;
  submitButton.setAttribute("aria-invalid", "true");
  pinInput = form.querySelector("input[name='pin']") as HTMLInputElement;
}

function showError(msg: string) {
  errorElement.textContent = msg;
  errorElement.hidden = false;
  pinInput.setAttribute("aria-invalid", "true");
}

export function setupAuthenticationOverlay() {
  loadElements();

  dialog.addEventListener("cancel", (e) => {
    if (!Api.isAuthenticated) {
      e.preventDefault();
    }
  });

  Api.on(ApiEventName.is_authenticated, (data) => {
    if (data.apiIsAuthenticated) {
      dialog.close();
    } else {
      dialog.showModal();
    }
  });

  if (!Api.isAuthenticated) {
    dialog.showModal();
  }

  form.addEventListener("submit", async (e) => {
    e.preventDefault();
    const formData = new FormData(form);
    const pin = formData.get("pin") as ApiPinCode;

    submitButton.setAttribute("aria-busy", "true");
    const result = await Api.authenticate(pin);
    submitButton.setAttribute("aria-busy", "false");

    if (result !== true) {
      showError(result);
    }
  });

  requestPinButton.addEventListener("click", async () => {
    requestPinButton.setAttribute("aria-busy", "true");
    await Api.requestPinCode();
    requestPinButton.setAttribute("aria-busy", "false");

    pinInput.value = "";
    pinInput.setAttribute("aria-invalid", "");
    errorElement.hidden = true;
    pinInput.focus();
  });
}
