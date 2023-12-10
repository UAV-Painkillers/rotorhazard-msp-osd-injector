export type ApiPinCode = `${number}${number}${number}${number}`;

export interface ApiWifiNetwork {
  ssid: string;
  password: string;
}

export interface ApiWifiStatus {
  hotspotIsEnabled: boolean;
  connectedSSID: string;
  storedSSID: string;
}

export interface ApiRotorHazardPilot {
  id: number;
  callsign: string;
  name: string;
}

export interface ApiRotorHazardConnectionDetails {
  hostname: string;
  port: number;
  socketIOPath: string;
}

export interface ApiAllDataWifi {
  storedSSID: string;
}

export interface ApiAllDataHotspot {
  isEnabled: boolean;
  credentials: ApiWifiNetwork;
}

export interface ApiAllDataRotorHazard {
  pilotId: number;
  connection: ApiRotorHazardConnectionDetails;
}

export interface ApiAllDataSerial {
  flightControllerBaudRate: number;
  loggingBaudRate: number;
}

export interface ApiAllDataOTA {
  isEnabled: boolean;
}

export interface ApiAllData {
  apiIsAuthenticated: boolean;
  wifi: ApiAllDataWifi;
  hotspot: ApiAllDataHotspot;
  rotorhazard: ApiAllDataRotorHazard;
  serial: ApiAllDataSerial;
  ota: ApiAllDataOTA;
}

export enum ApiEventName {
  is_authenticated = "is_authenticated",
  wifi = "wifi",
  hotspot = "hotspot",
  rotorhazard = "rotorhazard",
  serial = "serial",
  ota = "ota",
}

type ApiEventNameDataTypeMap = {
  [ApiEventName.is_authenticated]: Pick<ApiAllData, "apiIsAuthenticated">;
  [ApiEventName.wifi]: Pick<ApiAllData, "wifi">;
  [ApiEventName.hotspot]: Pick<ApiAllData, "hotspot">;
  [ApiEventName.rotorhazard]: Pick<ApiAllData, "rotorhazard">;
  [ApiEventName.serial]: Pick<ApiAllData, "serial">;
  [ApiEventName.ota]: Pick<ApiAllData, "ota">;
};

const LOCAL_STORAGE_PIN_KEY = "api_pin_code";

type DeepPartial<T> = T extends object
  ? {
      [P in keyof T]?: DeepPartial<T[P]>;
    }
  : T;

export class Api {
  private static rotorhazardHostname = "";
  private static rotorhazardPort = 5000;

  private static get pinCode() {
    return (localStorage.getItem(LOCAL_STORAGE_PIN_KEY) ?? "") as ApiPinCode;
  }

  private static set pinCode(code: ApiPinCode) {
    localStorage.setItem(LOCAL_STORAGE_PIN_KEY, code);
  }

  private static _callbacks: Record<ApiEventName, ((data: any) => void)[]> = {
    is_authenticated: [],
    wifi: [],
    hotspot: [],
    rotorhazard: [],
    serial: [],
    ota: [],
  };

  private static _lastEventPayloads: Partial<Record<ApiEventName, any>> = {};

  public static isAuthenticated = false;

  public static on<TEventName extends ApiEventName>(
    eventName: TEventName,
    callback: (data: ApiEventNameDataTypeMap[TEventName]) => void
  ) {
    this._callbacks[eventName].push(callback);

    const lastEventPayload = this._lastEventPayloads[eventName];
    if (lastEventPayload !== undefined) {
      callback(lastEventPayload);
    }
  }

  private static emit(
    eventName: ApiEventName | "*",
    data: DeepPartial<ApiAllData>
  ) {
    if (eventName === "*") {
      const allEventNames = Object.values(ApiEventName);
      allEventNames.forEach((event) => this.emit(event, data));
      return;
    }

    this._callbacks[eventName].forEach((callback) => callback(data));
  }

  private static getFullUrl(path: string): string {
    let hostname = location.hostname;

    const localStorageHostName = localStorage.getItem("BASE_URL");
    if (localStorageHostName) {
      hostname = localStorageHostName as string;
    }

    const protocol = location.protocol;
    const fullUrl = `${protocol}//${hostname}/api${path}`;
    return fullUrl;
  }

