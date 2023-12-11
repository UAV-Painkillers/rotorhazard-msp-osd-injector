import { Api, ApiUpdateMode } from "../api";

let section: HTMLDivElement;
let updateForm: HTMLFormElement;
let updateButton: HTMLButtonElement;

function loadElements() {
    section = document.querySelector("section#update") as HTMLDivElement;
    updateForm = section.querySelector("form#update-form") as HTMLFormElement;
    updateButton = updateForm.querySelector("button[type=submit]") as HTMLButtonElement;
}

export function setupUpdateSection() {
    loadElements();

    updateForm.addEventListener("submit", async (event) => {
        event.preventDefault();

        // get the file from the form
        const file = updateForm.querySelector("input[type=file]") as HTMLInputElement;
        const firmware = file.files![0];

        // get the mode from the form select field
        const modeSelect = updateForm.querySelector("select[name=mode]") as HTMLSelectElement;
        const mode = modeSelect.value;

        updateButton.setAttribute("aria-busy", "true");
        const response = await Api.uploadFirmware(firmware, mode as ApiUpdateMode);
        updateButton.removeAttribute("aria-busy");

        // reload on success
        if (response === true) {
            window.location.reload();
        }
    });
}