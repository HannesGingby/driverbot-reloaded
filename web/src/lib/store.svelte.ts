export const theme = $state({
  lightMode: false,
});

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
  mqttError?: string;
  connectedToMQTT: boolean;
  attemptingMQTTConnection: boolean;
}

export const authStore = $state<AuthStore>({
  loggedIn: false,
  loginSuccess: undefined,
  username: "",
  mqttSuccess: undefined,
  mqttError: undefined,
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
  latestMovementSend?: string,
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
  latestMovementSend: undefined,
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

interface movement {
  driveDirection?: number,
  steerDirection?: number,
  driveSpeed?: number,
  steerSpeed?: number,
}

export const movementData = $state<movement>({
  driveDirection: undefined,
  steerDirection: undefined,
  driveSpeed: undefined,
  steerSpeed: undefined,
})

export const applicationLogs = $state({logs: [["Init app", new Date()]]});

export function logApplicationEvent(log: string): void {
  applicationLogs.logs.push([log, new Date()]);
  console.log(log);
}