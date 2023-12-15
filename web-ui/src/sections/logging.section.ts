import {Api, ApiEventName} from "../api";

let section: HTMLDivElement;
let toggleFcUsesMainSerialBtn: HTMLButtonElement;
let toggleFcUsesMainSerialBtnText: HTMLSpanElement;

function loadElements() {
    section = document.querySelector('section#logging-section') as HTMLDivElement;
    toggleFcUsesMainSerialBtn = section.querySelector('button#toggle-fc-uses-main-serial') as HTMLButtonElement;
    toggleFcUsesMainSerialBtnText = toggleFcUsesMainSerialBtn.querySelector('span') as HTMLSpanElement;
}

let fcUsesMainSerial = false;
function updateState() {
    if (fcUsesMainSerial) {
        toggleFcUsesMainSerialBtnText.innerText = 'Enable Logging on Main Serial';
    } else {
        toggleFcUsesMainSerialBtnText.innerText = 'Disable Logging on Main Serial';
    }
}

export function setupLoggingSection() {
    loadElements();

    toggleFcUsesMainSerialBtn.addEventListener('click', async () => {
        const response = await Api.toggleFcUsesMainSerial();
        if (response.ok) {
            const data = await response.json();
            fcUsesMainSerial = data.fcUsesMainSerial;
            updateState();
        }
    });

    Api.on(ApiEventName.serial, (data) => {
        if (data.serial.fcSerialUsesMainSerial === undefined) {
            return;
        }

        fcUsesMainSerial = data.serial.fcSerialUsesMainSerial;
    });
}