  private static async request(
    path: string,
    method: string = "GET",
    payload?: any,
    stringify = true
  ) {
    const response = await fetch(Api.getFullUrl(path), {
      method,
      body: payload && (stringify ? JSON.stringify(payload) : payload),
      headers: {
        "Content-Type": "application/json",
        Authorization: Api.pinCode
          ? `Basic ${btoa(`rh-osd:${Api.pinCode}`)}`
          : "",
      },
    });

    if (response.status === 403) {
      this.isAuthenticated = false;
      this.emit(ApiEventName.is_authenticated, {
        apiIsAuthenticated: this.isAuthenticated,
      });
      Api.pinCode = "" as ApiPinCode;
    }

    const data = await response.json();
    return {
      success: response.ok,
      data,
    };
  }

  public static async getAllData(): Promise<ApiAllData | string> {
    try {
      const response = await Api.request("/all");
      if (response.success) {
        this.rotorhazardHostname =
          response.data.rotorhazard.connection.hostname;
        this.rotorhazardPort = response.data.rotorhazard.connection.port;

        this.emit("*", {
          ...response.data,
          apiIsAuthenticated: this.isAuthenticated,
        });

        return response.data;
      } else {
        return response.data.error;
      }
    } catch (e) {
      console.error(e);
      return (e as Error).message;
    }
  }

  public static async authenticate(pin?: ApiPinCode): Promise<true | string> {
    try {
      if (!pin) {
        pin = Api.pinCode;
      }

      const response = await Api.request("/pin/verify", "POST", { pin });

      this.isAuthenticated = response.success;
      this.emit(ApiEventName.is_authenticated, {
        apiIsAuthenticated: this.isAuthenticated,
      });

      if (response.success) {
        Api.pinCode = pin;
        return true;
      } else {
        return response.data.error;
      }
    } catch (e) {
      console.error(e);
      return (e as Error).message;
    }
  }

  public static async requestPinCode(): Promise<true | string> {
    try {
      const response = await Api.request("/pin");
      if (response.success) {
        return true;
      }

      return response.data.error ?? "Something went wrong";
    } catch (e) {
      console.error(e);
      return (e as Error).message;
    }
  }

  public static async enableHotspot(): Promise<true | string> {
    try {
      const response = await Api.request("/wifi/actions/hotspot", "POST");
      if (response.success) {
        return true;
      }

      return response.data.error ?? "Something went wrong";
    } catch (e) {
      console.error(e);
      return (e as Error).message;
    }
  }

  public static async setHotspotCredentials(
    ssid: string,
    password: string
  ): Promise<true | string> {
    try {
      const response = await Api.request("/wifi/hotspot/credentials", "POST", {
        ssid,
        password,
      });
      if (response.success) {
        return true;
      }

      return response.data.error ?? "Something went wrong";
    } catch (e) {
      console.error(e);
      return (e as Error).message;
    }
  }

  public static async setWifiCredentials(
    ssid: string,
    password: string
  ): Promise<true | string> {
    try {
      const response = await Api.request("/wifi/credentials", "POST", {
        ssid,
        password,
      });
      if (response.success) {
        this.emit(ApiEventName.wifi, {
          wifi: {
            storedSSID: ssid,
          },
        });
        return true;
      }

      return response.data.error ?? "Something went wrong";
    } catch (e) {
      console.error(e);
      return (e as Error).message;
    }
  }

  public static async connectToWiFi(): Promise<true | string> {
    try {
      const response = await Api.request("/wifi/actions/connect", "POST");
      if (response.success) {
        return true;
      }

      return response.data.error ?? "Something went wrong";
    } catch (e) {
      console.error(e);
      return (e as Error).message;
    }
  }

  public static async reboot(): Promise<true | string> {
    try {
      const response = await Api.request("/system/actions/reboot", "POST");
      if (response.success) {
        return true;
      }

      return response.data.error ?? "Something went wrong";
    } catch (e) {
      console.error(e);
      return (e as Error).message;
    }
  }

