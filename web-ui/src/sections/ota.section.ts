import { Api, ApiEventName } from "../api";

let otaEnableContainer: HTMLDivElement;
let otaEnableButton: HTMLButtonElement;

let otaDisableContainer: HTMLDivElement;
let otaDisableButton: HTMLButtonElement;

function loadElements() {
    otaEnableContainer = document.getElementById("ota-enable") as HTMLDivElement;
    otaEnableButton = otaEnableContainer.querySelector("button") as HTMLButtonElement;

    otaDisableContainer = document.getElementById("ota-disable") as HTMLDivElement;
    otaDisableButton = otaDisableContainer.querySelector("button") as HTMLButtonElement;
}

export function setupOTASection() {
    loadElements();

    otaDisableContainer.hidden = true;

    otaEnableButton.addEventListener("click", async (e) => {
        e.preventDefault();
        otaEnableButton.setAttribute("aria-busy", "true");
        const result = await Api.enableOTA();
        otaEnableButton.removeAttribute("aria-busy");

        if (result === true) {
            otaEnableContainer.hidden = true;
            otaDisableContainer.hidden = false;
        }
    });

    otaDisableButton.addEventListener("click", async (e) => {
        e.preventDefault();
        otaDisableButton.setAttribute("aria-busy", "true");
        const result = await Api.disableOTA();
        otaDisableButton.removeAttribute("aria-busy");

        if (result === true) {
            otaEnableContainer.hidden = false;
            otaDisableContainer.hidden = true;
        }
    });

    Api.on(ApiEventName.ota, (data) => {
        if (data.ota.isEnabled) {
            otaEnableContainer.hidden = true;
            otaDisableContainer.hidden = false;
        } else {
            otaEnableContainer.hidden = false;
            otaDisableContainer.hidden = true;
        }
    });
}