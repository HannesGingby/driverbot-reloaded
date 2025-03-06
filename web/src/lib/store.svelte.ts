export const co = $state({
  mouseCo: [0, 0],
});

export const mapProperties = $state({
  zoom: 100,
})

export const commandsSidebar = $state({
  width: 322,
})

export const infoSidebar = $state({
  width: 322,
})

interface AuthStore {
  loggedIn: boolean;
  loginSuccess?: boolean;
  username: string;
  mqttSuccess?: boolean;
  connectedToMQTT: boolean;
  attemptingMQTTConnection: boolean;
}

export const authStore = $state<AuthStore>({
  loggedIn: false,
  loginSuccess: undefined,
  username: "",
  mqttSuccess: undefined,
  connectedToMQTT: false,
  attemptingMQTTConnection: false,
})

export function resetAuthStore() {
  authStore.loggedIn = false;
  authStore.username = "";
  authStore.connectedToMQTT = false;
  authStore.attemptingMQTTConnection = false;
}

interface mqttStore {
  clusterURL: string,
  clusterBaseURL: string,
  port: number,
  protocol: string,
  username: string,
  password: string,
  espConnected: boolean,
  time: Date,
  elapsedTime: number,
  clientID: string,
  latestMqttSend?: string,
}

export const mqttStore = $state<mqttStore>({
  clusterURL: "",
  clusterBaseURL: "",
  port: 0,
  protocol: "",
  username: "",
  password: "",
  espConnected: false,
  time: new Date(),
  elapsedTime: 0,
  clientID: "",
  latestMqttSend: undefined,
})

export function resetMqttStore() {
  mqttStore.clusterURL = "";
  mqttStore.clusterBaseURL = "";
  mqttStore.port = 0;
  mqttStore.protocol = "";
  mqttStore.username = "";
  mqttStore.password = "";
  mqttStore.espConnected = false;
  mqttStore.time = new Date();
  mqttStore.elapsedTime = 0;
  mqttStore.clientID = "";
}

interface mqttSendData {
  direction?: string,
  steer?: string,
  speed?: number,
  steerAngle?: number,
}

export const mqttSendData = $state<mqttSendData>({
  direction: undefined,
  steer: undefined,
  speed: undefined,
  steerAngle: undefined,
})

export const applicationLogs = $state({logs: [["Init", new Date()]]});