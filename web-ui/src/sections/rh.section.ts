import { ApiRotorHazardPilot, Api, ApiEventName } from "../api";

let section: HTMLDivElement;

let selectPilotForm: HTMLFormElement;
let selectPilotFormSubmitButton: HTMLButtonElement;
let selectPilotFormSelect: HTMLSelectElement;

let refreshPilotsButton: HTMLButtonElement;

let selectedPilotDisplay: HTMLDivElement;

let connectionForm: HTMLFormElement;
let connectionFormSubmitButton: HTMLButtonElement;

function loadElements() {
  section = document.querySelector("section#rotorhazard") as HTMLDivElement;

  selectPilotForm = section.querySelector('#rh-select-pilot-form') as HTMLFormElement;
  selectPilotFormSubmitButton = selectPilotForm.querySelector('button[type="submit"]') as HTMLButtonElement;
  selectPilotFormSelect = selectPilotForm.querySelector('select') as HTMLSelectElement;

  selectedPilotDisplay = section.querySelector('#rh-selected-pilot-display kbd') as HTMLDivElement;

  connectionForm = section.querySelector('#rh-connection-form') as HTMLFormElement;
  connectionFormSubmitButton = connectionForm.querySelector('button[type="submit"]') as HTMLButtonElement;

  refreshPilotsButton = section.querySelector('#rh-pilots-reload-btn') as HTMLButtonElement;
}

function updateSelectedPilotDisplay(selectedPilot?: ApiRotorHazardPilot) {
  if (!selectedPilot) {
    selectedPilotDisplay.textContent = 'None';
    return;
  }

  selectedPilotDisplay.textContent = `${selectedPilot.name} (${selectedPilot.callsign})`;
}

function setupSelectPilotForm() {
  selectPilotForm.addEventListener('submit', async (event) => {
    event.preventDefault();

    const data = new FormData(selectPilotForm);
    const pilotId = data.get('pilotId');

    if (!pilotId) {
      return;
    }

    selectPilotFormSubmitButton.setAttribute('aria-busy', 'true');
    await Api.setRotorHazardPilotId(parseInt(pilotId as string));
    selectPilotFormSubmitButton.setAttribute('aria-busy', 'false');
  });

  refreshPilotsButton.addEventListener('click', async () => {
    refreshPilotsButton.setAttribute('aria-busy', 'true');
    await fetchAndDisplayPilots();
    refreshPilotsButton.setAttribute('aria-busy', 'false');
  });
}

function populateSelectPilotSelectInput(pilots: ApiRotorHazardPilot[]) {
  // clear existing options
  selectPilotFormSelect.innerHTML = '';

  // add new options
  pilots.forEach((pilot) => {
    const option = document.createElement('option');
    option.value = pilot.id.toString();
    option.textContent = `${pilot.name} (${pilot.callsign})`;

    selectPilotFormSelect.appendChild(option);
  });
}

function setupConnectionForm() {
  connectionForm.addEventListener('submit', async (event) => {
    event.preventDefault();

    const data = new FormData(connectionForm);
    const hostname = data.get('hostname') as string;
    const port = data.get('port') as string ?? undefined;
    const socketIOPath = data.get('socketIOPath') as string ?? undefined;

    if (!hostname) {
      return;
    }

    connectionFormSubmitButton.setAttribute('aria-busy', 'true');
    await Api.setRotorHazardConnectionDetails(
      hostname,
      port ? parseInt(port) : undefined,
      socketIOPath,
    );
    connectionFormSubmitButton.setAttribute('aria-busy', 'false');
  });

  Api.on(ApiEventName.rotorhazard, (data) => {
    connectionForm.hostname.value = data.rotorhazard.connection.hostname;
    connectionForm.port.value = data.rotorhazard.connection.port?.toString() ?? '';
    connectionForm.socketIOPath.value = data.rotorhazard.connection.socketIOPath ?? '';
  });
}

let _selectedPilotId = -1;
async function fetchAndDisplayPilots() {
  console.log('Fetching pilots');
  const pilots = await Api.getRotorHazardPilots();
  populateSelectPilotSelectInput(pilots);

  const selectedPilot = pilots.find((pilot) => pilot.id === _selectedPilotId);
  updateSelectedPilotDisplay(selectedPilot);
}

export function setupRotorHazardSection() {
  console.log('Setting up RotorHazard section');

  console.log('Loading elements');
  loadElements();

  fetchAndDisplayPilots();
  setupSelectPilotForm();
  setupConnectionForm();

  Api.on(ApiEventName.rotorhazard, (data) => {
    _selectedPilotId = data.rotorhazard.pilotId;
    fetchAndDisplayPilots();
  });
}
