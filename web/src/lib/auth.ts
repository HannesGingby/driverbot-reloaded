import { disconnect } from "./mqtt.js";
import { authStore, mqttStore, resetMqttStore, resetAuthStore, logApplicationEvent } from "./store.svelte.js";

export function login(username: string, password: string): boolean {
  // todo

  // todo: get MQTT collection associated with the user

  authStore.loggedIn = true;
  authStore.username = username;

  logApplicationEvent("Login successful");
  return true;
}

export function setManualMQTT(clusterURL: string, username: string, password: string ) {
  const url = new URL(clusterURL);
  const clusterBaseURL = url.hostname;
  const clusterPort = parseInt(url.port);

  mqttStore.clusterURL = clusterURL;
  mqttStore.clusterBaseURL = clusterBaseURL;
  mqttStore.port = clusterPort;
  mqttStore.username = username;
  mqttStore.password = password;

  logApplicationEvent("Manual MQTT connection set");
}

export function logout() {
  // todo

  disconnect();
  logApplicationEvent("Disconnected from MQTT");

  resetAuthStore();
  resetMqttStore();
  logApplicationEvent("Resetting session data");
}