  public static async setRotorHazardPilotId(
    pilotId: number
  ): Promise<true | string> {
    try {
      const response = await Api.request("/rotorhazard/pilot_id", "POST", {
        pilotId,
      });
      if (response.success) {
        this.emit(ApiEventName.rotorhazard, {
          rotorhazard: {
            ...(this._lastEventPayloads[ApiEventName.rotorhazard] ?? {}),
            pilotId,
          },
        });
        return true;
      }

      return response.data.error ?? "Something went wrong";
    } catch (e) {
      console.error(e);
      return (e as Error).message;
    }
  }

  public static async setRotorHazardConnectionDetails(
    hostname: string,
    port: number = 5000,
    socketIOPath?: string
  ): Promise<true | string> {
    try {
      const response = await Api.request("/rotorhazard/connection", "POST", {
        hostname,
        port,
        socketIOPath,
      });
      if (response.success) {
        this.emit(ApiEventName.rotorhazard, {
          rotorhazard: {
            ...(this._lastEventPayloads[ApiEventName.rotorhazard] ?? {}),
            connection: {
              hostname,
              port,
              socketIOPath,
            },
          },
        });
        return true;
      }

      return response.data.error ?? "Something went wrong";
    } catch (e) {
      console.error(e);
      return (e as Error).message;
    }
  }

  public static async setFlightControllerBaudRate(
    baudRate: number
  ): Promise<true | string> {
    try {
      const response = await Api.request(
        "/flight_controller/baud_rate",
        "POST",
        { baudRate }
      );
      if (response.success) {
        this.emit(ApiEventName.serial, {
          serial: {
            ...(this._lastEventPayloads[ApiEventName.serial] ?? {}),
            flightControllerBaudRate: baudRate,
          },
        });
        return true;
      }

      return response.data.error ?? "Something went wrong";
    } catch (e) {
      console.error(e);
      return (e as Error).message;
    }
  }

  public static async setLoggingBaudRate(
    baudRate: number
  ): Promise<true | string> {
    try {
      const response = await Api.request("/logging/baud_rate", "POST", {
        baudRate,
      });
      if (response.success) {
        this.emit(ApiEventName.serial, {
          serial: {
            ...(this._lastEventPayloads[ApiEventName.serial] ?? {}),
            loggingBaudRate: baudRate,
          },
        });
        return true;
      }

      return response.data.error ?? "Something went wrong";
    } catch (e) {
      console.error(e);
      return (e as Error).message;
    }
  }

  public static async getRotorHazardPilots() {
    if (!this.rotorhazardHostname) {
      console.error("RotorHazard hostname not set");
      return [];
    }

    try {
      const response = await fetch(
        "http://" +
          Api.rotorhazardHostname +
          ":" +
          Api.rotorhazardPort +
          "/api/pilot/all"
      );

      if (!response.ok) {
        const data = await response.json();
        console.error(data);
        return [];
      }

      const data = await response.json();
      return data.pilots as ApiRotorHazardPilot[];
    } catch (e) {
      console.error(e);
      return [];
    }
  }

  public static async disableOTA(): Promise<true | string> {
    try {
      const response = await Api.request("/ota/actions/disable", "POST");
      if (response.success) {
        this.emit(ApiEventName.ota, {
          ota: {
            ...(this._lastEventPayloads[ApiEventName.ota] ?? {}),
            isEnabled: false,
          },
        });
        return true;
      }

      return response.data.error ?? "Something went wrong";
    } catch (e) {
      console.error(e);
      return (e as Error).message;
    }
  }

  public static async enableOTA(): Promise<true | string> {
    try {
      const response = await Api.request("/ota/actions/enable", "POST");
      if (response.success) {
        this.emit(ApiEventName.ota, {
          ota: {
            ...(this._lastEventPayloads[ApiEventName.ota] ?? {}),
            isEnabled: true,
          },
        });
        return true;
      }

      return response.data.error ?? "Something went wrong";
    } catch (e) {
      console.error(e);
      return (e as Error).message;
    }
  }

  public static async uploadFirmware(file: File) {
    try {
      // file needs to be uploaded as multipart/form-data
      const formData = new FormData();
      formData.append("firmware", file);

      // send the request
      const response = await this.request(
        "/system/actions/update",
        "POST",
        formData,
        false
      );

      if (response.success) {
        return true;
      }

      return response.data.error ?? "Something went wrong";
    } catch (e) {
      console.error(e);
      return (e as Error).message;
    }
  }
}
