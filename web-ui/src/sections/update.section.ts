import { Api } from "../api";

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

        updateButton.setAttribute("aria-busy", "true");
        const response = await Api.uploadFirmware(firmware);
        updateButton.removeAttribute("aria-busy");
    });
}